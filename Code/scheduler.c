#include "headers.h"
#include "queue.h"
#include "LinkedList.h"
#include "BuddyBinaryTree.h"
#include <stdlib.h>
#include <string.h>

//#define QUANTUM 3

// TODO: in PG, clear all IPC recources

// Variables for communication between the PG & the scheduler
struct PG_S_MB processGenBuffer;
int PG_S_MQid;

// Variables for communication between the scheduler & each process
int S_P_ShMemid;
int *memAdr;

// Queue to be used for first_come_first_serve, Round Robin
// Feel free to use it if you need it in any other algorithm
QUEUE *PCB_Scheduling_Queue;

// This queue contains all allocated processes
QUEUE *PCBs;

// Contains all processes that are waiting to be allocated
LIST *WaitingPCBs;

//Used to store the Algorithm number. If this number is equal to 2 then elements are enqueued sorted by Running time in PCBs
// If this number is equal to 4 then elements are enqueued sorted by Remaining time in PCBs
// Any other algorithm is sorted normally
Scheduling_Algorithm_Type theAlgorithm;

// enum which hole the type of emory algo we need
MemoryAlgorithm theMemoryAlgorithm;
// this Linekd List to be used for the first 3 memory algorithms
LIST *MemoryList;
// Binary Tree Root: Used to access the buddy system memory tree
BinaryTreeNode *memRoot;

// If the PG is sending a new process to the scheduler, it signals it before sending
// Then it sends the process data via a Message queue
void new_process_handler(int signum);

// Scheduling Algorithms
void First_Come_First_Serve_Scheduling(void);
void Shortest_Job_First_Scheduling(void);
void PreemptiveHighestPriorityFirst(void);
void Shortest_Remaining_Time_Next_Scheduling(void);
void Round_Robin_Scheduling(void);

// Memory Algorithms
void First_Fit_memAlgo(void);

// Variables used for output files
float CPU_util;
int total_run_time;
int numOfProcs;
float waiting_time;
float weighted_turnaround_time;
FILE *logFile;
FILE *perfFile;
char *InputFileName;
int QUANTUM;
int memAlgo;

// Comparison functions
int CompareRunningTime(void *, void *);
int ComparePriority(void *, void *);
int CompareRemainingTime(void *, void *);
int dummy_compare(void *a, void *b);

// Allocation
void *allocateMemory_BSA();

// Deallocation functions
void deallocateMemory(int process_id);
void deallocateMemory_BSA(int process_id);

