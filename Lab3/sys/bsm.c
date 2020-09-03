/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
    STATWORD ps;
    disable(ps);
    int i;
    for(i=0;i<BSELIMIT;i++){
        bsm_tab[i].bs_status = BSM_UNMAPPED;
        bsm_tab[i].bs_sem = -1;       /* bs_Sem is set to -1 */
        bsm_tab[i].bs_limit = 0;     /*Number of pages initially is 0 in each BS */
        int j;
        for(j = 0; j<NPROC;j++){
            bsm_tab[i].bs_pid[j] =  0;   /* Both pid and vpn0 for each pid is set it as 0 */
            bsm_tab[i].bs_vpno[j] = 0;
            bsm_tab[i].bs_npages[j] = 0;
        }
        bsm_tab[i].bs_private = SHARED;   /*BS is shared */
    }

    restore(ps);
    return(OK);
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
    STATWORD ps;
    disable(ps);
    int i;
    for(i=0;i<BSELIMIT;i++){
        if(bsm_tab[i].bs_status == BSM_UNMAPPED){   
            *avail = i;
	    restore(ps);
            return(OK);
        }
    }
    restore(ps);
    return(SYSERR);
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
    STATWORD ps;
    disable(ps);       /* Only if the BS is private, should it be reinitialized */
    bsm_tab[i].bs_status = BSM_UNMAPPED;
    bsm_tab[i].bs_sem = -1;
    bsm_tab[i].bs_limit = 0;     /*Number of pages initially is 0 in each BS */
    int j;
    for(j = 0; j<NPROC;j++){
        bsm_tab[i].bs_pid[j] = 0;
        bsm_tab[i].bs_vpno[j] = 0;
        bsm_tab[i].bs_npages[j] = 0;
    }
    bsm_tab[i].bs_private = SHARED;
  
    restore(ps);
    return OK;
}


/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, unsigned long vaddr, int* store, int* pageth)
{
    STATWORD ps;
    disable(ps);
    int i;
    unsigned int vpagenum = (unsigned int)vaddr/NBPG;
    for(i=0;i<BSELIMIT;i++){
        if(bsm_tab[i].bs_pid[pid]==1 && vpagenum <= bsm_tab[i].bs_vpno[pid] + bsm_tab[i].bs_npages[pid] && vpagenum >= bsm_tab[i].bs_vpno[pid]){
             *store = i;
             *pageth = (int)(vpagenum - bsm_tab[i].bs_vpno[pid]);
	      restore(ps);
	      return(OK);
        }
    }
    kprintf("\nInvalid virtual address %08x, killing process with process id %d", vaddr, pid);
    kill(pid);
    restore(ps);
    return(SYSERR);
}


/*-------------------------------------------------------------------------
 * bsm_map - add a mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
    STATWORD ps;
    disable(ps);
    if(bsm_tab[source].bs_status == BSM_UNMAPPED){      /* Previously unmapped, set status and limit */
        bsm_tab[source].bs_limit = npages;
    }
    bsm_tab[source].bs_status = BSM_MAPPED;
    bsm_tab[source].bs_pid[pid] = 1;
    bsm_tab[source].bs_vpno[pid] = vpno;
    bsm_tab[source].bs_npages[pid] = npages;
    restore(ps);
    return(OK);
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno)
{
    STATWORD ps;
    disable(ps);
    int i, k;
    int store, pageth;
    for(k=0; k<NFRAMES; k++){  // Check inverted page table and write back to BS 
        if(frm_tab[k].fr_type == FR_PAGE && frm_tab[k].fr_pid == pid   ){
		// Check if page is dirty 
		virt_addr_t *va;
		unsigned long vadd = frm_tab[k].fr_vpno*NBPG;
		va = (virt_addr_t *)&vadd;
		unsigned int pdentry = va->pd_offset;
		unsigned int ptentry = va->pt_offset;
		pd_t *pdent = (pd_t *)((proctab[frm_tab[k].fr_pid].pdbr)+pdentry*4);
		pt_t *ptent = (pt_t *)((pdent->pd_base)*NBPG + ptentry*4);
		bsm_lookup(pid, frm_tab[k].fr_vpno * NBPG, &store, &pageth);
	
		if(frm_tab[k].fr_vpno <= vpno + bsm_tab[store].bs_npages[pid] && frm_tab[k].fr_vpno >= vpno){
		if(ptent->pt_dirty == 1){		// Only if frame is dirty, write back to BS 
        	write_bs((char *)((FRAME0 + k)*NBPG) ,store, pageth);
		}
		page_removal_queue(k);              
		free_frm(k); 
		 }
         }}
                
    restore(ps);
    return(OK);
}

void bsm_unmapped(unsigned int i, int pid){

    bsm_tab[i].bs_pid[pid] = 0;
    bsm_tab[i].bs_npages[pid] = 0;
    bsm_tab[i].bs_vpno[pid] = 0;
 
    if(bsm_tab[i].bs_private == PRIVATE){
        bsm_tab[i].bs_status = BSM_UNMAPPED;
    }
    if(bsm_tab[i].bs_private == SHARED){
        int j;
        int flag = 0;
        for(j=0;j<NPROC;j++){
            if(proctab[j].pstate != PRFREE && bsm_tab[i].bs_pid[j] == 1){
                flag = 1;
                break;
            }
        }
        if(!flag){
            bsm_tab[i].bs_status = BSM_PRESENT;
            free_bsm(i);
        }
    }
}
/* Writing out pages when resched is called */

void switch_writeouts(int pid){         // Not implemented
	int  k;
    	int store, pageth;
	for(k=0; k<NFRAMES; k++){  // Check inverted page table and write back to BS 
        if(frm_tab[k].fr_type == FR_PAGE && frm_tab[k].fr_pid == pid  ){
                page_removal_queue(k);
                free_frm(k); 
          }                   
         }
}


void switch_writeoutsr(int pid){
        int  k;
        int store, pageth;
        for(k=0; k<NFRAMES; k++){  
        if(frm_tab[k].fr_type == FR_PAGE && frm_tab[k].fr_pid == pid  ){

              virt_addr_t *va;
              unsigned long vadd = frm_tab[k].fr_vpno*NBPG;
              va = (virt_addr_t *)&vadd;
              unsigned int pdentry = va->pd_offset;
              unsigned int ptentry = va->pt_offset;
              pd_t *pdent = (pd_t *)((proctab[frm_tab[k].fr_pid].pdbr)+pdentry*4);
              pt_t *ptent = (pt_t *)((pdent->pd_base)*NBPG + ptentry*4);
              bsm_lookup(pid, frm_tab[k].fr_vpno * NBPG, &store, &pageth); 
              if(frm_tab[k].fr_vpno <= bsm_tab[store].bs_vpno[pid] + bsm_tab[store].bs_npages[pid] && frm_tab[k].fr_vpno >= bsm_tab[store].bs_vpno[pid] ){
                 page_removal_queue(k);
                free_frm(k);

          }
         }
}} 
