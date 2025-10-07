/* insert.c  -  insert */

#include <conf.h>
#include <kernel.h>
#include <q.h>

/*------------------------------------------------------------------------
 * insert.c  --  insert an process into a q list in key order
 *------------------------------------------------------------------------
 */

/* 
   D: KT
   maintains a doubly-linked sorted circular key (sorted by priority (key))
   proc: process ID of the process to be inserted
   head: index of the head of the queue
   key: priority of the process to be inserted
   q[]: global array representing the queue, where each element has fields qnext, qprev, and qkey
   next: variable used to traverse the queue
   prev: variable used to keep track of the previous element in the queue during traversal
*/

int insert(int proc, int head, int key)
{
	int	next;			/* runs through list		*/
	int	prev;

	next = q[head].qnext;
	while (q[next].qkey < key)	/* tail has maxint as key	*/
		next = q[next].qnext;
	
	/* D: KT
		insert between prev and next
	 	sets up the new node q[proc] to point to its neighbors and assigns its keys 
	*/
	q[proc].qnext = next;
	q[proc].qprev = prev = q[next].qprev;
	q[proc].qkey  = key;

	/* D: KT
		links neighbors to new node
	*/
	q[prev].qnext = proc;
	q[next].qprev = proc;
	
	return(OK);
}