int main(int argc, char *argv[])
{
    printf("Scheduler spawned!\n");

    initClk();
    // if the scheduler receives this signal, it means that the PG is sending it a new process
    signal(SIGUSR1, new_process_handler);

    theAlgorithm = atoi(argv[1]);
    if (theAlgorithm == RR)
    {
        QUANTUM = atoi(argv[3]);
        memAlgo = atoi(argv[4]);
    }
    else
    {
        QUANTUM = 0;
        memAlgo = atoi(argv[3]);
    }

    //******************************MEMORY INIT*************************//

    void (*MemAlgoToRun)(void);
    switch (memAlgo)
    {
    case FF:
        MemoryList = createList(&dummy_compare);

        memory_fragment *initMem;
        initMem = (memory_fragment *)malloc(sizeof(memory_fragment));
        initMem->theState = GAP;
        initMem->start_position = 0;
        initMem->length = 1024;
        _insert(MemoryList, MemoryList->rear, (void *)initMem);

        MemAlgoToRun = &First_Fit_memAlgo;
        //todo: function_pointer= FF algo
        break;
    case NF:

        break;
    case BF:

        break;
    case BSA:

        break;

    default:
        break;
    }

    ///////////////////////////////////////////////////////////////////////
    //itnialise the linked list of coming unAllocated processes

    WaitingPCBs = createList(&dummy_compare);

    // Initialize the queue
    PCBs = createQueue();

    processGenBuffer.mtype = getpid() % 10000;

    // Getting the ID of the PG_S MQ
    key_t kid1 = ftok(PROCGEN_SCHED_QKEY, 'A');
    PG_S_MQid = msgget(kid1, 0666 | IPC_CREAT);
    // printf("PG_S_MQid = %d\n", PG_S_MQid);

    // Getting the ID of the S_P MQ
    key_t kid2 = ftok(SCHED_PROC_QKEY, 'B');
    S_P_ShMemid = shmget(kid2, sizeof(int), 0666 | IPC_CREAT);
    // printf("SCHED SHM ID: %d", S_P_ShMemid);

    // Attach the shared memory to the scheduler
    memAdr = (int *)shmat(S_P_ShMemid, (void *)0, 0);

    // printf("S_P_MQid = %d\n", S_P_ShMemid);

    // printf("All the arguments received: ");
    // for (int i = 0; i < argc; i++)
    // {
    //     printf("%s ", argv[i]);
    // }
    // printf("\n");

    // Output file variables
    //LOG = createQueue();
    //PERF = createQueue();

    CPU_util = 0;
    total_run_time = 0;
    numOfProcs = atoi(argv[2]);

    //InputFileName = argv[3]; //Uncomment after changing the project structure --> Probably won't need it anyway
    //Uncomment after changing the project structure --> Used to count the number of processes
    // FILE *ptrToFile;
    // numOfProcs = 0;
    // char cr;
    // ptrToFile = fopen(InputFileName, "r");
    // if (ptrToFile == NULL)
    // {
    //     printf("Could not open file %s", InputFileName);
    //     return 0;
    // }
    // cr = getc(ptrToFile);
    // while (cr != EOF)
    // {
    //     if (cr == '\n')
    //     {
    //         numOfProcs = numOfProcs + 1;
    //     }
    //     cr = getc(ptrToFile);
    // }
    //fclose(ptrToFile);
    //printf("There are %d processes in the input file\n", numOfProcs);

    waiting_time = 0;
    weighted_turnaround_time = 0;
    int num = numOfProcs;
    logFile = fopen(LOG, "w");
    if (logFile == NULL)
        printf("Error opening log file\n");

    printf("The number of Processes to schedule is %d\n", numOfProcs);
    // TODO: depending on the value of sigAlgo, make the required Datastructures & use them
    // the Algorithm should be equal to the wanted Algorithm (from 1 to 5 as in the document)
    //1. First Come First Serve (FCFS)
    //2. Shortest Job First (SJF)
    //3. Preemptive Highest Priority First (HPF)
    //4. Shortest Remaining Time Next (SRTN)
    //5. Round Robin (RR)
    // we should determine the algo from the args in process generator

    //printf("Quantum= %d\n", QUANTUM);

    // This switch case will be used to make
    // The necessary initializations for each algorithm
    void (*AlgoToRun)(void);
    switch (theAlgorithm)
    {
    case FCFS:
        PCB_Scheduling_Queue = createQueue();
        AlgoToRun = &First_Come_First_Serve_Scheduling;
        break;
    case SJF:
        PCB_Scheduling_Queue = createQueue();
        AlgoToRun = &Shortest_Job_First_Scheduling;
        break;
    case HPF:
        PCB_Scheduling_Queue = createQueue();
        AlgoToRun = &PreemptiveHighestPriorityFirst;
        break;
    case SRTN:
        PCB_Scheduling_Queue = createQueue();
        AlgoToRun = &Shortest_Remaining_Time_Next_Scheduling;
        break;
    case RR:
        PCB_Scheduling_Queue = createQueue();
        AlgoToRun = &Round_Robin_Scheduling;
        break;
    default:
        break;
    }

    fflush(stdin);

    int currTime = getClk();

    while (numOfProcs > 0)
    {
        MemAlgoToRun();
        AlgoToRun();
    }

    CPU_util = (total_run_time / ((float)getClk()) * 100);
    perfFile = fopen(PERF, "w");
    if (perfFile == NULL)
        printf("Error in opening the perf file\n");

    // Take only the first two decimal digits after the floating point

    weighted_turnaround_time /= num;

    weighted_turnaround_time *= 100;
    int temp = weighted_turnaround_time;
    weighted_turnaround_time = temp / 100.0;

    waiting_time /= num;
    waiting_time *= 100;
    temp = waiting_time;
    waiting_time = temp / 100.0;

    CPU_util *= 100;
    temp = CPU_util;
    CPU_util = temp / 100.0;

    fprintf(perfFile, "CPU utilization = %0.2f%%\nAvg WTA = %0.2f\nAvg Waiting = %0.2f\n",
            CPU_util, weighted_turnaround_time, waiting_time);

    fclose(perfFile);
    fclose(logFile);

    destroyClk(true);
}

void output_started(PCB *process)
{
    fprintf(logFile, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), process->id,
            process->arrivaltime, process->runningtime, process->remainingtime,
            getClk() - process->arrivaltime - (process->runningtime - process->remainingtime));
}

