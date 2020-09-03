/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>


/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()    
{
  //kprintf("\nPFINT");
  int store = -1;
  int pageth = -1;
  int flag=0;         /* flag = 1, create a PT and update PD. flag = 0, create an entry in PT */
  STATWORD ps;
  disable(ps);
  unsigned long vaddr= read_cr2();   /* Read virtual address from CR2 */
  virt_addr_t *v_a;
  v_a = (virt_addr_t*)&vaddr;	/* Obtain page number, page directory and offset */
  unsigned int ptentry = v_a->pt_offset;
  unsigned int pdentry = v_a->pd_offset;
  unsigned int vp = vaddr/NBPG;   /* Virtual page number */

  pd_t *pdent  =(pd_t *) ((proctab[currpid].pdbr) + pdentry*4 );

  bsm_lookup(currpid,vaddr, &store, &pageth);
  if(store == -1){
    kill(currpid);    /* Kill process and return SYSERR */
    restore(ps);
    return(SYSERR);   /* Invalid page */
  }

      if(pdent->pd_pres == 0){       /* Create a PT, obtain entry number in PT from vaddr and update values*/
            int ent = initPT(currpid);   /* Init a Page table */
            pdent->pd_pres = 1;
            pdent->pd_base = FRAME0 + ent;     /* PT created */
            pdent->pd_write = 1; /* Page table was accessed */
            }
	    int frameent;
      get_frm(&frameent);   /* Either the page is not present or PT is not present, so get a free frame */
 
      if(page_replace_policy == LFU){
                frm_tab[frameent].fr_refcnt += 1;
        }

      if(prhead == PRHEAD){ /*Queue inserting */        /*Only firsttime, inset at head */
            prhead = frameent;    /* Set head of PR queue */
            frm_tab[frameent].fr_pframe = PRHEAD;
            frm_tab[frameent].fr_nframe = PRTAIL;
            prtail = frameent;
            currpoint = prhead;
      }
      else{
            frm_tab[prtail].fr_nframe = frameent;
            frm_tab[frameent].fr_pframe = prtail;
            frm_tab[prhead].fr_pframe = frameent;
            frm_tab[frameent].fr_nframe = prhead;
            prtail = frameent;
      }

            frm_tab[frameent].fr_status = FRM_MAPPED;

            pt_t *ptent = (pt_t *)((pdent->pd_base)*NBPG + ptentry*4 );
            ptent->pt_pres = 1;
            ptent->pt_base = FRAME0 + frameent;   /* Address in the memory */
            ptent->pt_write = 1;  
	          ptent->pt_user = 0;
            ptent->pt_pwt = 0;
            ptent->pt_pcd = 0;
            ptent->pt_acc = 0;
            ptent->pt_dirty = 0;
            ptent->pt_mbz = 0;
            ptent->pt_global = 0;
            ptent->pt_avail = 3;    
	          frm_tab[pdent->pd_base - FRAME0].fr_refcnt += 1;
            /*Add a mapping in frm_tab*/  
            frm_tab[frameent].fr_pid = currpid;    
            frm_tab[frameent].fr_type = FR_PAGE;   
            frm_tab[frameent].fr_vpno = vp;
            read_bs((char *)((FRAME0+frameent)*NBPG), store, pageth);	/* Copy everything to the frame */
      
   
  return OK;
}
