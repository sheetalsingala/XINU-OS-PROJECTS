#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {

  /* release the backing store with ID bs_id */
  STATWORD ps;
  disable(ps);
  
  if(bs_id<0 || bs_id>BSELIMIT){      /* Validity of bst_id */
      restore(ps);
      return(SYSERR);
  }
  if((bsm_tab[bs_id].bs_private == SHARED) & (bsm_tab[bs_id].bs_status == BSM_PRESENT)){
       bsm_tab[bs_id].bs_status == BSM_UNMAPPED;
       free_bsm(bs_id);
       restore(ps);
       return(OK);
  }
    restore(ps);
    return(SYSERR);

}