void output_resumed(PCB *process)
{
    fprintf(logFile, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), process->id,
            process->arrivaltime, process->runningtime, process->remainingtime,
            getClk() - process->arrivaltime - (process->runningtime - process->remainingtime));
}

void output_finished(PCB *process)
{
    float temp = (getClk() - process->arrivaltime) / (float)process->runningtime;
    temp *= 100;
    int temp1 = temp;
    temp = temp1 / 100.0;

    fprintf(logFile, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %0.2f\n", getClk(),
            process->id,
            process->arrivaltime, process->runningtime, process->remainingtime,
            getClk() - process->arrivaltime - (process->runningtime - process->remainingtime),
            getClk() - process->arrivaltime, temp);

    total_run_time += process->runningtime;
}

void output_stopped(PCB *process)
{
    fprintf(logFile, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), process->id,
            process->arrivaltime, process->runningtime, process->remainingtime,
            getClk() - process->arrivaltime - (process->runningtime - process->remainingtime));
}

void update_cpu_data(PCB *process)
{
    weighted_turnaround_time += (getClk() - process->arrivaltime) / (float)process->runningtime;
    waiting_time += getClk() - process->arrivaltime - process->runningtime;
}

void new_process_handler(int signum)
{
    int PG_S_recVal = msgrcv(PG_S_MQid, &processGenBuffer, sizeof(processGenBuffer.P), processGenBuffer.mtype, !IPC_NOWAIT);
    if (PG_S_recVal != -1)
    {

        // For debugging
        //fflush(stdin);
        //printf("Process with id %d received at time %d\n", processGenBuffer.P.id, getClk());

        // Fork the new process, send it to the process file
        // int pid = fork();
        // if (pid == 0) // Child
        // {
        //     execl("process", "process", (char *)NULL);
        // }

        // Pause the process that we just forked
        //kill(pid, SIGSTOP);

        // Create a new PCB structure  & fill it will info
        // Needs to be dynamically created so that it persists after the handler ends
        PCB *tempPCB = malloc(sizeof(PCB));
        tempPCB->arrivaltime = processGenBuffer.P.arrivaltime;
        tempPCB->id = processGenBuffer.P.id;
        //tempPCB->pid = pid;
        tempPCB->runningtime = processGenBuffer.P.runningtime;
        tempPCB->remainingtime = processGenBuffer.P.runningtime;
        tempPCB->priority = processGenBuffer.P.priority;
        tempPCB->memsize = processGenBuffer.P.memsize;
        //fflush(stdin);
        //printf("Remaining time %d\n", tempPCB->remainingtime);
        //enqueue(WaitingPCBs, (void *) tempPCB);
        _insert(WaitingPCBs, WaitingPCBs->rear, (void *)tempPCB);

        // Add this process to the new processes queue
        // The selected Algo can then take this new process and add it
        // To its ready queue (could different for every Algorithm)

        // if (theAlgorithm == SJF)
        //     enqueue_sorted(PCBs, (void *)tempPCB, CompareRunningTime);
        // else if (theAlgorithm == SRTN)
        //     enqueue_sorted(PCBs, (void *)tempPCB, CompareRemainingTime);
        // else
        //     enqueue(PCBs, (void *)tempPCB);
    }

    // Reassign this function as SIGUSR1 handler
    signal(SIGUSR1, new_process_handler);
}

void First_Come_First_Serve_Scheduling(void)
{
    PCB *ptr_to_arriving_processes;

    // Check if there is a new process from the new process handler
    // If yes, add it to the ready queue (PCB_Scheduling_Queue)
    if (!emptyQueue(PCBs))
    {
        dequeue(PCBs, (void *)&ptr_to_arriving_processes);
        enqueue(PCB_Scheduling_Queue, (void *)ptr_to_arriving_processes);
    }

    if (!emptyQueue(PCB_Scheduling_Queue))
    {
        PCB *front_process_queue;
        dequeue(PCB_Scheduling_Queue, (void *)&front_process_queue);

        //printf("process with id %d is now running\n", front_process_queue->pid);

        int end_time = getClk() + front_process_queue->remainingtime;

        //printf("end_time=%d\n",end_time);

        int currTime = getClk();

        *memAdr = front_process_queue->remainingtime;

        output_started(front_process_queue);

        kill(front_process_queue->pid, SIGCONT);

        while (front_process_queue->remainingtime > 0)
        {
            if (getClk() != currTime)
            {
                front_process_queue->remainingtime -= 1;
                *memAdr = front_process_queue->remainingtime;
                currTime = getClk();
            }
            //printf("current_time=%d",getClk());
            // blocking
        }

        //fflush(stdin);

        //printf("Process with id %d finished at time %d\n", front_process_queue->pid, getClk());

        //kill(front_process_queue->pid, SIGSTOP);

        // Delete the allocated data for this PCB

        update_cpu_data(front_process_queue);

        output_finished(front_process_queue);

        free(front_process_queue);
        /*
        
        */
        numOfProcs--;
    }
}

