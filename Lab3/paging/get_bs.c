#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {

  /* requests a new mapping of npages with ID map_id */
    if(bs_id <0 || bs_id > BSELIMIT){
	return(SYSERR);
    }
    if(npages <= 0 || npages > NPAGES) {    /* If npages is not valid */
      kprintf("\nnpages is invalid");
      return(SYSERR);
    }

    if(bsm_tab[bs_id].bs_private == PRIVATE ){    /* If BS is private */
      kprintf("\nTrying to access a private backing store");
      return(SYSERR);
    }
    if(bsm_tab[bs_id].bs_status == BSM_UNMAPPED){ /* Unmapped, return 128 */
      return npages;
    }

    if(bsm_tab[bs_id].bs_status == BSM_MAPPED & npages > bsm_tab[bs_id].bs_limit){  
      npages = bsm_tab[bs_id].bs_limit ;             /* size of new or existing BS is returned*/
    }
/*Return npages if none if this is true */
    

    return npages;

}


