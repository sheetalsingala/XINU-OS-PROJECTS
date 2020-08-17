/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
  // kprintf("\nInitializing frm_tab....\n");
    STATWORD ps;
    disable(ps);
    int i;
    for(i=0;i<NFRAMES;i++){
      frm_tab[i].fr_vpno = 0;
      frm_tab[i].fr_type = FR_PAGE;     /* All pages are pages in the beginning */
      frm_tab[i].fr_refcnt = 0;
      frm_tab[i].fr_status = FRM_UNMAPPED;
      frm_tab[i].fr_dirty = NDIRTY;
      frm_tab[i].fr_pid = -1;
      frm_tab[i].fr_nframe = -1;
      frm_tab[i].fr_pframe = -1;
    }
    restore(ps);
    return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
  // kprintf("To be implemented!\n");
  STATWORD ps;
  disable(ps);
  int i, victimPage;
  for(i=0;i<NFRAMES;i++)
  {
    if(frm_tab[i].fr_status==FRM_UNMAPPED){   /* Find an entry that is unmapped */
          *avail = i;
	//  kprintf("\nGet frame %d and currpid %d", *avail, currpid);
	   restore(ps);
	   return(OK);
	   //kprintf("\n Get frame %d", i);
    }
  }
  /* If no available frame call replacement policies */
  if(page_replace_policy == SC){
    victimPage = SC_policy();
  }
  if(page_replace_policy == LFU){
    victimPage = LFU_Policy();
  }
  *avail = victimPage;
  //kprintf("\nVictim Page %d", *avail);
  restore(ps);
  return OK;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)       /* Free a frame same as reinitializing the frame */
{
  STATWORD ps;
  disable(ps);
  if ( i> NFRAMES){    /*Check if index is valid */
    return(SYSERR);
  }
    if(frm_tab[i].fr_type == FR_PAGE){
	virt_addr_t *v_a;
	unsigned long vaddr =(unsigned long)( frm_tab[i].fr_vpno * NBPG);
  	v_a = (virt_addr_t*)&vaddr;	
  	unsigned int ptentry = v_a->pt_offset;
  	unsigned int pdentry = v_a->pd_offset;
  	unsigned int vp = vaddr/NBPG;  
	pd_t *pdent  =(pd_t *) ((proctab[frm_tab[i].fr_pid].pdbr) + pdentry*4 );
	pt_t *ptent = (pt_t *)((pdent->pd_base)*NBPG + ptentry*4 );
	ptent->pt_pres = 0;	/* Invalidate entry in page table */
	ptent->pt_write = 0;
	frm_tab[pdent->pd_base - FRAME0].fr_refcnt -= 1;  
	if(frm_tab[pdent->pd_base - FRAME0].fr_refcnt == 0){	/*Decrement counter and check if number of pages is zero*/
		pdent->pd_pres = 0;
		frm_tab[pdent->pd_base - FRAME0].fr_type = FR_PAGE;
		frm_tab[pdent->pd_base - FRAME0].fr_status = FRM_UNMAPPED;
		frm_tab[pdent->pd_base - FRAME0].fr_dirty = NDIRTY;
		frm_tab[pdent->pd_base - FRAME0].fr_pid = -1;
		frm_tab[pdent->pd_base - FRAME0].fr_refcnt = 0;		/* Free page table */	
		//kprintf("\nFreed Page table");		
	}
	}
    frm_tab[i].fr_type = FR_PAGE;     /* All pages are pages in the beginning */
    frm_tab[i].fr_status = FRM_UNMAPPED;
    frm_tab[i].fr_dirty = NDIRTY;
    frm_tab[i].fr_pid = -1;  
    frm_tab[i].fr_vpno = 0;
    //kprintf("\nFreed Frame %d", i);
  restore(ps);
  return OK;
}



int SC_policy(void){
  int retVal, next;
  while(1){
        virt_addr_t *v_a;
        unsigned long vaddr = frm_tab[currpoint].fr_vpno*NBPG;
        v_a = (virt_addr_t*)&vaddr;	/* Obtain page number, page directory and offset */
        unsigned int ptentry = v_a->pt_offset;
        unsigned int pdentry = v_a->pd_offset;
        unsigned int vp = vaddr/NBPG; 
        pd_t *pdent  =(pd_t *) ((proctab[frm_tab[currpoint].fr_pid].pdbr) + pdentry*4 );
        pt_t *pageEntry = (pt_t *)((pdent->pd_base)*NBPG + ptentry*4 );
        if(pageEntry->pt_acc == 1){
            pageEntry->pt_acc = 0;
	//    kprintf("\nHits here");
        }
        else{
          /*Replace page */
          if(pageEntry->pt_dirty == 1){
                page_removal_queue(currpoint);
                page_removal(currpoint, 1);
          }
          else{
                page_removal_queue(currpoint);
                page_removal(currpoint, 0);
          }
	  break;
        }
      
        next = frm_tab[currpoint].fr_nframe;
	currpoint = next;
	
    }
	next = frm_tab[currpoint].fr_nframe;
	retVal = currpoint;
	frm_tab[currpoint].fr_nframe = -1;
	frm_tab[currpoint].fr_pframe = -1;
	free_frm(currpoint);
	currpoint = next;
	if(debug_mode){
		kprintf("\nFrame replaced %d (SC)\n", retVal + FRAME0);
	}
	return(retVal);
  }

