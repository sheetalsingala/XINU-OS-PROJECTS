/* paginginit.c = paginginit*/

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * paginginit - initalize global page tables and page directories
 *-------------------------------------------------------------------------
 */


void initglobaltable(void)          /* Initialize global page tables */
{
        int i;
        for(i=0;i<NBPG;i++){
            pt_t *ptentry =(pt_t *)( 1025 * NBPG + i*4);
            ptentry->pt_base = i;
            ptentry->pt_pres = 1;
            ptentry->pt_write = 1;
            ptentry->pt_user = 0;
            ptentry->pt_pwt = 0;
            ptentry->pt_pcd = 0;
            ptentry->pt_acc = 0;
            ptentry->pt_dirty = 0;
            ptentry->pt_mbz = 0;
            ptentry->pt_global = 1;
            ptentry->pt_avail = 0;
	}

        int j;
        for(j=1;j<5;j++){
        frm_tab[j].fr_type = FR_GPT;        /* Add entries for global page table */
	frm_tab[j].fr_status = FRM_MAPPED;
    }
}

int initPT(int pid)                /* Initialize a page table */
{      
        int ptiter = -1;
        get_frm(&ptiter);
        frm_tab[ptiter].fr_type = FR_TBL;        /* Add entries for global page table */
        frm_tab[ptiter].fr_status = FRM_MAPPED;
        frm_tab[ptiter].fr_pid = pid;  
    return(ptiter);
}

int initPD(int pid)               /* Initialize PD */
{
    int i;
    int pditer;
    get_frm(&pditer);
    pd_t *pdentry = (pd_t *)((FRAME0 + pditer) * NBPG);
    for(i=0;i<NENTRIES;i++){       /* set all other values */
        if(i<4){                  /* Setting the first 4 values that point to the global page table */
            pdentry->pd_base = (FRAME0 + i+1);
            pdentry->pd_pres = 1;       /* Global Page table present */
        }
        else{                       
            pdentry->pd_base = 0;       
            pdentry->pd_pres = 0;
        }
        pdentry->pd_write = 1;
        pdentry->pd_user = 0;
        pdentry->pd_pwt = 0;
        pdentry->pd_acc = 0;
        pdentry->pd_avail = 0;
        pdentry->pd_global = 0;
        pdentry->pd_mbz = 0;
        pdentry->pd_fmb = 0;
        pdentry->pd_pcd = 0;
	pdentry++;
    }
    frm_tab[pditer].fr_pid = pid;          /*Add an entry for process page directory - 1024 frame */
    frm_tab[pditer].fr_type = FR_DIR;   /*Indicates that it is a page directory */
    frm_tab[pditer].fr_status = FRM_MAPPED;
    return(pditer);
}
