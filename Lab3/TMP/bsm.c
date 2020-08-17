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
   // kprintf("\nInitializing bsm_tab...");
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
   // kprintf("\nNo free entry");
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
	/*if(bsm_tab[i].bs_status == BSM_MAPPED){
		restore(ps);
		return(SYSERR);
	}*/
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
//    if(vpagenum == 262271){
  //  kprintf("\nvaddr %08x vpagenum %d and vpno %d and pid val%d npages %d", vaddr, vpagenum, bsm_tab[1].bs_vpno[pid], bsm_tab[1].bs_pid[pid], bsm_tab[1].bs_npages[pid] );
    for(i=0;i<BSELIMIT;i++){
        if(bsm_tab[i].bs_pid[pid]==1 && vpagenum <= bsm_tab[i].bs_vpno[pid] + bsm_tab[i].bs_npages[pid] && vpagenum >= bsm_tab[i].bs_vpno[pid]){
//	      kprintf("\nvpagenum %d and vpno %d and npages %d", vpagenum, bsm_tab[i].bs_vpno[pid], bsm_tab[i].bs_npages[pid]);
             *store = i;
             *pageth = (int)(vpagenum - bsm_tab[i].bs_vpno[pid]);
	      restore(ps);
	      return(OK);
        }
    }
    //kprintf("\nvaddr %08x vpagenum %d and vpno %d and pid val%d npages %d", vaddr, vpagenum, bsm_tab[1].bs_vpno[pid], bsm_tab[1].bs_pid[pid], bsm_tab[1].bs_npages[pid] );

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
   // bsm_tab[source].bs_status = BSM_MAPPED;
    bsm_tab[source].bs_limit = npages;
    //kprintf("shaered ..");
    }
	//kprintf("\nbsm vpno %d and pid %d and bsm status %d and bsm number %d", vpno,pid, bsm_tab[source].bs_status, source); /* By default BS type is shared */
    bsm_tab[source].bs_status = BSM_MAPPED;
    //kprintf("\nMapped bs %d with status %d and vpno %d and npages %d", source, bsm_tab[source].bs_status, vpno, npages);
    bsm_tab[source].bs_pid[pid] = 1;
    bsm_tab[source].bs_vpno[pid] = vpno;
    bsm_tab[source].bs_npages[pid] = npages;
    
//   kprintf("\nMapped bs %d with status %d and vpno %d and pid %d", source, bsm_tab[source].bs_vpno[pid], bsm_tab[source].bs_npages[pid], pid);

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
   // kprintf("\nUnmapping %d", vpno);
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
		//if( pageth  == 14|| pageth == 15 || pageth == 16 ){
		//kprintf("\n vpno %d frame vpno %d dirty bit %d", bsm_tab[store].bs_vpno[pid] + bsm_tab[store].bs_npages[pid], frm_tab[k].fr_vpno, ptent -> pt_dirty);
		//	}
	
		if(frm_tab[k].fr_vpno <= vpno + bsm_tab[store].bs_npages[pid] && frm_tab[k].fr_vpno >= vpno){
		if(ptent->pt_dirty == 1){		// Only if frame is dirty, write back to BS 
                	//bsm_lookup(pid, frm_tab[k].fr_vpno*NBPG , &store, &pageth);
                	write_bs((char *)((FRAME0 + k)*NBPG) ,store, pageth);
//			kprintf("\nWrote store %d and page %d with vpno %d frvpno %d npages %d ",  store, pageth, vpno,frm_tab[k].fr_vpno, bsm_tab[store].bs_npages[pid]);
			}
		page_removal_queue(k);              
		free_frm(k); 
		 }
         }}
                
    restore(ps);
    return(OK);
}

void bsm_unmapped(unsigned int i, int pid){
//	for(i=0;i<BSELIMIT;i++){
//        if(bsm_tab[i].bs_vpno[pid]!= vpno ){
            bsm_tab[i].bs_pid[pid] = 0;
            bsm_tab[i].bs_npages[pid] = 0;

            bsm_tab[i].bs_vpno[pid] = 0;
  //          break;
  //      }
  //  }
    if(bsm_tab[i].bs_private == PRIVATE){
        bsm_tab[i].bs_status = BSM_UNMAPPED;
    }
    if(bsm_tab[i].bs_private == SHARED){
        int j;
        int flag = 0;
        for(j=0;j<NPROC;j++){
            if(proctab[j].pstate != PRFREE && bsm_tab[i].bs_pid[j] == 1){
		//kprintf("\nHits here");
                flag = 1;
                break;
            }
        }
        if(!flag){
            bsm_tab[i].bs_status = BSM_PRESENT;
            free_bsm(i);
//            kprintf("\n Freed %d", i);
        }
    }
}
/* Writing out pages when resched is called */

void switch_writeouts(int pid){
	int  k;
    	int store, pageth;
	for(k=0; k<NFRAMES; k++){  // Check inverted page table and write back to BS 
        if(frm_tab[k].fr_type == FR_PAGE && frm_tab[k].fr_pid == pid  ){
               
/*		virt_addr_t *va;
                unsigned long vadd = frm_tab[k].fr_vpno*NBPG;
                va = (virt_addr_t *)&vadd;
                unsigned int pdentry = va->pd_offset;
                unsigned int ptentry = va->pt_offset;
                pd_t *pdent = (pd_t *)((proctab[frm_tab[k].fr_pid].pdbr)+pdentry*4);
                pt_t *ptent = (pt_t *)((pdent->pd_base)*NBPG + ptentry*4);
                bsm_lookup(pid, frm_tab[k].fr_vpno * NBPG, &store, &pageth);
                

                if(frm_tab[k].fr_vpno <= bsm_tab[store].bs_vpno[pid] + bsm_tab[store].bs_npages[pid] && frm_tab[k].fr_vpno >= bsm_tab[store].bs_vpno[pid]){
                if(ptent->pt_dirty == 1){               
                        write_bs((char *)((FRAME0 + k)*NBPG) ,store, pageth);

                        }*/

                page_removal_queue(k);
                free_frm(k); 
//		kprintf("\nFreed frame %d", k);
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
                                                                                                                                                               if(ptent->pt_dirty == 1){
                                                                                                                                                                                       write_bs((char *)((FRAME0 + k)*NBPG) ,store, pageth);
 
                                                                                                                                                                                                               }
                page_removal_queue(k);
                free_frm(k);

          }
         }
}} 
