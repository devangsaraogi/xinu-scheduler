/**
 * sys/lxschedt.c - LNXSCHED selector
 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sched.h>


/* flag to indicate active epoch */
static int epoch_active = 0;

static void lnx_epoch_start(void) {
    // kprintf("\n--- New Epoch ---\n");

    /* one pass of proctab to update counters, goodness and pexhaust */
    for (int it=0; it < NPROC; it++) {
        struct pentry *pptr = &proctab[it];
        if (pptr->pstate == PRFREE || it == NULLPROC) {
            pptr->lnx_counter = 0;
            pptr->lnx_goodness = 0;
            pptr->lnx_pexhaust = 0;
            
            continue;
        }

        /* calculate new counter */
        int new_counter;
        if (pptr->lnx_new) {
            /* new process */
            new_counter = pptr->pprio;
            pptr->lnx_new = 0;
        } else {
            /* old process */
            if (pptr->lnx_pexhaust) {
                /* process exhausted its quantum in the last epoch */
                new_counter = pptr->pprio;
            } else {
                /* process did not exhaust its quantum in the last epoch */
                new_counter = pptr->pprio + (pptr->lnx_counter / 2);
            }
        }

        /* update */
        pptr->lnx_counter = (new_counter > 0) ? new_counter : 0;
        pptr->lnx_goodness = (pptr->lnx_counter > 0) ? (pptr->pprio + pptr->lnx_counter) : 0;
        pptr->lnx_pexhaust = 0; /* reset pexhaust because new epoch */

        // kprintf("[epoch] pid=%d prio=%d new_cnt=%d good=%d new=%d\n", it, pptr->pprio, pptr->lnx_counter, pptr->lnx_goodness, pptr->lnx_new);
    }

    epoch_active = 1;
}

/* pick ready process with max goodness */
static int lnx_pick_max_goodness(void) {
    int best_proc = -1;
    int max_goodness = -0x7FFFFFFF;

    /* one pass of the ready queue to find max goodness */
    for (int it = q[rdyhead].qnext; it != rdytail; it = q[it].qnext) {
        struct pentry *pptr = &proctab[it];
        // kprintf("[pick-scan] pid=%d prio=%d cnt=%d good=%d state=%d\n", it, pptr->pprio, pptr->lnx_counter, pptr->lnx_goodness, pptr->pstate);
        
        if (pptr->pstate != PRREADY) continue;
        if (pptr->lnx_goodness > 0 && pptr->lnx_goodness > max_goodness) {
            max_goodness = pptr->lnx_goodness;
            best_proc = it;
        }
    }
    return best_proc; 
    /* should never return -1 */
}

int lnx_pick_next(void) {

    /* start epoch if not active */
    if (!epoch_active) {
        lnx_epoch_start();
    }

    int next_proc = lnx_pick_max_goodness();

    /* if next_proc = -1; initiate new epoch and kick by choosing next process */
    if (next_proc < 0) {
        // kprintf("[pick] none>0, starting new epoch\n");
        lnx_epoch_start();
        next_proc = lnx_pick_max_goodness();
    }

    /* if next_proc is still -1 return NULL PROCESS */
    if(next_proc < 0) return NULLPROC;
    
    // kprintf("[pick] next=%d\n", next_proc);
    
    return next_proc;
}