/*-----------------------------------------
// Author: Ryan Campbell
//
// REMARKS: Implement a Task struct along with a
// Queue-like structure created out of an array
// of Task struct pointers.
//-----------------------------------------*/

#ifndef SCHEDULING_TASK_H
#define SCHEDULING_TASK_H

#include <stdio.h>
#include <time.h>

typedef struct timespec timespec;

typedef struct {
    char *task_name;
    int task_type;
    int task_length;
    int odds_of_IO;
    int priority;
    int first_run;
    int allotment;
    timespec first_run_time;
    timespec arrival_time;
    timespec completion_time;

} Task ;

/*------------------------------------------------------
// makeTasks
//
// PURPOSE: given a pointer to a file populated by tasks,
// makeTasks will create each task one by one, populating
// an array of Tasks.
// INPUT PARAMETERS:
//     Takes in a pointer to a task file as an input
// parameter.
// OUTPUT PARAMETERS:
//     Returns a pointer to the array of Tasks.  The array
// tasks must be destroyed with destroyTasks.
//------------------------------------------------------*/
Task* makeTasks(FILE* task_file);

/*------------------------------------------------------
// getTask
//
// PURPOSE: Given an array of Tasks, this function returns
// a pointer to the desired indexed Task.
// INPUT PARAMETERS:
//     Takes in a pointer to an array of Tasks, along with
// the array index that is desired to be returned.
// OUTPUT PARAMETERS:
//     Returns a pointer to the desired Task if it exists,
// else return NULL.
//------------------------------------------------------*/
Task* getTask(Task* tasks, int index);

/*------------------------------------------------------
// addTask
//
// PURPOSE: Given a pointer to an array of Tasks, along
// with a pointer to a new Task, this function adds the
// task to the end of the array of Tasks.
// INPUT PARAMETERS:
//     Takes in a a pointer to an array of Tasks that
// you would like the new Task pointer to be added to.
//------------------------------------------------------*/
void addTask(Task* tasks, Task* newTask);

/*------------------------------------------------------
// getNumberOfTasks
//
// PURPOSE: Returns the global variable that is used to
// keep track of how many tasks are currently in the array
// of Tasks.
// OUTPUT PARAMETERS:
//     Returns an integer which is the number of Task
// currently in the array of Tasks.
//------------------------------------------------------*/
int getNumberOfTasks();

/*------------------------------------------------------
// compareTasks
//
// PURPOSE: This function compares to Tasks. It is used
// to sort the array of Tasks in Shortest Job First order.
// INPUT PARAMETERS:
//     Takes in two void pointers that will be used to
// pass the Tasks we would like to compare.
// OUTPUT PARAMETERS:
//     If taskA has a shorter length, return a negative,
// if taskB is shorter, return a positive number, if equal,
// return 0.
//------------------------------------------------------*/
int compareTasks(const void *voidA, const void* voidB);


/*------------------------------------------------------
// copyTask
//
// PURPOSE: Given pointers to two different Tasks, this
// function will copy the contents of the source Task to
// the destination Task.
// INPUT PARAMETERS:
//     Takes in two pointers.  One for the destination Task,
// and one for the source Task.
//------------------------------------------------------*/
void copyTask(Task* destination, Task* source);

/*------------------------------------------------------
// runIO
//
// PURPOSE: This function looks at a Task and using random
// number generation, determines whether or not the calling
// Task will perform IO.
// INPUT PARAMETERS:
//     Takes in a pointer to a Task as an input parameter.
// OUTPUT PARAMETERS:
//     Returns the length of time in nanoseconds to perform
// IO.  If the Task does not perform IO, return 0.
//------------------------------------------------------*/
long runIO(Task* task );

/*------------------------------------------------------
// printTask
//
// PURPOSE: takes a pointer to a Task and
// neatly prints the details.
//
// INPUT PARAMETERS:
//     Takes in a pointer to a tasks.
//------------------------------------------------------*/
void printTask(Task* task);

/*------------------------------------------------------
// completedTasks
//
// PURPOSE: Allocates memory to store the information\
// about the completed Tasks.
// OUTPUT PARAMETERS:
//     Returns a pointer to an array of Task pointers.
//------------------------------------------------------*/
Task* completedTasks();

/*------------------------------------------------------
// printTasks
//
// PURPOSE: takes a pointer to an array of Tasks and
// neatly prints the details of each task.
//
// INPUT PARAMETERS:
//     Takes in a pointer to an array of Tasks.
//------------------------------------------------------*/
void printTasks(Task* tasks);

/*------------------------------------------------------
// destroyTasks
//
// PURPOSE: this is the shell's main that is run when we
// shell is executed. It reads the arguments passed to the
// program by the user, looking for a single script file
// to read.  If the file can be read, it is passed to
// to the processScript method for processing.
// INPUT PARAMETERS:
//     Takes in arguments from standard I/O given by the
// user.
//------------------------------------------------------*/
void destroyTasks(Task* tasks);

#endif /*SCHEDULING_TASK_H */
