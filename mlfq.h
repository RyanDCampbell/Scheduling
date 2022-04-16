

#ifndef SCHEDULING_MLFQ_H
#define SCHEDULING_MLFQ_H

#include "task.h"
#include <time.h>

typedef struct {

    /*The number of tasks in each queue*/
    int num_priority_0; /* Lowest Priority */
    int num_priority_1;
    int num_priority_2; /* Highest Priority */

    /*The current task index we are working on (Round Robin) */
    int curr_priority_0;
    int curr_priority_1;
    int curr_priority_2;

    /* Pointer to each queue of tasks */
    Task* priority_0;
    Task* priority_1;
    Task* priority_2;

}Mlfq;

/*------------------------------------------------------
// makeMLFQ
//
// PURPOSE: This method is used to create and initialize
// a new MLFQ.  It takes a pointer to an array of Tasks,
// along with the number of Tasks in the array. It adds
// every Task to the highest priority queue, and sets the
// medium and low priority queues to empty.
//
// INPUT PARAMETERS:
//     Takes in a pointer to the array of Tasks, along with\
// number of Tasks in the array.
// OUTPUT
//      Returns a pointer to the newly created MLFQ.
//------------------------------------------------------*/
Mlfq* makeMLFQ(Task* tasks, int num_tasks);

/*------------------------------------------------------
// getTaskMLFQ
//
// PURPOSE: This method gets the next Task that should be
// run based on the specified MLFQ rules.  The next highest
// priority Task in the queue will be returned.
//
// INPUT PARAMETERS:
//     Must be passed a pointer to the MLFQ.
// OUTPUT
//      Returns a Task pointer to the next Task to run.
//------------------------------------------------------*/
Task* getTaskMLFQ(Mlfq* mlfq);

/*------------------------------------------------------
// getTaskQueueMLFQ
//
// PURPOSE: This method gets the Task from the front of
// the desired priority queue.
//
// INPUT PARAMETERS:
//     This method needs to be passed a pointer to the
// MLFQ along with which priority of Task to return.
// OUTPUT
//      Returns a pointer to desired task to run.
//------------------------------------------------------*/
Task *getTaskQueueMLFQ(Mlfq *mlfq, int queue_number);

/*------------------------------------------------------
// addTaskToQueue
//
// PURPOSE: This method adds a new task to the end of the
// queue of tasks.
//
// INPUT PARAMETERS:
//     Must be passed a pointer to the array of Task pointers,
// a pointer to the new Task to be added to the queue, along with
// the index of the last Task in the queue.
//------------------------------------------------------*/
void addTaskToQueue(Task *tasks, Task *newTask, int tasks_length);

/*------------------------------------------------------
// increasePriorities
//
// PURPOSE: This method increases the priority of all Tasks.
// The method moves all Tasks from the medium and low priority
// queues to the high priority queue.  It then resets the
// time allotment of each task.
//
// INPUT PARAMETERS:
//     Must be passed a pointer to the MLFQ.
//------------------------------------------------------*/
void increasePriorities(Mlfq* mlfq);

/*------------------------------------------------------
// addTaskMLFQ
//
// PURPOSE: This method Takes in a new Task, and adds to
// back to the appropriate priority queue depending on
// the current state of the Task, taking into account how
// much allotment time the Task has for the current queue.
//
// INPUT PARAMETERS:
//     Takes in a pointer to the MLFQ, along with a pointer
// to the Task to add to the MLFQ.
//------------------------------------------------------*/
void addTaskMLFQ(Mlfq* mlfq, Task* newTask);

/*------------------------------------------------------
// getNumberOfTasksMLFQ
//
// PURPOSE: This method returns the sum of the number of
// Tasks in each priority queue.  This sum is equal to the
// number of Tasks in the MLFQ.
//
// INPUT PARAMETERS:
//     Must be passed a pointer to the MLFQ
// OUTPUT
//      Returns the number of Tasks in the MLFQ
//------------------------------------------------------*/
int getNumberOfTasksMLFQ(Mlfq* mlfq);

#endif /*SCHEDULING_MLFQ_H */
