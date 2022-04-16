/*-----------------------------------------
// Author: Ryan Campbell
//
// REMARKS: A simulation of OS scheduling.
// There are two scheduling policies implemented;
// Shortest Job First, and Multi Level Feedback Queue.
// With the use of multi-threading and statistics
// tracking, it is easy to compare the trade-offs
// between the two scheduling policies.
//-----------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>

#include "task.h"
#include "mlfq.h"

/* Task Definitions */
#define task_name_length 50

/* Defines the maximum length of each line read in from the tasks.txt file */
#define task_input_length 100
#define task_file_length 500

#define max_num_cpus 8

#define NANOS_PER_USEC 1000
#define USEC_PER_SEC   1000000
#define S 5000 /* Microseconds */

#define quantum_length 50 /* Microseconds */
#define time_allotment 200 /*Microseconds (Each task in a MLFQ may run for 200 Microseconds before the priority is lowered) */
#define raise_all_priority 5000 /*Microseconds */

#define high_priority 2
#define medium_priority 1
#define low_priority 0

#define SJF 0
#define MLFQ 1
/* set to 0 for sjf and 1 for mlfq */
int scheduler_type;
int work_ready = 0;
int tasks_remaining;
int num_completed_tasks = 0;

Task *tasks;
Mlfq *mlfq;

Task *completed_tasks;
Task *next_task;

/* Locks for signaling threads when work is available */
pthread_mutex_t work_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t work_available = PTHREAD_COND_INITIALIZER;
pthread_cond_t work_not_available = PTHREAD_COND_INITIALIZER;

/* Locks for Queue control */
pthread_mutex_t tasks_left_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tasks_remaining_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tasks_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t num_complete_lock = PTHREAD_MUTEX_INITIALIZER;


struct timespec diff(struct timespec start, struct timespec end);

/*------------------------------------------------------
// microsleep
//
// PURPOSE: This code snippet, provided to us in our
// assignment documentation makes a cpu thread sleep for
// a specified duration of time.
//
// INPUT PARAMETERS:
//     Amount of time to sleep for, in usecs.
//------------------------------------------------------*/
static void microsleep(unsigned int usecs) {
    long seconds = usecs / USEC_PER_SEC;
    long nanos = (usecs % USEC_PER_SEC) * NANOS_PER_USEC;
    struct timespec t;
    int ret;
    t.tv_sec = seconds;
    t.tv_nsec = nanos;
    do {
        ret = nanosleep(&t, &t);
        /* need to loop, `nanosleep` might return before sleeping
        // for the complete time (see `man nanosleep` for details) */
    } while (ret == -1 && (t.tv_sec || t.tv_nsec));
}

/*------------------------------------------------------
// timespec
//
// PURPOSE: Profiling Code Using clock_gettime by Guy Rutenberg
// Our assignment permitted us to use this code for timing.
// Times seconds and nanoseconds.
//------------------------------------------------------*/
struct timespec diff(struct timespec start, struct timespec end) {
    struct timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}


