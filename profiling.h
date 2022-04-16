//
// Created by Ryan on 7/7/2021.
//

#ifndef SCHEDULING_PROFILING_H
#define SCHEDULING_PROFILING_H



static void microsleep(unsigned int usecs);

struct timespec diff(struct timespec start, struct timespec end);

#endif //SCHEDULING_PROFILING_H
