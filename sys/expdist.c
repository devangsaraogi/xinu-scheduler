/**
 * sys/expdist.c - EXPDISTSCHED selector
 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <math.h>
#include <sched.h>

/* returns the oldest or frontmost PID at a given priority key */
static int pick_front_at_key(int target_key) {
    int it;
    for (it = q[rdyhead].qnext; it != rdytail; it = q[it].qnext) {
        if (q[it].qkey == target_key) {
            /* return the first or oldest process match with the target key */
            return it; 
        }
    }

    kprintf("expdist invariant violated, target_key: %d not found\n", target_key);

    /* 
        expdist_pick_next arrives at target_key by scanning the ready queue 
        fallback option, should never reach here
    */
    return q[rdytail].qprev; 
}

int expdist_pick_next(void) {

    /* ready queue is empty */
    if (q[rdyhead].qnext == rdytail) 
        return NULLPROC;

    double r = expdev(0.1);


    /* trackers for min/max priority and candidate key */
    int minP = 0x7FFFFFFF;
    int maxP = -0x7FFFFFFF;
    int candidate = 0x7FFFFFFF;

    /* one pass of the ready queue to find min/max and smallest key strictly greater than r */
    int it;
    for (it = q[rdyhead].qnext; it != rdytail; it = q[it].qnext) {
        int currP = q[it].qkey;
        if (currP < minP) minP = currP;
        if (currP > maxP) maxP = currP;
        if ((double)currP > r && currP < candidate) candidate = currP;
    }

    /* pick the candidate if found, else r is out of range [minP, maxP] and pick minP/maxP accordingly */
    int target_key = (candidate != 0x7FFFFFFF) ? candidate : ((r < (double)minP) ? minP : maxP);
    return pick_front_at_key(target_key);
}