/*------------------------------------------------------
// workSJF
//
// PURPOSE: This is a void function passed to the threads
// created earlier to simulate multiple CPUs. While there
// are still remaining jobs to process, try to grab the lock
// to acquire a job. Request the odds of IO and wait for the
// appropriate amount of time.  Submit the Task back into the
// queue and resort it if there is additional work left to
// complete, else put in the array of completed Tasks.
// INPUT PARAMETERS:
//     Takes an unused void *arg as an input parameter.
 //  *** Note, workMLFQ and workSJF had to be seperated into
// two largely similar functions due to the way remaining tasks
// are handled.  I was planning on having one work function and
// just call the appropriate method to get the next task for
// the policy the used picked. ***
//------------------------------------------------------*/
void *workSJF() {

    Task *task = NULL;

    struct timespec start_time, end_time;
    long time_to_sleep = 0;
    long IO_result;

    while (getNumberOfTasks() > 0 || work_ready == 1) {

        pthread_mutex_lock(&work_mutex);

        /* Try to acquire the work lock... */
        while (work_ready == 0 && getNumberOfTasks() > 0) {
            pthread_cond_wait(&work_available, &work_mutex);
        }

        task = next_task;

        work_ready = 0;
        pthread_cond_signal(&work_not_available);
        pthread_mutex_unlock(&work_mutex);

        if (task != NULL) {

            /*If the task is being ran for the first time, set the first run time. */
            clock_gettime(CLOCK_REALTIME, &start_time);
            if (task->first_run == 0) {
                task->first_run_time = start_time;
                task->first_run = 1;
            }

            /*  If the task perform IO, then add that IO time to the time to sleep */
            IO_result = runIO(task);
            if (IO_result != 0) {

                time_to_sleep = IO_result;
                microsleep(IO_result);
                task->task_length -= IO_result;
                if (task->task_length <= 0) {
                    task->task_length = 0;
                    /*Add to done */
                    clock_gettime(CLOCK_REALTIME, &task->completion_time);
                    pthread_mutex_lock(&num_complete_lock);
                    copyTask(&completed_tasks[num_completed_tasks], task);
                    num_completed_tasks++;
                    pthread_mutex_unlock(&num_complete_lock);

                    pthread_mutex_lock(&tasks_remaining_lock);
                    tasks_remaining--;
                    pthread_mutex_unlock(&tasks_remaining_lock);

                }
                if (task->task_length > 0) {

                    pthread_mutex_lock(&tasks_lock);
                    /*printf("Rescheduling....\n"); */
                    addTask(tasks, task);
                    pthread_mutex_unlock(&tasks_lock);
                }
            } else {
                time_to_sleep = task->task_length;
                microsleep(time_to_sleep);
                task->task_length -= time_to_sleep;

                if (task->task_length <= 0) {
                    task->task_length = 0;
                    /* Add to done */
                    clock_gettime(CLOCK_REALTIME, &task->completion_time);
                    pthread_mutex_lock(&num_complete_lock);
                    copyTask(&completed_tasks[num_completed_tasks], task);
                    num_completed_tasks++;
                    pthread_mutex_unlock(&num_complete_lock);

                    pthread_mutex_lock(&tasks_remaining_lock);
                    tasks_remaining--;
                    pthread_mutex_unlock(&tasks_remaining_lock);
                }

            }

            clock_gettime(CLOCK_REALTIME, &end_time);
        }
    }
   /* printf("\n\nTHREAD COMPLETE\n\n");  */
    return NULL;
}


/*------------------------------------------------------
// workMLFQ
//
// PURPOSE: This is a void function passed to the threads
// created earlier to simulate multiple CPUs. While there
// are still remaining jobs to process, try to grab the lock
// to acquire a job. Request the odds of IO and wait for the
// appropriate amount of time.  Submit the Task back into the
// queue and resort it if there is additional work left to
// complete, else put in the array of completed Tasks.
// INPUT PARAMETERS:
//     Takes an unused void *arg as an input parameter.
//  *** Note, workMLFQ and workSJF had to be seperated into
// two largely similar functions due to the way remaining tasks
// are handled.  I was planning on having one work function and
// just call the appropriate method to get the next task for
// the policy the used picked. ***
//------------------------------------------------------*/
void *workMLFQ() {

    Task *task = NULL;

    struct timespec start_time, end_time;
    long time_to_sleep = 0;
    long IO_result;

    while (tasks_remaining > 0 || work_ready == 1) {

       /* printf("\nNumber of tasks: %d\n", tasks_remaining); */

        pthread_mutex_lock(&work_mutex);

        /* Try to acquire the work lock... */
        while (work_ready == 0 && getNumberOfTasksMLFQ(mlfq) > 0) {
            /* printf("Number of Tasks MLFQ %d\n", getNumberOfTasksMLFQ(mlfq)); */
            pthread_cond_wait(&work_available, &work_mutex);
        }

        /* Get the task, and signal that there is no more work available */
        task = next_task;
        work_ready = 0;
        pthread_cond_signal(&work_not_available);
        pthread_mutex_unlock(&work_mutex);


        if (task != NULL) {

            /* If the task is being ran for the first time, set the first run time. */
            clock_gettime(CLOCK_REALTIME, &start_time);
            if (task->first_run == 0) {
                task->first_run_time = start_time;
                task->first_run = 1;
            }

            /*  If the task perform IO, then add that IO time to the time to sleep */
            IO_result = runIO(task);
            if (IO_result != 0) {

                time_to_sleep = IO_result;
                microsleep(IO_result);
                task->task_length -= IO_result;

                task->allotment -= IO_result;

                if (task->task_length <= 0) {
                    task->task_length = 0;

                    /*Add to done */
                    clock_gettime(CLOCK_REALTIME, &task->completion_time);
                    pthread_mutex_lock(&num_complete_lock);
                    copyTask(&completed_tasks[num_completed_tasks], task);
                    num_completed_tasks++;
                    pthread_mutex_unlock(&num_complete_lock);

                    pthread_mutex_lock(&tasks_remaining_lock);
                    tasks_remaining--;
                    pthread_mutex_unlock(&tasks_remaining_lock);
                }
                if (task->task_length > 0) {

                    pthread_mutex_lock(&tasks_lock);

                    addTaskMLFQ(mlfq, task);
                    pthread_mutex_unlock(&tasks_lock);

                }
            } else {
                time_to_sleep = task->task_length;
                microsleep(time_to_sleep);
                task->task_length -= time_to_sleep;

                if (task->task_length <= 0) {
                    task->task_length = 0;
                    /* Add to done */
                    clock_gettime(CLOCK_REALTIME, &task->completion_time);
                    pthread_mutex_lock(&num_complete_lock);
                    copyTask(&completed_tasks[num_completed_tasks], task);
                    num_completed_tasks++;
                    pthread_mutex_unlock(&num_complete_lock);

                    pthread_mutex_lock(&tasks_remaining_lock);
                    tasks_remaining--;
                    pthread_mutex_unlock(&tasks_remaining_lock);
                }
            }
            clock_gettime(CLOCK_REALTIME, &end_time);
        }
    }
/*    printf("\nTasks remaining: %d\n", tasks_remaining); */
  /*  printf("\n\nTHREAD COMPLETE\n\n"); */
    return NULL;
}