void page_removal_queue(int iter){
  if(iter == prhead && iter == prtail){
	prhead = PRHEAD;
	//kprintf("\Should hit here!");
	prtail = PRTAIL;
 	}
  if(iter == prhead){    /* Replaced page is head, change PRHEAD */
    //kprintf("\nHEad replaced prhead %d prtail %d", prhead, prtail);
    prhead = frm_tab[iter].fr_nframe;
    frm_tab[prhead].fr_pframe = prtail;
    frm_tab[prtail].fr_nframe = prhead;
  }
  if(iter == prtail){  /* Replaced page is tail, change PRTAIL */
    prtail = frm_tab[iter].fr_pframe;
    frm_tab[prtail].fr_nframe = prhead;
    frm_tab[prhead].fr_pframe = prtail;
  }
 if(iter != prhead && iter != prtail){	/* Replaced page is neither head nor tail */
    int temp = frm_tab[iter].fr_pframe;
    frm_tab[temp].fr_nframe = frm_tab[iter].fr_nframe;
    frm_tab[frm_tab[iter].fr_nframe].fr_pframe = temp;
  }
  //free_frm(iter);
}

void page_removal(int iter, int flag){  /* Page is dirty if flag =1 */
      if(flag){   /*Dirty page */
	    //kprintf("\nDirty!");
            int store, pageth;
            bsm_lookup(currpid, frm_tab[iter].fr_vpno*NBPG , &store, &pageth);
//	    kprintf("\nPage written back %d", pageth);
            write_bs((char *)((FRAME0 + iter)*NBPG) ,store, pageth);
      }
}


int LFU_Policy(void){
	int retVal, next;
	int iter = prhead;
	int minIter = prhead;
	int min = frm_tab[iter].fr_refcnt;
	while(iter!= prtail){
	//		kprintf("\nIter %d and prtail %d nextframe %d", iter, prtail, frm_tab[iter].fr_nframe);
			/*if(iter == 0){
				iter = prhead;
			}*/
			virt_addr_t *v_a;
       			unsigned long vaddr = frm_tab[iter].fr_vpno*NBPG;
        		v_a = (virt_addr_t*)&vaddr;     /* Obtain page number, page directory and offset */
        		unsigned int ptentry = v_a->pt_offset;
        		unsigned int pdentry = v_a->pd_offset;
        		unsigned int vp = vaddr/NBPG;
        		pd_t *pdent  =(pd_t *) ((proctab[frm_tab[iter].fr_pid].pdbr) + pdentry*4 );
        		pt_t *pageEntry = (pt_t *)((pdent->pd_base)*NBPG + ptentry*4 );
			if(min>frm_tab[iter].fr_refcnt){	/* Check if minimum */
				min = frm_tab[iter].fr_refcnt;
				minIter = iter;
			
			}
			if(min == frm_tab[iter].fr_refcnt && frm_tab[minIter].fr_vpno < frm_tab[iter].fr_vpno){
        			min = frm_tab[iter].fr_refcnt;
       		 		minIter = iter;
      				} 
			next = frm_tab[iter].fr_nframe;
			iter = next;
			if(iter == prtail){
				break;
			}
	//		kprintf("\niter %d prhead %d prtail %d", iter, prhead, prtail);
	}
//	kprintf("\nEnd of loop");
	virt_addr_t *v_a;
        unsigned long vaddr = frm_tab[prtail].fr_vpno*NBPG;
        v_a = (virt_addr_t*)&vaddr;     /* Obtain page number, page directory and offset */
        unsigned int ptentry = v_a->pt_offset;
        unsigned int pdentry = v_a->pd_offset;
        unsigned int vp = vaddr/NBPG;
        pd_t *pdent  =(pd_t *) ((proctab[frm_tab[prtail].fr_pid].pdbr) + pdentry*4 );
        pt_t *pageEntry = (pt_t *)((pdent->pd_base)*NBPG + ptentry*4 );
        if(min>frm_tab[prtail].fr_refcnt){        /* Check if minimum */
                  min = frm_tab[prtail].fr_refcnt;
                  minIter = prtail;
                  }
	if(min == frm_tab[iter].fr_refcnt && frm_tab[minIter].fr_vpno < frm_tab[iter].fr_vpno){
                                min = frm_tab[iter].fr_refcnt;
                                minIter = iter;
                                }

	/* now that the minimum is found */
//	kprintf("\nMiniter %d prhead %d prtail %d", minIter, prhead, prtail), 
	page_removal_queue(minIter);
//	kprintf("\nMiniter %d");
	virt_addr_t *va1;
	unsigned long vaddr1 = frm_tab[minIter].fr_vpno*NBPG;
        va1 = (virt_addr_t*)&vaddr1;     /* Obtain page number, page directory and offset */
        unsigned int ptentry1 = va1->pt_offset;
        unsigned int pdentry1 = va1->pd_offset;
        unsigned int vp1 = vaddr1/NBPG;
        pd_t *pdent1  =(pd_t *) ((proctab[frm_tab[minIter].fr_pid].pdbr) + pdentry1*4 );
        pt_t *pageEntry1 = (pt_t *)((pdent1->pd_base)*NBPG + ptentry1*4 );

	if(pageEntry1->pt_dirty == 1){
		//kprintf("\nDirty %d", minIter);
		page_removal(minIter, 1);
	}
	frm_tab[minIter].fr_refcnt += 1;
	free_frm(minIter);
	if(debug_mode){
		kprintf("\nFrame replaced %d (LFU)", minIter + FRAME0);
	}
	return(minIter);
}
