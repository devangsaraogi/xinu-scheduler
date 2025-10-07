/* implementaion of setschedclass() and getschedclass() */

#include <sched.h>

int sched_class = EXPDISTSCHED;

/* setter & getter */

void setschedclass(int s){
    sched_class = s;
}

int getschedclass(){
    return sched_class;
}