void Shortest_Job_First_Scheduling(void)
{
    PCB *ptr_to_arriving_processes;

    // Check if there is a new process from the new process handler
    // If yes, add it to the ready priority queue (PCB_Scheduling_Queue)
    if (!emptyQueue(PCBs))
    {
        dequeue(PCBs, (void *)&ptr_to_arriving_processes);
        enqueue_sorted(PCB_Scheduling_Queue, (void *)ptr_to_arriving_processes, CompareRunningTime);
    }

    if (!emptyQueue(PCB_Scheduling_Queue))
    {
        PCB *front_process_queue;
        dequeue(PCB_Scheduling_Queue, (void *)&front_process_queue);

        //printf("process with id %d is now running\n", front_process_queue->pid);

        kill(front_process_queue->pid, SIGCONT);

        output_started(front_process_queue);

        int currTime = getClk();

        *memAdr = front_process_queue->remainingtime;

        while (front_process_queue->remainingtime > 0)
        {
            if (getClk() != currTime)
            {
                front_process_queue->remainingtime -= 1;
                *memAdr = front_process_queue->remainingtime;
                currTime = getClk();
            }
            //printf("current_time=%d",getClk());
            // blocking
        }

        //fflush(stdin);

        //printf("Process with id %d and of RT=%d finished at time %d\n", front_process_queue->id, front_process_queue->runningtime, getClk());

        //kill(front_process_queue->pid, SIGSTOP);

        // Delete the allocated data for this PCB
        update_cpu_data(front_process_queue);

        output_finished(front_process_queue);

        if (memAlgo != BSA)
            deallocateMemory(front_process_queue->id);
        else
            deallocateMemory_BSA(front_process_queue->id);

        free(front_process_queue);

        numOfProcs--;
    }
}

void Shortest_Remaining_Time_Next_Scheduling(void)
{
    PCB *ptr_to_arriving_processes;
    bool Preemption = false;
    // Check if there is a new process from the new process handler
    // If yes, add it to the ready priority queue (PCB_Scheduling_Queue) sorted by remaining time
    if (!emptyQueue(PCBs))
    {
        // View queue
        // printf("ENqueue in the PCB Scheduling Queue Count=%d:\n", PCB_Scheduling_Queue->count);
        // QUEUE_NODE *ptr = PCB_Scheduling_Queue->front;
        // while (ptr)
        // {
        //     printf("%d @%d -->\t", ((PCB *)ptr->dataPtr)->id, ((PCB *)ptr->dataPtr)->remainingtime);
        //     ptr = ptr->next;
        // }
        // printf("%s", "\n");
        dequeue(PCBs, (void *)&ptr_to_arriving_processes);
        enqueue_sorted(PCB_Scheduling_Queue, (void *)ptr_to_arriving_processes, CompareRemainingTime);
    }

    if (!emptyQueue(PCB_Scheduling_Queue))
    {
        // View queue
        // printf("Before dequeueing the PCB Scheduling Queue:\n");
        // QUEUE_NODE *ptr = PCB_Scheduling_Queue->front;
        // while (ptr)
        // {
        //     printf("%d @%d -->\t", ((PCB *)ptr->dataPtr)->id, ((PCB *)ptr->dataPtr)->remainingtime);
        //     ptr = ptr->next;
        // }
        // printf("%s", "\n");
        PCB *front_process_queue;
        dequeue(PCB_Scheduling_Queue, (void *)&front_process_queue);

        kill(front_process_queue->pid, SIGCONT);

        if (front_process_queue->remainingtime < front_process_queue->runningtime)
            output_resumed(front_process_queue);
        else
            output_started(front_process_queue);

        int currTime = getClk();

        *memAdr = front_process_queue->remainingtime;

        while (front_process_queue->remainingtime > 0)
        {
            if (getClk() != currTime)
            {
                front_process_queue->remainingtime -= 1;
                *memAdr = front_process_queue->remainingtime;
                currTime = getClk();
            }
            if (queueFront(PCBs, (void *)&ptr_to_arriving_processes))
            {
                if (ptr_to_arriving_processes->remainingtime < front_process_queue->remainingtime)
                {
                    Preemption = true;
                    enqueue_sorted(PCB_Scheduling_Queue, (void *)front_process_queue, CompareRemainingTime);
                    output_stopped(front_process_queue);
                    //printf("Process with id %d preempted that of id %d at time %d\n", ptr_to_arriving_processes->id, front_process_queue->id, getClk());
                    break;
                }
            }
            //printf("current_time=%d",getClk());
            // blocking
        }
        //fflush(stdin);
        if (Preemption != true)
        {
            printf("Process with id %d and of RT=%d finished at time %d\n", front_process_queue->id, front_process_queue->runningtime, getClk());

            //kill(front_process_queue->pid, SIGSTOP);

            output_finished(front_process_queue);

            update_cpu_data(front_process_queue);

            if (memAlgo != BSA)
                deallocateMemory(front_process_queue->id);
            else
                deallocateMemory_BSA(front_process_queue->id);

            // Delete the allocated data for this PCB
            free(front_process_queue);
            numOfProcs--;
        }
        // // View queue
        // printf("The remaining processes in the PCB Scheduling Queue:\n");
        // ptr = PCB_Scheduling_Queue->front;
        // while (ptr)
        // {
        //     printf("%d @%d -->\t", ((PCB *)ptr->dataPtr)->id, ((PCB *)ptr->dataPtr)->remainingtime);
        //     ptr = ptr->next;
        // }
        // printf("%s", "\n");
    }
}

