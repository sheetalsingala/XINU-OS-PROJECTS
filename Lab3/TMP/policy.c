/* policy.c = srpolicy*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>


extern int page_replace_policy;
/*-------------------------------------------------------------------------
 * srpolicy - set page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL srpolicy(int policy)
{
  /* sanity check ! */
  if(policy != SC && policy != LFU){
	//kprintf("\nInvalid policy");
	return(SYSERR);
  }
  page_replace_policy = policy;
  //kprintf("To be implemented!\n");
  debug_mode = 1;

  return OK;
}

/*-------------------------------------------------------------------------
 * grpolicy - get page replace policy 
 *-------------------------------------------------------------------------
 */
SYSCALL grpolicy()
{
  return page_replace_policy;
}
