

#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#include "mlfq.h"

#define task_input_length 100
#define quantum_length 50 /* Microseconds */
#define allotment_length 200 /*Microseconds */

#define high_priority 2
#define medium_priority 1
#define low_priority 0

pthread_mutex_t mlfq_lock = PTHREAD_MUTEX_INITIALIZER;

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
Mlfq *makeMLFQ(Task *tasks, int num_tasks) {

    int i;
    Mlfq *mlfq;

    /* printf("Making MLFQ... \n"); */

    mlfq = malloc(sizeof(Mlfq));
    assert(mlfq != NULL);

    /* allocating enough space to store (task_input_length) tasks in each Queue. */
    mlfq->priority_0 = malloc(task_input_length * sizeof(Task));
    mlfq->priority_1 = malloc(task_input_length * sizeof(Task));
    mlfq->priority_2 = tasks; /* We can use the array of tasks pointer because
                                 they all enter at the same time. We assume full
                                 control of the array */

    /* Setting default values */
    mlfq->curr_priority_0 = 0;
    mlfq->curr_priority_1 = 0;
    mlfq->curr_priority_2 = 0;

    /* Setting default values */
    mlfq->num_priority_0 = 0;
    mlfq->num_priority_1 = 0;
    mlfq->num_priority_2 = num_tasks;

    for (i = 0; i < num_tasks; i++) {
        mlfq->priority_2->priority = high_priority;
    }

    mlfq->priority_1[0].task_name = NULL;
    mlfq->priority_0[0].task_name = NULL;

    return mlfq;
}

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
Task *getTaskMLFQ(Mlfq *mlfq) {

    Task *result = NULL;
    Task *queue = NULL;
    int i;
    int num_in_queue = 0;
    int curr_queue = 0;

    pthread_mutex_lock(&mlfq_lock);

    if (mlfq->num_priority_2 > 0) {
        queue = mlfq->priority_2;
        num_in_queue = mlfq->num_priority_2;
        curr_queue = 2;
    } else if (mlfq->num_priority_1 > 0) {
        queue = mlfq->priority_1;
        num_in_queue = mlfq->num_priority_1;
        curr_queue = 1;
    } else if (mlfq->num_priority_0 > 0) {
        queue = mlfq->priority_0;
        num_in_queue = mlfq->num_priority_0;
        curr_queue = 0;
    }

    if (queue != NULL) {
        result = malloc(sizeof(Task));
        assert(result != NULL);

        copyTask(result, &queue[0]);

        for (i = 0; i < num_in_queue; i++) {
            queue[i] = queue[i + 1];
        }

        if (curr_queue == 2) {
            mlfq->num_priority_2--;
        } else if (curr_queue == 1) {
            mlfq->num_priority_1--;
        } else {
            mlfq->num_priority_0--;
        }
    }

    pthread_mutex_unlock(&mlfq_lock);

    return result;
}

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
Task *getTaskQueueMLFQ(Mlfq *mlfq, int queue_number) {

    Task *result = NULL;
    Task *queue = NULL;
    int i;
    int num_in_queue = 0;
    int curr_queue = 0;

    /*pthread_mutex_lock(&mlfq_lock); */

    if (queue_number == low_priority) {
        queue = mlfq->priority_0;
        num_in_queue = mlfq->num_priority_0;
        curr_queue = 0;
    } else if (queue_number == medium_priority) {
        queue = mlfq->priority_1;
        num_in_queue = mlfq->num_priority_1;
        curr_queue = 1;
    } else if (queue_number == high_priority) {
        queue = mlfq->priority_2;
        num_in_queue = mlfq->num_priority_2;
        curr_queue = 2;
    }

    if (queue != NULL) {

        if (queue->task_name != NULL) {
            result = malloc(sizeof(Task));
            assert(result != NULL);

            copyTask(result, &queue[0]);

            for (i = 0; i < num_in_queue; i++) {
                queue[i] = queue[i + 1];
            }

            if (curr_queue == 2) {
                mlfq->num_priority_2--;
            } else if (curr_queue == 1) {
                mlfq->num_priority_1--;
            } else {
                mlfq->num_priority_0--;
            }
        }
    }

    /*pthread_mutex_unlock(&mlfq_lock); */
    return result;
}


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
void addTaskToQueue(Task *tasks, Task *newTask, int tasks_length) {
    assert(tasks != NULL);
    assert(newTask != NULL);

    /* move over the null terminated task */
    copyTask(&tasks[tasks_length + 1], &tasks[tasks_length]);
    copyTask(&tasks[tasks_length], newTask);

    tasks[tasks_length + 1].task_name = NULL;
}


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
void increasePriorities(Mlfq *mlfq) {

    int i;
    Task *temp = NULL;
    Task *curr_queue = NULL;

    pthread_mutex_lock(&mlfq_lock);
    curr_queue = mlfq->priority_0;

     /* printTasks(mlfq->priority_0);  */

    temp = getTask(curr_queue, 0);
    while (temp->task_name != NULL) {

        temp->priority = high_priority;
        addTask(mlfq->priority_2,temp);
        mlfq->num_priority_2++;
        temp = getTask(curr_queue, 0);
    }

    curr_queue = mlfq->priority_1;
    temp = getTask(curr_queue, 0);
    while (temp->task_name != NULL) {
        temp->priority = high_priority;
        addTask(mlfq->priority_2,temp);
        mlfq->num_priority_2++;
        temp = getTask(curr_queue, 0);
    }

    i = 0;
    while (temp->task_name != NULL) {
        mlfq->priority_2[i].allotment = allotment_length;
        mlfq->priority_2[i].priority = high_priority;
        i++;
    }

    pthread_mutex_unlock(&mlfq_lock);
}



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
void addTaskMLFQ(Mlfq *mlfq, Task *newTask) {

    Task *queue = NULL;
    int num_in_queue = 0;

    pthread_mutex_lock(&mlfq_lock);

    if (newTask->priority == high_priority) {
        queue = mlfq->priority_2;
        num_in_queue = mlfq->num_priority_2;
    } else if (newTask->priority == medium_priority) {
        queue = mlfq->priority_1;
        num_in_queue = mlfq->num_priority_1;
    } else if (newTask->priority == low_priority) {
        queue = mlfq->priority_0;
        num_in_queue = mlfq->num_priority_0;
    }

    if (queue != NULL) {

        if (newTask->priority == high_priority) {

            if (newTask->allotment > 0) {
                addTask(queue, newTask);
                mlfq->num_priority_2++; /* Investigate */
            } else {
                newTask->allotment = allotment_length;
                newTask->priority = medium_priority;
                addTaskToQueue(mlfq->priority_1, newTask, mlfq->num_priority_1); /*Investigate */
                mlfq->num_priority_1++;
            }
        } else if (newTask->priority == medium_priority) {

            if (newTask->allotment > 0) {
                addTask(queue, newTask);
                mlfq->num_priority_1++;
            } else {
                newTask->allotment = allotment_length;
                newTask->priority = low_priority;
                addTaskToQueue(mlfq->priority_0, newTask, mlfq->num_priority_0);
                mlfq->num_priority_0++;
            }

        } else if (newTask->priority == low_priority) {

            newTask->allotment = allotment_length;
            addTaskToQueue(mlfq->priority_0, newTask, mlfq->num_priority_0);
            mlfq->num_priority_0++;

        }

        copyTask(&queue[num_in_queue + 1], &queue[num_in_queue]);
        copyTask(&queue[num_in_queue], newTask);
        num_in_queue++;
    }

    pthread_mutex_unlock(&mlfq_lock);
}

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
int getNumberOfTasksMLFQ(Mlfq *mlfq) {

    int result = 0;

    pthread_mutex_lock(&mlfq_lock);

    result += mlfq->num_priority_0;
    result += mlfq->num_priority_1;
    result += mlfq->num_priority_2;
    pthread_mutex_unlock(&mlfq_lock);

    return result;
}