void Round_Robin_Scheduling(void)
{
    PCB *ptr_to_arriving_processes;
    if (!emptyQueue(PCB_Scheduling_Queue))
    {
        PCB *front_process_queue;
        dequeue(PCB_Scheduling_Queue, (void *)&front_process_queue);
        //printf("process with id %d is now running at time %d\n", front_process_queue->id, getClk());

        //int end_time = getClk() + front_process_queue->remainingtime;

        int time_at_entry = getClk();
        int currTime = getClk();
        kill(front_process_queue->pid, SIGCONT);
        *memAdr = front_process_queue->remainingtime;

        if (front_process_queue->remainingtime < front_process_queue->runningtime)
            output_resumed(front_process_queue);
        else
            output_started(front_process_queue);

        // the loop will continue until the QUANTUM is finished or the time of queue is finished
        while (currTime < (time_at_entry + QUANTUM) && front_process_queue->remainingtime != 0)
        {
            if (getClk() != currTime) // update the time each time clock changes
            {
                front_process_queue->remainingtime -= 1;
                *memAdr = front_process_queue->remainingtime;
                currTime = getClk();
            }
        }
        // flag to determine if the process is ended
        bool flag = (front_process_queue->remainingtime == 0) ? 0 : 1;
        if (flag) // if the process has to run again
        {
            //printf("process with id %d is now stopping at time %d\n", front_process_queue->id, getClk());
            kill(front_process_queue->pid, SIGSTOP);
            output_stopped(front_process_queue);
        }
        else //free the memory if the process is finished
        {
            //printf("process with id %d is now finishing at time %d\n", front_process_queue->id, getClk());
            output_finished(front_process_queue);
            update_cpu_data(front_process_queue);

            if (memAlgo != BSA)
                deallocateMemory(front_process_queue->id);
            else
                deallocateMemory_BSA(front_process_queue->id);

            free(front_process_queue);
            numOfProcs--;
        }
        currTime = getClk();
        //get all the newly added processes while the Quantum was running
        if (!emptyQueue(PCBs))
        {
            dequeue(PCBs, (void *)&ptr_to_arriving_processes);

            // Take all processes that arrived before current clock cycle and add them to the
            // Scheduling queue
            while (ptr_to_arriving_processes->arrivaltime < currTime)
            {
                enqueue(PCB_Scheduling_Queue, (void *)ptr_to_arriving_processes);
                if (!emptyQueue(PCBs))
                    dequeue(PCBs, (void *)&ptr_to_arriving_processes);
                else
                    break;
            }

            if (flag)
                enqueue(PCB_Scheduling_Queue, (void *)front_process_queue);

            if (ptr_to_arriving_processes->arrivaltime == currTime)
            {
                enqueue(PCB_Scheduling_Queue, (void *)ptr_to_arriving_processes);
                while (!emptyQueue(PCBs))
                {
                    dequeue(PCBs, (void *)&ptr_to_arriving_processes);
                    enqueue(PCB_Scheduling_Queue, (void *)ptr_to_arriving_processes);
                }
            }
        }
        else
        {
            if (flag)
                enqueue(PCB_Scheduling_Queue, (void *)front_process_queue);
        }
    }

    // Check if there is a new process from the new process handler
    // If yes, add it to the ready queue (PCB_Scheduling_Queue
    if (!emptyQueue(PCBs))
    {
        dequeue(PCBs, (void *)&ptr_to_arriving_processes);
        enqueue(PCB_Scheduling_Queue, (void *)ptr_to_arriving_processes);
    }
}

