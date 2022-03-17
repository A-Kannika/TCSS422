// TCSS422 Operating Systems
// Assignment3: Linux Kernel Module - Page Table Walker
// @author Kannika Armstrong

#include <linux/module.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

// process data
typedef struct proc_data {
	int proc_id;
	char *proc_name;
	int contig_pages;
	int noncontig_pages;
  	struct proc_data *next;
} proc_data;

//global variables.
typedef struct count_data {
  	proc_data *head;
  	int total_cnt;
  	int total_ncnt;
} count_data;

count_data count_head;

// function prototypes
int proc_init (void);
static void generate_list (void);
unsigned long virt2phys (struct mm_struct*, unsigned long);
static int proc_open (struct inode *, struct file *);
static int proc_print (struct seq_file *, void *);
void proc_cleanup (void);
void proc_free (void);

// define proc_open prior to being called
static const struct file_operations proc_fops = {
  	.owner = THIS_MODULE,
  	.open = proc_open,
  	.read = seq_read,
  	.llseek = seq_lseek,
  	.release = single_release,
};

int proc_init (void) {
  	generate_list();
  	proc_create("proc_report", 0, NULL, &proc_fops); 
  	return 0;
}

// Create a linked list of proc_data nodes for processes with PID > 650 
static void generate_list (void) {
  	struct task_struct *task;
  	struct vm_area_struct *vma;
  	unsigned long prev, vpage;
  	proc_data *temp;

  	temp = kmalloc(sizeof(proc_data), GFP_KERNEL);
  	temp->next = NULL;
  	count_head.head = temp;
  	prev = 0;

  	for_each_process(task) {
    		if(task->pid > 650) {
      			temp->proc_id = task->pid;
      			temp->proc_name = task->comm;
      			temp->contig_pages = 0;
      			temp->noncontig_pages = 0;
      			vma = 0;
      			if (task->mm && task->mm->mmap) {
        			for (vma = task->mm->mmap; vma; vma = vma->vm_next) {
          				for (vpage = vma->vm_start; vpage < vma->vm_end; vpage += PAGE_SIZE) {
            					unsigned long physical_page_addr = virt2phys(task->mm, vpage);
            					if(physical_page_addr) {
           						(physical_page_addr == prev + PAGE_SIZE)? temp->contig_pages++ : temp->noncontig_pages++;
              						prev = physical_page_addr;
            					}
          				}
        			}
        			count_head.total_cnt += temp->contig_pages;
        			count_head.total_ncnt += temp->noncontig_pages;  
      			}
      			temp->next = kmalloc(sizeof(proc_data), GFP_KERNEL);
      			temp = temp->next;
    		}
  	} 
  	temp->next = NULL;
  	temp = NULL;
  	kfree(temp);
}

// Virtual address to physical address
unsigned long virt2phys(struct mm_struct *mm, unsigned long vpage) {
  	unsigned long physical_page_addr;
  	pgd_t *pgd;
  	p4d_t *p4d;
  	pud_t *pud;
  	pmd_t *pmd;
  	pte_t *pte;
  	struct page *page;
  	pgd = pgd_offset(mm, vpage);
  	if(pgd_none(*pgd) || pgd_bad(*pgd)) {
      		return 0;
  	}
  	p4d = p4d_offset(pgd, vpage);
  	if(p4d_none(*p4d) || p4d_bad(*p4d)) {
      		return 0;
  	}
  	pud = pud_offset(p4d, vpage);
  	if(pud_none(*pud) || pud_bad(*pud)) {
      		return 0;
  	}
  	pmd = pmd_offset(pud, vpage);
  	if(pmd_none(*pmd) || pmd_bad(*pmd)) {
      		return 0;
  	}
  	if(!(pte = pte_offset_map(pmd,vpage))){
      		return 0;
  	}
  	if(!(page = pte_page(*pte))){
      		return 0;
  	}
  	physical_page_addr = page_to_phys(page);
  	pte_unmap(pte);
  	if(physical_page_addr == 70368744173568) {
      		return 0;
  	}
  	return physical_page_addr;
}

//creates, opens and writes to file in /proc/proc_report using the proc_show method.
static int proc_open(struct inode *inode, struct file *file) {
  	return single_open(file, proc_print, NULL);
}

//Print process report to var/syslog and to /proc/proc_report as a CSV list
static int proc_print(struct seq_file *m, void *v) {
  	proc_data *temp = count_head.head;
  	printk(KERN_INFO "PROCESS REPORT:\n");
  	printk(KERN_INFO "proc_id,proc_name,contig_pages,noncontig_pages,total_pages");
  	seq_printf(m, "PROCESS REPORT:\n");
  	seq_printf(m, "%s,%s,%s,%s,%s\n" ,"proc_id", "proc_name", "contig_pages", "noncontig_pages", "total_pages");

  	while(temp->next) {
    		seq_printf(m, "%d,%s,%d,%d,%d\n", temp->proc_id, temp->proc_name, temp->contig_pages, temp->noncontig_pages, temp->contig_pages + temp->noncontig_pages);
    		printk(KERN_INFO "%d,%s,%d,%d,%d\n", temp->proc_id, temp->proc_name, temp->contig_pages, temp->noncontig_pages, temp->contig_pages + temp->noncontig_pages);
    	temp = temp->next;  
  	}
  	
  	seq_printf(m, "%s,,%d,%d,%d\n", "TOTALS", count_head.total_cnt, count_head.total_ncnt, count_head.total_cnt + count_head.total_ncnt);
  	printk(KERN_INFO "%s,,%d,%d,%d\n", "TOTALS", count_head.total_cnt, count_head.total_ncnt, count_head.total_cnt + count_head.total_ncnt);
  	return 0;
}


//clean up and close the kernal process, memory
void proc_cleanup (void) {
  	remove_proc_entry("proc_report", NULL);
  	proc_free();
  	printk(KERN_INFO "procReport: performing cleanup of module\n");
}

// frees all the memory allocated to the proc_data nodes
void proc_free (void) {
  	proc_data *temp;  
  	while(count_head.head) {
    		temp = count_head.head->next;
    		kfree(count_head.head);
    		count_head.head = temp;
  	}
}

MODULE_LICENSE("GPL");
module_init(proc_init);
module_exit(proc_cleanup);
