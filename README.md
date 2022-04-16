
# Simulating Scheduling

### How to run

Step 1) Please open a terminal window, and navigate to the folder containing my code.

Step 2) Type "make" into the terminal (without the quotations) and press the enter key.

Step 3) Next, type "./scheduling 4 sjf" (without the quotations).  Replace "4" with the desired number of CPUs to simulate.  Replace "sjf" with the scheduling policy to use (sjf/mlfq). Press enter.


###This table had each value computed as the average of 5 runs.

| CPUs |  Type  | SJF Turnaround Time | SJF Response Time | MLFQ Turnaround Time | MLFQ Response Time |
|------|:------:|:-------------------:|-------------------|----------------------|--------------------|
| 1    | Type 0 |      5756 usec      |     5156 usec     |      18515 usec      |     14234 usec     |
|      | Type 1 |      29877 usec     |     28173 usec    |      60164 usec      |     51186 usec     |
|      | Type 2 |      93312 usec     |     89674 usec    |      53685 usec      |     38172 usec     |
|      | Type 3 |      79654 usec     |     79898 usec    |      50563 usec      |     46625 usec     |
| 2    | Type 0 |      3173 usec      |     2674 usec     |      16955 usec      |     14310 usec     |
|      | Type 1 |      15556 usec     |     14163 usec    |      33084 usec      |     26307 usec     |
|      | Type 2 |      63497 usec     |     56393 usec    |      34054 usec      |     32915 usec     |
|      | Type 3 |      66711 usec     |     58202 usec    |      48120 usec      |     47173 usec     |
| 4    | Type 0 |      1706 usec      |     6193 usec     |       7223 usec      |      6848 usec     |
|      | Type 1 |      8057 usec      |     6748 usec     |      19701 usec      |     13084 usec     |
|      | Type 2 |      33539 usec     |     26985 usec    |      19705 usec      |     18528 usec     |
|      | Type 3 |      35622 usec     |     28558 usec    |      27834 usec      |     26700 usec     |
| 8    | Type 0 |      1003 usec      |      563 usec     |       5498 usec      |      2910 usec     |
|      | Type 1 |      4420 usec      |     3199 usec     |       8265 usec      |      3709 usec     |
|      | Type 2 |      18530 usec     |     12180 usec    |       9115 usec      |      7907 usec     |
|      | Type 3 |      19839 usec     |     13160 usec    |      13675 usec      |     12837 usec     |


Observation 1)

The difference in response time that I observed is not what I first expected, but after some thinking it makes sense.  At first, I expected MLFQ to outperform SJF. From what I observed this is not this not the case because the SJF policy knows how long each task will take to complete.  This is very unrealistic in the real world, as most times, we don't know how long a process will run. This gives the SJF policy an unfair advantage, as the MLFQ does not have to privilege to know how long each task will take. The MLFQ is a much more complex policy, thus I believe the longer turn around time is due to the overhead of the algorithm.  However the response time of the MLFQ I expected to be much better and surpass SJF, but it doesn't appear to be.  I believe this may be due to a race condition in  my code.

Observation 2)

As the numbers of CPUs in the system are increased, turnaround time and response time increase drastically.  The turn around and response time for tasks appears to be highly correlated to the number of CPUs.  This is because then using 1 CPU, there is no parallelization.  When we add additional CPUs, we allow the program to run much quicker due to parallelization. 

## Authors

- Ryan Campbell