void PreemptiveHighestPriorityFirst()
{

    static PCB *newProc, *currentRunning, *dequeuePtr;
    static int currTime = 0;
    fflush(stdin);

    // Check for an incoming process
    // If the process queue has a process, consume it
    if (!emptyQueue(PCBs))
    {
        dequeue(PCBs, (void *)(&newProc));

        printf("Process with PID: %d, remaining time: %d, priority: %d arrived\n", newProc->pid, newProc->remainingtime, newProc->priority);
        currTime = getClk();

        // if (currTime != getClk())
        // {
        //     printf("[%d] ", currTime);
        //     fflush(stdout);
        // }

        // If first process
        if (emptyQueue(PCB_Scheduling_Queue))
        {
            // printf("%s", "The process is alone, inserting at the front of the queue\n");
            enqueue(PCB_Scheduling_Queue, (void *)newProc);
            currentRunning = newProc;
            *memAdr = currentRunning->remainingtime;
            kill(currentRunning->pid, SIGCONT);
            if (currentRunning->remainingtime < currentRunning->runningtime)
                output_resumed(currentRunning);
            else
                output_started(currentRunning);
        }
        else
        {
            // printf("%s", "There are other processes, inserting somewhere in the queue\n");
            // If there's one, check if its priority is higher than the current priority
            if (ComparePriority(newProc, currentRunning) == 1)
            // If so, send SIGSTOP to the current process, place the new one in the queue, and send SIGSTRT to it if needed
            {
                // printf("%s", "Process has higher priority than the current running process, replacing\n");
                kill(currentRunning->pid, SIGSTOP);
                output_stopped(currentRunning);
                currentRunning = newProc;
                *memAdr = currentRunning->remainingtime;
                kill(currentRunning->pid, SIGCONT);
                if (currentRunning->remainingtime < currentRunning->runningtime)
                    output_resumed(currentRunning);
                else
                    output_started(currentRunning);
            }
            // If its priority is less, just enqueue
            enqueue_sorted(PCB_Scheduling_Queue, (void *)newProc, ComparePriority);
        }
    }

    if (emptyQueue(PCB_Scheduling_Queue))
        return;

    // TODO Update the running process's remaining time through the shared memory segment

    if (currTime != getClk())
    {
        currTime = getClk();

        // If there are no incoming processes, just run the current process
        currentRunning->remainingtime--;
        *memAdr = currentRunning->remainingtime;

        // If the process's time is out, remove it from the queue
        if (currentRunning->remainingtime == 0)
        {
            printf("%s", "A process has finished running, removing from the queue\n");
            output_finished(currentRunning);
            update_cpu_data(currentRunning);
            dequeue(PCB_Scheduling_Queue, (void *)(&dequeuePtr));

            if (memAlgo != BSA)
                deallocateMemory(dequeuePtr->id);
            else
                deallocateMemory_BSA(dequeuePtr->id);

            free(dequeuePtr);
            numOfProcs--;

            if (queueFront(PCB_Scheduling_Queue, (void *)(&dequeuePtr)))
            {
                // printf("%s", "There are other processes left, dequeuing one\n");
                currentRunning = dequeuePtr;
                *memAdr = currentRunning->remainingtime;
                if (currentRunning->remainingtime < currentRunning->runningtime)
                    output_resumed(currentRunning);
                else
                    output_started(currentRunning);
                kill(currentRunning->pid, SIGCONT);
            }
        }
        //printf("%d\n", *memAdr);
        *memAdr = currentRunning->remainingtime;

        // View queue
        QUEUE_NODE *ptr = PCB_Scheduling_Queue->front;
        while (ptr)
        {
            printf("{ PID: %d, pri: %d }\t", ((PCB *)ptr->dataPtr)->pid, ((PCB *)ptr->dataPtr)->priority);
            ptr = ptr->next;
        }
        printf("%s", "\n");
        ////////////////////////////////////////////////////
    }
}

