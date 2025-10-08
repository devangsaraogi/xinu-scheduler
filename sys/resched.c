/* resched.c  -  resched */
/* 
   D: KT
   XINU Scheduler
   core logic of context switching and scheduling mechanism
   suspends the currently running process 
   selects the highest-priority process from the ready queue to run next
*/

/* 
   D: KT
   XINU Scheduler
   proceeds in three logical steps:
   1. a check for necessity
   2. preparing the old process
   3. selecting/switching to the new process
*/

/*
   D: KT
   Some important global variables and functions used in this file:

   currSP: a global variable to hold the real stack pointer of the currently running process
   ctxsw(...): external function that perfroms the low-level context switch
   currpid: gloabl variable that holds the process ID of the currently running process
   proctab[]: global array of process table entries
   rdyhead and rdytail: global variables that points to the head and tail of the ready queue
   lastkey(...): function that returns the priority of the highest-priority process in the ready queue  
   pstate: field in the proctab[] that indicates the current state of the process
   pprio: field in the proctab[] that indicates the priority of the process
   PRCURR and PRREADY: constants representing the states of a process (currently running and ready)
*/

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sched.h>

unsigned long currSP;	/* REAL sp of current process */

extern int ctxsw(int, int, int, int);
/*-----------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */
int resched()
{
	register struct	pentry	*optr;	/* pointer to old process entry */
	register struct	pentry	*nptr;	/* pointer to new process entry */

	if (sched_class == EXPDISTSCHED) {
	    optr = &proctab[currpid];

		if (optr->pstate == PRCURR && currpid != NULLPROC) {
			optr->pstate = PRREADY;
			insert(currpid, rdyhead, optr->pprio);
		}

		/* if ready queue empty, run NULLPROC */
		if (q[rdyhead].qnext == rdytail) {
			int old = currpid;
			currpid = NULLPROC;
			proctab[currpid].pstate = PRCURR;
#ifdef	RTCLOCK
			preempt = QUANTUM;
#endif
			ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&proctab[NULLPROC].pesp, (int)proctab[NULLPROC].pirmask);
			return OK;
		}

		int selected_proc = expdist_pick_next();

		if (selected_proc == q[rdyhead].qnext) {
			selected_proc = getfirst(rdyhead);
		} else if (selected_proc == q[rdytail].qprev) {
			selected_proc = getlast(rdytail);
		} else {
			dequeue(selected_proc);
		}

		nptr = &proctab[selected_proc];
		nptr->pstate = PRCURR;		/* mark it currently running */
		int old = currpid;
		currpid = selected_proc;
#ifdef	RTCLOCK
		preempt = QUANTUM;		/* reset preemption counter	*/
#endif
		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
		return OK;
	}

	if (sched_class == LINUXSCHED) {
		optr = &proctab[currpid];

		if (optr->pstate == PRCURR && currpid != NULLPROC) {
			/* save remaining counter of old process */
			optr->lnx_counter = preempt;

			/* if old process exhausted its quantum
			   set goodness and counter to 0 and pexhaust to 1
			   else update goodness and pexhaust to 0 */
			if(optr->lnx_counter <= 0) {
				optr->lnx_counter = 0;
				optr->lnx_goodness = 0; 
				optr->lnx_pexhaust = 1; 
			}
			else {
				optr->lnx_goodness = optr->pprio + optr->lnx_counter;
				optr->lnx_pexhaust = 0;
			}

			/* insert old process back into ready queue */
			optr->pstate = PRREADY;
			insert(currpid, rdyhead, optr->pprio);
		}

		/* if ready queue empty, run NULLPROC */
		if (q[rdyhead].qnext == rdytail) {
			int old = currpid;
			currpid = NULLPROC;
			proctab[currpid].pstate = PRCURR;
#ifdef	RTCLOCK
			preempt = QUANTUM;
#endif
			ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&proctab[NULLPROC].pesp, (int)proctab[NULLPROC].pirmask);
			return OK;
		}
		/* pick next process to run */
		int selected_proc = lnx_pick_next();
		
		if (selected_proc == NULLPROC) {
			currpid = NULLPROC;
			proctab[currpid].pstate = PRCURR;
#ifdef	RTCLOCK
			preempt = QUANTUM;
#endif
			ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&proctab[NULLPROC].pesp, (int)proctab[NULLPROC].pirmask);
			return OK;
		}

		/* dequeue selected process from ready queue */
		if (selected_proc == q[rdyhead].qnext) {
			selected_proc = getfirst(rdyhead);
		} else if (selected_proc == q[rdytail].qprev) {
			selected_proc = getlast(rdytail);
		} else {
			dequeue(selected_proc);
		}

		nptr = &proctab[selected_proc];
		nptr->pstate = PRCURR;		/* mark it currently running */
		int old = currpid;
		currpid = selected_proc;
#ifdef	RTCLOCK
		preempt = (nptr-> lnx_counter > 0) ? nptr-> lnx_counter : QUANTUM;		/* reset preemption counter	*/
#endif
		ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
		return OK;
	}


	/* no switch needed if current process priority higher than next*/

	/* D: KT >> step 1: check for neccesity */
	/* 
	   D: KT
	   optr = &proctab[currpid]: gets a pointer (optr) to the current process's entry in proctab[]
	   (optr->pstate == PRCURR): checks if the current process is still in the "currentlt running" state (PRCURR)
	   lastkey(rdytail): retrieves the highest priority of any process currently on the ready queue
	   lastkey(rdytail) < optr->pprio: compares the next ready process's priority with the current process's priority
	   returns OK if the current process has a higher priority than any other process in the ready queue i.e. no context switch is needed
	*/
	if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
	   (lastkey(rdytail)<optr->pprio)) {
		return(OK);
	}
	
	/* force context switch */
	/* D: KT >> step 2: preparing the old process */
	/* 
	   D: KT
	   if a switch is required, the old process must be put back on the ready queue (obviosuly if it is still in the PRCURR state)
	   (optr->pstate == PRCURR): this check ensures the process is only put back on the ready queue if its still marked PRCURR (preempted)
	                             if its state was already set to something else (PRSLEEP, PRWAIT etc.) then it should not be put back on the ready queue
	   optr->pstate = PRREADY: sets the state of the old process to PRREADY (ready to run)
	   insert(currpid, rdyhead, optr->pprio): inserts the old process back into the ready queue based on its priority
	*/
	if (optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
	}

	/* remove highest priority process at end of ready list */
	/* D: KT >> step 3: selecting/switching to new process */
	/* 
	   D: KT
	   this is where the actual switch happens
	   currpid = getlast(rdytail): get PID of highest priority process from the ready queue
	   	                           update global currpid to this new PID
	   nptr = &proctab[...]: pointer (nptr) to the new process's entry in proctab[]
	   nptr->pstate = PRCURR: set the state of the new process to PRCURR (currently running)
	   preempt = QUANTUM: reset the preemption counter (if RTCLOCK is defined)
	   ctxsw(...): perform the low-level context switch between the old process and the new process
	*/
	nptr = &proctab[ (currpid = getlast(rdytail)) ];
	nptr->pstate = PRCURR;		/* mark it currently running	*/
#ifdef	RTCLOCK
	preempt = QUANTUM;		/* reset preemption counter	*/
#endif
	
	ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
	
	/* The OLD process returns here when resumed. */
	return OK;
}
