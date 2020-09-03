/* paging.h */

typedef unsigned int	 bsd_t;

/* Structure for a page directory entry */

typedef struct {

  unsigned int pd_pres	: 1;		/* page table present?		*/
  unsigned int pd_write : 1;		/* page is writable?		*/
  unsigned int pd_user	: 1;		/* is use level protection?	*/
  unsigned int pd_pwt	: 1;		/* write through cachine for pt?*/
  unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
  unsigned int pd_acc	: 1;		/* page table was accessed?	*/
  unsigned int pd_mbz	: 1;		/* must be zero			*/
  unsigned int pd_fmb	: 1;		/* four MB pages?		*/
  unsigned int pd_global: 1;		/* global (ignored)		*/
  unsigned int pd_avail : 3;		/* for programmer's use		*/
  unsigned int pd_base	: 20;		/* location of page table?	*/
} pd_t;

/* Structure for a page table entry */

typedef struct {

  unsigned int pt_pres	: 1;		/* page is present?		*/
  unsigned int pt_write : 1;		/* page is writable?		*/
  unsigned int pt_user	: 1;		/* is use level protection?	*/
  unsigned int pt_pwt	: 1;		/* write through for this page? */
  unsigned int pt_pcd	: 1;		/* cache disable for this page? */
  unsigned int pt_acc	: 1;		/* page was accessed?		*/
  unsigned int pt_dirty : 1;		/* page was written?		*/
  unsigned int pt_mbz	: 1;		/* must be zero			*/
  unsigned int pt_global: 1;		/* should be zero in 586	*/
  unsigned int pt_avail : 3;		/* for programmer's use, 0 for kernel*/
  unsigned int pt_base	: 20;		/* location of page?		*/
} pt_t;

typedef struct{      
  unsigned int pg_offset : 12;		/* page offset			*/
  unsigned int pt_offset : 10;		/* page table offset		*/
  unsigned int pd_offset : 10;		/* page directory offset	*/
} virt_addr_t;

typedef struct{
  int bs_status;			/* MAPPED or UNMAPPED		*/
  int bs_pid[NPROC];				/* All process ids using this BS   */
  int bs_vpno[NPROC];				/* starting virtual page number for each process */
  int bs_npages[NPROC];			/* number of pages held by each process*/
  int bs_limit;             /* Set when a process first requests BS */
  int bs_sem;				/* unused	*/
  int bs_private;   /* Indicates whether it is private or not */
} bs_map_t;

typedef struct{
  int fr_status;			/* MAPPED or UNMAPPED		*/
  int fr_pid;				/* process id using this frame  */
  int fr_vpno;				/* corresponding virtual page no*/
  int fr_refcnt;			/* reference count		*/
  int fr_type;				/* FR_DIR, FR_TBL, FR_PAGE	*/
  int fr_dirty;
  int fr_nframe;
  int fr_pframe;
}fr_map_t;


//extern struct pr_t pr_tab[];
extern bs_map_t bsm_tab[];
extern fr_map_t frm_tab[];
/* Prototypes for required API calls */
SYSCALL xmmap(int, bsd_t, int);
SYSCALL xunmap(int);

/* given calls for dealing with backing store */

int get_bs(bsd_t, unsigned int);
SYSCALL release_bs(bsd_t);
SYSCALL read_bs(char *, bsd_t, int);
SYSCALL write_bs(char *, bsd_t, int);

/* Calls for dealing with frame/paging */

SYSCALL get_frm(int* avail);
SYSCALL free_frm(int i);
SYSCALL init_frm(void);

/*Initialization */

void initglobaltable(void);
int initPD(int pid);
int initPT(int pid);

SYSCALL va_pa_map(long paddr,long vaddr);
SYSCALL pfint();

/* Number of entries */
extern int framecount;
extern int prhead, prtail;
extern int currpoint;
extern int page_replace_policy;
extern int debug_mode;

#define NBPG		4096	/* number of bytes per page	*/
#define FRAME0		1024	/* zero-th frame		*/
#define NFRAMES 	1024	/* number of frames		*/
#define NPAGES   128 /* Number of pages in a frame */
#define NENTRIES 1024 /* Number of entries in a page */

#define BSM_UNMAPPED	0
#define BSM_MAPPED	1
#define BSM_PRESENT 2
#define FRM_UNMAPPED	0
#define FRM_MAPPED	1
#define FRM_UNINIT 2

#define FR_PAGE		0
#define FR_TBL		1
#define FR_DIR		2
#define FR_GPT    3

#define SC 3
#define LFU 4

#define BACKING_STORE_BASE	0x00800000
#define BACKING_STORE_UNIT_SIZE 0x00080000

#define BSELIMIT 16   /* Just like semaph[], proctab[] */

#define PRIVATE 1
#define SHARED 0
#define UNINIT -1

#define BASEHEAP 4096
#define ENTRYSIZE 4

#define DIRTY 1
#define NDIRTY 0

#define PRHEAD -1
#define PRTAIL -2