int CompareRunningTime(void *left, void *right)
{
    PCB *leftObj = (PCB *)left, *rightObj = (PCB *)right;
    if (leftObj->runningtime > rightObj->runningtime)
        return -1;
    else if (leftObj->runningtime < rightObj->runningtime)
        return 1;
    return 0;
}

int CompareRemainingTime(void *left, void *right)
{
    PCB *leftObj = (PCB *)left, *rightObj = (PCB *)right;
    if (leftObj->remainingtime > rightObj->remainingtime)
        return -1;
    else if (leftObj->remainingtime < rightObj->remainingtime)
        return 1;
    return 0;
}

// Lower priority number == higher priority
// We need to return 1 left's priority < right's priority
int ComparePriority(void *left, void *right)
{
    PCB *leftObj = (PCB *)left, *rightObj = (PCB *)right;
    if (leftObj->priority > rightObj->priority)
        return -1;
    else if (leftObj->priority < rightObj->priority)
        return 1;
    else
        return 0;
}

int dummy_compare(void *a, void *b)
{
    return 0;
}
////////////////////////////////////////////////////////////////////////
///////////////////////MEMORY ALGO///////////////////////////////////

void First_Fit_memAlgo(void)
{
    NODE *iterator_processes = WaitingPCBs->head;
    NODE *iterator_memory = MemoryList->head;
    while (iterator_processes)
    {
        //logic for first fit
        PCB *process_to_allocate = (PCB *)iterator_processes;
        while (iterator_memory)
        {
            memory_fragment *memory_to_cut = (memory_fragment *)(iterator_memory->dataPtr);
            bool flag1 = (memory_to_cut->theState == GAP);
            int free_size = memory_to_cut->length - memory_to_cut->start_position;
            bool flag2 = (free_size == process_to_allocate->memsize);
            if (flag1 == flag2)
            {
                // take the needed part now ;
                memory_fragment *memory_needed = (memory_fragment *)malloc(sizeof(memory_fragment));
                memory_needed->theState = PROCESS;
                memory_needed->start_position = memory_to_cut->start_position;
                memory_needed->length = process_to_allocate->memsize;
                memory_needed->id = process_to_allocate->id;

                int new_beginnig = memory_needed->start_position + memory_needed->length;
                new_beginnig++;

                memory_to_cut->length -= memory_needed->length;
                memory_to_cut->start_position = new_beginnig;

                _insert(MemoryList, get_before_node(MemoryList, iterator_memory), (void *)memory_needed);
            }
        }
    }

    // itterate on waitnngPCB and then fill PCB
    // algorithm to to allocate and then
}