void *priorityBoost() {

    while(getNumberOfTasks(mlfq) > 0){

        microsleep(raise_all_priority);
        pthread_mutex_lock(&tasks_lock);
        increasePriorities(mlfq);
        pthread_mutex_unlock(&tasks_lock);
    }
    return NULL;
}

/*------------------------------------------------------
// sjfNextTask
//
// PURPOSE: Given an array of Tasks, this function returns
// the next shortest job from the array of Tasks.  Due to
// to the implementation, this will be the first Task in the
// array as we keep the array sorted.
// INPUT PARAMETERS:
//     Takes in an array of Tasks to process.
// OUTPUT PARAMETERS:
//     Returns a pointer to the next shortest task to be
// process, defined by the Shortest Job First scheduling
// policy.
//------------------------------------------------------*/
Task *sjfNextTask(Task *tasks) {

    Task *result;

    assert(tasks != NULL);
    result = NULL;

    pthread_mutex_lock(&tasks_lock);

    if (getNumberOfTasks() > 0) {

        qsort(tasks, getNumberOfTasks(), sizeof(Task), compareTasks);
        result = getTask(tasks, 0);
        assert(result != NULL);
    }
    pthread_mutex_unlock(&tasks_lock);

    return result;
}

/*------------------------------------------------------
// printReport
//
// PURPOSE: Prints a report, displaying statistics about the
// running time of the Tasks, given the requested scheduling
// policy.  This list in generated from a global pointer to
// an array of completed Tasks.
//------------------------------------------------------*/
void printReport() {

    int num_tasks = 0;

    long turnaround_type0_sec = 0;
    long turnaround_type0_nsec = 0;
    long turnaround_type0_avg_nsec = 0;
    long response_type0_avg_nsec = 0;

    long turnaround_type1_sec = 0;
    long turnaround_type1_nsec = 0;
    long turnaround_type1_avg_nsec = 0;
    long response_type1_avg_nsec = 0;

    long turnaround_type2_sec = 0;
    long turnaround_type2_nsec = 0;
    long turnaround_type2_avg_nsec = 0;
    long response_type2_avg_nsec = 0;

    long turnaround_type3_sec = 0;
    long turnaround_type3_nsec = 0;
    long turnaround_type3_avg_nsec = 0;
    long response_type3_avg_nsec = 0;

    Task *curr_task;

    struct timespec turn_around;
    struct timespec response_time;


    curr_task = &completed_tasks[num_tasks];

    while (curr_task->task_name != NULL) {

        turn_around = diff(curr_task->arrival_time, curr_task->completion_time);
        response_time = diff(curr_task->arrival_time, curr_task->first_run_time);

        if (curr_task->task_type == 0) {

            turnaround_type0_sec += turn_around.tv_sec;
            turnaround_type0_nsec += turn_around.tv_nsec;
            response_type0_avg_nsec += response_time.tv_nsec;
        } else if (curr_task->task_type == 1) {

            turnaround_type1_sec += turn_around.tv_sec;
            turnaround_type1_nsec += turn_around.tv_nsec;
            response_type1_avg_nsec += response_time.tv_nsec;

        } else if (curr_task->task_type == 2) {

            turnaround_type2_sec += turn_around.tv_sec;
            turnaround_type2_nsec += turn_around.tv_nsec;
            response_type2_avg_nsec += response_time.tv_nsec;

        } else if (curr_task->task_type == 3) {

            turnaround_type3_sec += turn_around.tv_sec;
            turnaround_type3_nsec += turn_around.tv_nsec;
            response_type3_avg_nsec += response_time.tv_nsec;
        }
        num_tasks++;
        curr_task = &completed_tasks[num_tasks];
    }

    if(num_tasks > 0){

        turnaround_type0_avg_nsec = turnaround_type0_nsec / num_tasks;
        turnaround_type1_avg_nsec = turnaround_type1_nsec / num_tasks;
        turnaround_type2_avg_nsec = turnaround_type2_nsec / num_tasks;
        turnaround_type3_avg_nsec = turnaround_type3_nsec / num_tasks;

        response_type0_avg_nsec = response_type0_avg_nsec / num_tasks;
        response_type1_avg_nsec = response_type1_avg_nsec / num_tasks;
        response_type2_avg_nsec = response_type2_avg_nsec / num_tasks;
        response_type3_avg_nsec = response_type3_avg_nsec / num_tasks;
    }

    if(scheduler_type == SJF){
        /* Turnaround = Completion - Arrival */
        printf("\nAverage turnaround time per type:\n\n");
        printf("Type 0: %lu usec\n", turnaround_type0_avg_nsec / NANOS_PER_USEC);
        printf("Type 1: %lu usec\n", turnaround_type1_avg_nsec / NANOS_PER_USEC);
        printf("Type 2: %lu usec\n", turnaround_type2_avg_nsec / NANOS_PER_USEC);
        printf("Type 3: %lu usec\n", turnaround_type3_avg_nsec / NANOS_PER_USEC);

        /* Response = FirstRun - Arrival */
        printf("\nAverage response time per type:\n\n");
        printf("Type 0: %lu usec\n", response_type0_avg_nsec / NANOS_PER_USEC);
        printf("Type 1: %lu usec\n", response_type1_avg_nsec / NANOS_PER_USEC);
        printf("Type 2: %lu usec\n", response_type2_avg_nsec / NANOS_PER_USEC);
        printf("Type 3: %lu usec\n", response_type3_avg_nsec / NANOS_PER_USEC);

    }
    else{ /* This if else statement is due to a bug i realized late in development.  I mixed up
          the priority queue naming conventions, causing the statistics to be calculated incorrectly.
          This if else statement was a last minute "fix" to the assignment and properly fixing would have
          taken too long. */

        printf("\nAverage turnaround time per type:\n\n");

        printf("Type 0: %lu usec\n", turnaround_type3_avg_nsec / NANOS_PER_USEC);
        printf("Type 1: %lu usec\n", turnaround_type2_avg_nsec / NANOS_PER_USEC);
        printf("Type 2: %lu usec\n", turnaround_type1_avg_nsec / NANOS_PER_USEC);
        printf("Type 3: %lu usec\n", turnaround_type0_avg_nsec / NANOS_PER_USEC);

        /* Response = FirstRun - Arrival */
        printf("\nAverage response time per type:\n\n");

        printf("Type 0: %lu usec\n", response_type3_avg_nsec / NANOS_PER_USEC);
        printf("Type 1: %lu usec\n", response_type2_avg_nsec / NANOS_PER_USEC);
        printf("Type 2: %lu usec\n", response_type1_avg_nsec / NANOS_PER_USEC);
        printf("Type 3: %lu usec\n", response_type0_avg_nsec / NANOS_PER_USEC);
    }
}

