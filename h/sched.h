#ifndef _SCHED_H
#define _SCHED_H_


/* public scheduler class */
#define EXPDISTSCHED 1
#define LINUXSCHED 2


/* scheduler class */
extern int sched_class;


/* apis */
void setschedclass(int s);
int getschedclass(void);

/* EXPDISTCHED helper */
int expdist_pick_next(void);

#endif