/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  STATWORD ps;
  disable(ps);
  if(source <0 || source > BSELIMIT - 1 || npages < 1 ||npages > NPAGES || virtpage < NBPG){
        restore(ps);
        return(SYSERR);
}
  if(bsm_tab[source].bs_private == PRIVATE){  /* Creating a mapping in a private BS */
        restore(ps);
        return(SYSERR);
    }
  if(bsm_tab[source].bs_status == BSM_MAPPED && bsm_tab[source].bs_limit < npages){
      restore(ps); 
      return(SYSERR);
  }
  bsm_map(currpid,virtpage,source,npages); /* Call bsm_map otherise */ 
  bsm_tab[source].bs_private = SHARED;
  restore(ps);
  return OK;
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage)
{
  STATWORD ps;
  disable(ps);
  int i;
  if(virtpage <NBPG){
		restore(ps);
		return(SYSERR);
 }
  bsm_unmap(currpid, virtpage);
  for(i=0;i<BSELIMIT;i++){
        if(bsm_tab[i].bs_vpno[currpid] == virtpage ){
            bsm_tab[i].bs_pid[currpid] = 0;
            bsm_tab[i].bs_npages[currpid] = 0;
            bsm_tab[i].bs_vpno[currpid] = 0;
            break;
        }
    }
    if(bsm_tab[i].bs_private == PRIVATE){
        bsm_tab[i].bs_status = BSM_UNMAPPED;
    }
    else{
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
  restore(ps);
  return OK;
}