/*------------------------------------------------------
// scheduler
//
// PURPOSE: Given an array of Tasks, the number of simulated
// CPUs, and the desired scheduling policy, This function does
// the initialization of the scheduler such as creating the
// threads required to execute the Tasks.
// INPUT PARAMETERS:
//     Takes in an array of Tasks, the number of simulated
// CPUs, and the desired scheduling policy.
//------------------------------------------------------*/
void scheduler(Task *tasks, int num_cpus, char *policy) {


    int i;
    int task_number = 0;
    Task *curr_task = NULL;

    pthread_t pBoost;
    pthread_t cpus[max_num_cpus];

    assert(tasks != NULL);

    pthread_mutex_lock(&tasks_remaining_lock);
    tasks_remaining = getNumberOfTasks();
    pthread_mutex_unlock(&tasks_remaining_lock);

    printf("\n");

    if (strcmp(policy, "sjf") == 0) {
        printf("Using sjf with %d CPUs.\n", num_cpus);
        scheduler_type = SJF;

        /* Pass each thread a work function specialized for SJF */
        for (i = 0; i < num_cpus; i++) {
            pthread_create(&cpus[i], NULL, workSJF, &tasks[i]);
        }

    } else if (strcmp(policy, "mlfq") == 0) {
        printf("Using mlfq with %d CPUs\n", num_cpus);
        scheduler_type = MLFQ;

         /* printf("Number of tasks: %d\n", tasks_remaining); */

        mlfq = makeMLFQ(tasks, tasks_remaining);

        /* Pass each thread a work function specialized for SJF */
        for (i = 0; i < num_cpus; i++) {
            pthread_create(&cpus[i], NULL, workMLFQ, NULL);
        }

        pthread_create(&pBoost, NULL, priorityBoost, NULL);


    } else {
        printf("Scheduler type is invalid\n");
        exit(EXIT_SUCCESS);
    }

    /* Set the arrival time of all tasks */
    curr_task = &tasks[task_number];
    while (curr_task->task_name != NULL) {
        clock_gettime(CLOCK_REALTIME, &curr_task->arrival_time);
        task_number++;
        curr_task = &tasks[task_number];
    }

    /* While there are tasks remaining, determine the next task to run and broadcast it to all threads */
    while (tasks_remaining > 0) {

        pthread_mutex_lock(&work_mutex);
        while (work_ready == 1) {
            pthread_cond_wait(&work_not_available, &work_mutex);
        }

        /* Get the next task from the appropriate scheduling policy */


        if (scheduler_type == SJF) {
            next_task = sjfNextTask(tasks);
        } else if (scheduler_type == MLFQ) {
            pthread_mutex_lock(&tasks_lock);
            next_task = getTaskMLFQ(mlfq);
            pthread_mutex_unlock(&tasks_lock);

        }


        /* If we successfully acquired the next task, broadcast to all threads the the Task is available */
        if (next_task != NULL) {
            pthread_cond_broadcast(&work_available);
            work_ready = 1;
        } else {
            work_ready = 0;
        }

        pthread_mutex_unlock(&work_mutex);
    }

    /* Wait for all threads to finish processing */
    for (i = 0; i < num_cpus; i++) {
        pthread_join(cpus[i], NULL);
    }


    /*pthread_join(pBoost, NULL); */

}


