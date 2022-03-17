// TCSS422 Operating Systems
// Assignment3: Linux Kernel Module - Page Table Walker
// @author Kannika Armstrong

#include<linux/module.h>
#include<linux/sched/signal.h>
#include<linux/skbuff.h>

// globals counters for contig_pages and noncontig_pages
unsigned long contig_pages = 0;
unsigned long noncontig_pages = 0;

// translate virtual address to physical address
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

// the function pro_count
int proc_count (void) {
  	int i = 0;
  	int total_pages = 0;
  	int counter = 0;
  	struct task_struct *thechild; 
  	struct vm_area_struct *vma = 0;
  	unsigned long vpage;

  	// the helper function for task_struct data
  	for_each_process(thechild) {
    	int contig = 0;
    	int noncontig = 0;
    	// only print infos for pid > 650
    	if (thechild->pid > 650) {
      		if(thechild->mm && thechild->mm->mmap) {
        		unsigned long prev_page;
        		for(vma = thechild->mm->mmap; vma; vma = vma->vm_next) {
          			for(vpage = vma->vm_start; vpage < vma->vm_end; vpage += PAGE_SIZE) {
            				unsigned long physical_page_addr = virt2phys (thechild->mm, vpage);
            				if (physical_page_addr != 0) {
              					if (PAGE_SIZE + prev_page == physical_page_addr) {
                					contig++;
              					} else {
                					noncontig++;
              					}
              					prev_page = physical_page_addr;
              					counter++;
            				}
          			}
        		}
      		}
      		total_pages += counter; 
      		if (counter > 0) {
        		printk("%d,%s,%d,%d,%d\n",thechild->pid, thechild->comm, contig, noncontig, counter);
        		i++;
      		}
    	}
    	contig_pages += contig;
    	noncontig_pages += noncontig;
    	counter = 0;
  	}
  	printk(KERN_INFO "TOTALS,,%lu,%lu,%d\n", contig_pages, noncontig_pages, total_pages);
  	return i;
}

int proc_init (void) {
  	printk("PROCESS REPORT:\n");
	printk(KERN_INFO "proc_id,proc_name,contig_pages,noncontig_pages,total_pages\n");
  	proc_count();
  	return 0;
}


void proc_cleanup(void) {
  	printk(KERN_INFO "procReport: performing cleanup of module\n");
}

MODULE_LICENSE("GPL");
module_init(proc_init);
module_exit(proc_cleanup);