void deallocateMemory(int process_id) // only called for ff,nf,bf
{
    // iterate on the list until we find the node with id that's equal to process_id
    // Then there are three cases
    // Case 1 - Both the previous node & the next node are filled
    // --> Simply deallocate the place and free it as a gap
    // Case 2 - One of the two nodes is free
    // --> Merge this node & that free node
    // Case 3 - Both nodes are free
    // Merge all three nodes into one

    // Edge cases
    // at the head --> (only check next)
    // at the read --> (only check prev)
    NODE *ptr = MemoryList->head;
    if (ptr == NULL)
    {
        printf("Memory list is emtpy - Failed to deallocate memory of process %d\n", process_id);
        return;
    }

    bool found = false;
    NODE *prev = ptr; // ptr to prev node;
    NODE *temp;

    while (ptr != NULL && !found)
    {
        if (((memory_fragment *)ptr->dataPtr)->id == process_id)
        {
            found = true;
            ((memory_fragment *)ptr->dataPtr)->id = -1;
            // Edge case 1
            if (ptr == MemoryList->head)
            {
                if (ptr->link != NULL)
                {
                    temp = ptr->link;
                    // I'm the head & the next node isn't null & is a gap
                    // I need to merge both nodes
                    if (((memory_fragment *)ptr->link->dataPtr)->theState == GAP)
                    {
                        ((memory_fragment *)ptr->dataPtr)->length += ((memory_fragment *)temp->dataPtr)->length;
                        ((memory_fragment *)ptr->dataPtr)->theState = GAP;
                        ptr->link = temp->link;
                        if (temp == MemoryList->rear)
                            MemoryList->rear = ptr;
                        free(temp->dataPtr);
                        free(temp);
                        MemoryList->count--;
                    }
                    // I'm the had & the next node isn't null & isn't a gap
                    else
                    {
                        ((memory_fragment *)ptr->dataPtr)->theState = GAP;
                    }
                }
                // I'm the head & the next node is null
                else
                {
                    ((memory_fragment *)ptr->dataPtr)->theState = GAP;
                }
            }
            // Edge case 2
            else if (ptr == MemoryList->rear)
            {
                // I already addressed the case where I'm the head so I don't need to check if
                // Ptr == prev
                // The prev node is also a gap
                if (((memory_fragment *)prev->dataPtr)->theState == GAP)
                {
                    ((memory_fragment *)prev->dataPtr)->length += ((memory_fragment *)ptr->dataPtr)->length;
                    ((memory_fragment *)prev->dataPtr)->id = -1;
                    prev->link = ptr->link;
                    free(ptr->dataPtr);
                    free(ptr);
                    MemoryList->rear = prev;
                    MemoryList->count--;
                }
                // The prev node is not a gap
                else
                {
                    ((memory_fragment *)ptr->dataPtr)->theState = GAP;
                }
            }
            // general case
            else
            {
                temp = ptr->link;
                // I have 3 cases here
                if (((memory_fragment *)prev->dataPtr)->theState == GAP &&
                    ((memory_fragment *)temp->dataPtr)->theState == GAP)
                {
                    ((memory_fragment *)prev->dataPtr)->length += ((memory_fragment *)ptr->dataPtr)->length +
                                                                  ((memory_fragment *)temp->dataPtr)->length;
                    ((memory_fragment *)prev->dataPtr)->id = -1;
                    prev->link = temp->link;
                    if (MemoryList->rear == temp)
                        MemoryList->rear = prev;
                    free(temp->dataPtr);
                    free(ptr->dataPtr);
                    free(temp);
                    free(ptr);
                    MemoryList->count -= 2;
                }
                else if (((memory_fragment *)prev->dataPtr)->theState == GAP &&
                         ((memory_fragment *)temp->dataPtr)->theState != GAP)
                {
                    ((memory_fragment *)prev->dataPtr)->length += ((memory_fragment *)ptr->dataPtr)->length;
                    ((memory_fragment *)prev->dataPtr)->id = -1;
                    prev->link = ptr->link;
                    free(ptr->dataPtr);
                    free(ptr);
                    MemoryList->count--;
                }
                else if (((memory_fragment *)prev->dataPtr)->theState != GAP &&
                         ((memory_fragment *)temp->dataPtr)->theState == GAP)
                {

                    ((memory_fragment *)ptr->dataPtr)->length += ((memory_fragment *)temp->dataPtr)->length;
                    ((memory_fragment *)ptr->dataPtr)->theState = GAP;
                    ptr->link = temp->link;
                    if (temp == MemoryList->rear)
                        MemoryList->rear = ptr;
                    free(temp->dataPtr);
                    free(temp);
                    MemoryList->count--;
                }
                else
                {
                    ((memory_fragment *)ptr->dataPtr)->theState = GAP;
                }
            }
        }
        else
        {
            prev = ptr;
            ptr = ptr->link;
        }
    }
    if (!found)
    {
        printf("Process with id %d not found in memory list\n", process_id);
        return;
    }
}

// Traverses down the memory binary tree searching for an approriate sized memory fragments
// Favours left children.
// After getting the current smallest fragment, it further splits it until it reaches a fragment not large enough to be split.
// It then updates how much actual memory is used in that node
// Note: don't update parents to allow other relative nodes (sibling or uncle) to be able to be allocated
void *allocateMemory_BSA()
{
    NODE *iterator_processes = WaitingPCBs->head;
    int requiredMem = ((PCB *)iterator_processes->dataPtr)->memsize;

    BinaryTreeNode *fittingMemoryFrag = NULL;
    FindSmallestFittingNode(memRoot, &requiredMem, &fittingMemoryFrag);

    ((BuddySystemData *)(fittingMemoryFrag->dataPtr))->actualAllocated = requiredMem;

    return (void *)fittingMemoryFrag;
}

void deallocateMemory_BSA(int process_id) //ony for BSA
{
    NODE *ptr = MemoryList->head;
    // Access the PCB, get a pointer to the binary tree node
    // Set the node's acutalAllocated to 0
    // Check the other sibling, if its acutalAllocated is also 0, merge
    // Free the remaining resources (PCB and pointers)
}