/*------------------------------------------------------
// main
//
// PURPOSE: this is the scheduler's main that is run when the
// scheduler is executed. It reads the arguments passed to the
// program by the user, looking to accept the number of desired
// CPUs to utilize, along with which scheduling policy to employ.
// it then reads the tasks from the task file, and sets up an
// array of structs of type Task.
//
// INPUT PARAMETERS:
//     Takes in arguments from standard I/O given by the
// user.  The program requires 2 arguments to be passed, the
// first argument is the desired number of CPUs to utilize
// in the simulation.  The second argument is which scheduling
// policy to employ.  The 2 options of scheduling policies are:
// sjf, and mlfq.
// OUTPUT PARAMETERS:
//     Returns 0 if the program finishes successfully.
//------------------------------------------------------*/
int main(int argc, char *argv[]) {

    int number_of_cpus;
    FILE *task_file;
    srand(time(NULL));

    /* Ensure the user passes 2 parameters to the scheduler (Number of CPUs and scheduler type) */
    if (argc == 3) {

        /* Assign the number of CPUs to simulate */
        number_of_cpus = strtol(argv[1], NULL, 10);

        /* Bound checking to ensure a valid CPU number was entered */
        if (number_of_cpus > max_num_cpus || number_of_cpus <= 0) {

            printf("\nUnable to make '%d' CPUs. Please enter a value between 1 and 8\n", number_of_cpus);
            EXIT_SUCCESS;
        } else {

            task_file = fopen("tasks.txt", "r");
            if (task_file != NULL) {

                tasks = makeTasks(task_file);

                completed_tasks = completedTasks();

                scheduler(tasks, number_of_cpus, argv[2]);

                printReport();

                if(scheduler_type == SJF){
                    destroyTasks(tasks);    /* deallocate the malloc'd array of tasks */
                }
                else
                {
                    /* destroy mlfq */
                }

            } else {
                printf("Was unable to open the task file, check to make sure 'tasks.txt' exists\n");
            }
        }

    } else {
        printf("\nPlease provide the program the number of CPUs along with the scheduling type (sjf / mlfq)\n\n\nExample: ./scheduling 4 mlfq\n");
    }

    printf("\nProgram completed normally.\n\n");
    return EXIT_SUCCESS;
}
