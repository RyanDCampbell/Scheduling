
/*-----------------------------------------
// NAME: Ryan Campbell
// STUDENT NUMBER: 7874398
// COURSE: COMP 3430, SECTION: A01
// INSTRUCTOR: Franklin Bristow
// ASSIGNMENT: assignment #3
//
// REMARKS: Implement a Task struct along with a
// Queue-like structure created out of an array
// of Task struct pointers.
//-----------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "task.h"
#include "token.h"

#define task_input_length 100
#define task_file_length 500

#define quantum_length 50 /* Microseconds */
#define allotment_length 200 /*Microseconds */


static int number_of_tasks = 0;

pthread_mutex_t task_list_lock = PTHREAD_MUTEX_INITIALIZER;

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
Task* makeTasks(FILE* task_file) {


    int task_number;
    char **tokens;
    Task *tasks;
    char line[task_input_length];

    assert(task_file != NULL);

    tasks = malloc(task_file_length * sizeof(Task));
    assert(tasks != NULL);

    task_number = 0;
    while(fgets(line, task_input_length, task_file) != NULL) {

        tokens = tokenizeLine(line);

        tasks[task_number].task_name = strdup(tokens[0]);
        tasks[task_number].task_type = atoi(tokens[1]);
        tasks[task_number].task_length = atoi(tokens[2]);
        tasks[task_number].odds_of_IO = atoi(tokens[3]);
        tasks[task_number].allotment = allotment_length;
        tasks[task_number].first_run = 0;
        tasks[task_number].priority = 0;

        task_number++;
    }

    number_of_tasks = task_number;
    tasks[task_number].task_name = NULL; /*Signifies end of tasks, like a null terminator at the end of string*/

    return tasks;
}


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
Task* getTask(Task* tasks, int index){

    int i;
    Task *result;

    assert(tasks != NULL);
    result = NULL;
    pthread_mutex_lock(&task_list_lock);

    if(index >= number_of_tasks || index < 0){
        printf("Unable to get: Task %d, because %d id not a valid index location\n ", index, number_of_tasks);
    }
    else
    {
        result = malloc(task_file_length * sizeof(Task));
        assert(result != NULL);

        copyTask(result, &tasks[index]);

        for(i = index; i < number_of_tasks; i++)
        {
            tasks[i] = tasks[i+1];
        }
        number_of_tasks--;
    }

    pthread_mutex_unlock(&task_list_lock);

    return result;
}


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
void addTask(Task* tasks, Task* newTask){
    assert(tasks != NULL);
    assert(newTask != NULL);

    pthread_mutex_lock(&task_list_lock);

    copyTask( &tasks[number_of_tasks], newTask);

    number_of_tasks++;

    tasks[number_of_tasks].task_name = NULL;

    pthread_mutex_unlock(&task_list_lock);

}



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
int getNumberOfTasks(){

    return number_of_tasks;
}


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
int compareTasks(const void *voidA, const void* voidB){

    /*Only called when sorting the list of Tasks, already locked */
    Task taskA;
    Task taskB;


    assert(voidA != NULL);
    assert(voidB != NULL);

    taskA = *(Task*)voidA;
    taskB = *(Task*)voidB;

    return (taskA.task_length - taskB.task_length);
}


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
void copyTask(Task* destination, Task* source){

    /*Only called when sorting the list of Tasks, already locked */

    assert(destination != NULL);
    assert(source != NULL);

    destination->task_name = source->task_name;
    destination->task_length = source->task_length;
    destination->odds_of_IO = source->odds_of_IO;
    destination->task_type = source->task_type;
    destination->priority = source->priority;
    destination->first_run = source->first_run;
    destination->first_run_time = source->first_run_time;
    destination->arrival_time = source->arrival_time;
    destination->completion_time = source->completion_time;
    destination->allotment = source->allotment;

}

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
long runIO(Task* task ){

    int random_number;
    long result = 0;

    assert(task != NULL);

    random_number = rand()%100;

    /* If this evaluates to true, the do IO */
    if(random_number <= task->odds_of_IO){
        /* IO time is a random value between 0 and a time-slice */
        random_number = rand()%quantum_length;
        result = random_number;
    }
    return result;
}


/*------------------------------------------------------
// printTask
//
// PURPOSE: takes a pointer to a Task and
// neatly prints the details.
//
// INPUT PARAMETERS:
//     Takes in a pointer to a tasks.
//------------------------------------------------------*/
void printTask(Task* task) {

    assert(task != NULL);

    printf("\n********* Printing task *********\n");

    if(task->task_name!= NULL){

        printf("\nTask:\n task_name: \"%s\"\n task_type: %d\n task_length: %d microseconds left to complete\n odds_of_IO: %d%%\n",
               task->task_name, task->task_type, task->task_length, task->odds_of_IO);

    }
    printf("\n**** Finished printing tasks *****\n");
}

/*------------------------------------------------------
// completedTasks
//
// PURPOSE: Allocates memory to store the information\
// about the completed Tasks.
// OUTPUT PARAMETERS:
//     Returns a pointer to an array of Task pointers.
//------------------------------------------------------*/
Task* completedTasks() {

    Task *tasks = malloc(task_file_length * sizeof(Task));
    assert(tasks != NULL);
    return tasks;
}


/*------------------------------------------------------
// printTasks
//
// PURPOSE: takes a pointer to an array of Tasks and
// neatly prints the details of each task.
//
// INPUT PARAMETERS:
//     Takes in a pointer to an array of Tasks.
//------------------------------------------------------*/
void printTasks(Task* tasks) {

    int task_number;
    Task *curr_task;

    assert(tasks != NULL);

    task_number = 0;
    curr_task = &tasks[task_number];

    printf("\n********* Printing tasks *********\n");

    while(curr_task->task_name!= NULL){

        printf("\nTask %d:\n task_name: \"%s\"\n task_type: %d\n task_length: %d microseconds left to complete\n odds_of_IO: %d%%\n",
               task_number, curr_task->task_name, curr_task->task_type, curr_task->task_length, curr_task->odds_of_IO);

        task_number++;
        curr_task = &tasks[task_number];
    }
    printf("\n**** Finished printing tasks *****\n");
}


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
void destroyTasks(Task* tasks) {

    int task_number;
    Task *curr_task;

    assert(tasks != NULL);

    pthread_mutex_lock(&task_list_lock);

    task_number = 0;
    curr_task = &tasks[task_number];

    while(curr_task->task_name!= NULL){

        /* Free the malloc'd string and the task itself */
        free(curr_task->task_name);
        free(curr_task);

        task_number++;
        curr_task = &tasks[task_number];
    }

    pthread_mutex_unlock(&task_list_lock);
    pthread_mutex_destroy(&task_list_lock);

}