#include "headers.h"
#include "queue.h"
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

// This queue is used to transfer process data from the new process handler to the current Algorithm function
QUEUE *PCBs;

//Used to store the Algorithm number. If this number is equal to 2 then elements are enqueued sorted by Running time in PCBs
// If this number is equal to 4 then elements are enqueued sorted by Remaining time in PCBs
// Any other algorithm is sorted normally
Scheduling_Algorithm_Type theAlgorithm;

// If the PG is sending a new process to the scheduler, it signals it before sending
// Then it sends the process data via a Message queue
void new_process_handler(int signum);

// Scheduling Algorithms
void First_Come_First_Serve_Scheduling(void);
void Shortest_Job_First_Scheduling(void);
void PreemptiveHighestPriorityFirst(void);
void Shortest_Remaining_Time_Next_Scheduling(void);
void Round_Robin_Scheduling(void);

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

int *Waiting;
int *Running;

int Enough;

// Comparison functions
int CompareRunningTime(void *, void *);
int ComparePriority(void *, void *);
int CompareRemainingTime(void *, void *);

int main(int argc, char *argv[])
{
    printf("Scheduler spawned!\n");

    initClk();
    // if the scheduler receives this signal, it means that the PG is sending it a new process
    signal(SIGUSR1, new_process_handler);

    // Initialize the queue
    PCBs = createQueue();

    processGenBuffer.mtype = getpid() % 10000;

    // Getting the ID of the PG_S MQ
    key_t kid1 = ftok(PROCGEN_SCHED_QKEY, 'A');
    PG_S_MQid = msgget(kid1, 0666 | IPC_CREAT);
    printf("PG_S_MQid = %d\n", PG_S_MQid);

    // Getting the ID of the S_P MQ
    key_t kid2 = ftok(SCHED_PROC_QKEY, 'B');
    S_P_ShMemid = shmget(kid2, sizeof(int), 0666 | IPC_CREAT);

    // Attach the shared memory to the scheduler
    memAdr = (int *)shmat(S_P_ShMemid, (void *)0, 0);

    printf("S_P_MQid = %d\n", S_P_ShMemid);

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

    InputFileName = argv[3]; //Uncomment after changing the project structure --> Probably won't need it anyway
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
    theAlgorithm = atoi(argv[1]);
    if (theAlgorithm == RR)
        QUANTUM = atoi(argv[4]);
    else
        QUANTUM = 0;

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
        int pid = fork();
        if (pid == 0) // Child
        {
            execl("process", "process", (char *)NULL);
        }

        // Pause the process that we just forked
        kill(pid, SIGSTOP);

        // Create a new PCB structure  & fill it will info
        // Needs to be dynamically created so that it persists after the handler ends
        PCB *tempPCB = malloc(sizeof(PCB));
        tempPCB->arrivaltime = processGenBuffer.P.arrivaltime;
        tempPCB->id = processGenBuffer.P.id;
        tempPCB->pid = pid;
        tempPCB->runningtime = processGenBuffer.P.runningtime;
        tempPCB->remainingtime = processGenBuffer.P.runningtime;
        tempPCB->priority = processGenBuffer.P.priority;
        //fflush(stdin);
        //printf("Remaining time %d\n", tempPCB->remainingtime);

        // Add this process to the new processes queue
        // The selected Algo can then take this new process and add it
        // To its ready queue (could different for every Algorithm)
        if (theAlgorithm == SJF)
            enqueue_sorted(PCBs, (void *)tempPCB, CompareRunningTime);
        else if (theAlgorithm == SRTN)
            enqueue_sorted(PCBs, (void *)tempPCB, CompareRemainingTime);
        else
            enqueue(PCBs, (void *)tempPCB);
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

        printf("Process with ID: %d, remaining time: %d, priority: %d arrived\n", newProc->id, newProc->remainingtime, newProc->priority);
        currTime = getClk();

        // if (currTime != getClk())
        // {
        //     printf("[%d] ", currTime);
        //     fflush(stdout);
        // }

        // If first process
        if (emptyQueue(PCB_Scheduling_Queue))
        {
            printf("%s", "The process is alone, inserting at the front of the queue\n");
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
            printf("%s", "There are other processes, inserting somewhere in the queue\n");
            // If there's one, check if its priority is higher than the current priority
            if (ComparePriority(newProc, currentRunning) == 1)
            // If so, send SIGSTOP to the current process, place the new one in the queue, and send SIGSTRT to it if needed
            {
                printf("%s", "Process has higher priority than the current running process, replacing\n");
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
            free(dequeuePtr);
            numOfProcs--;

            if (queueFront(PCB_Scheduling_Queue, (void *)(&dequeuePtr)))
            {
                printf("%s", "There are other processes left, dequeuing one\n");
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
            printf("%d \t", ((PCB *)ptr->dataPtr)->pid);
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

int ComparePriority(void *left, void *right)
{
    PCB *leftObj = (PCB *)left, *rightObj = (PCB *)right;
    if (leftObj->priority > rightObj->priority)
        return 1;
    else if (leftObj->priority < rightObj->priority)
        return -1;
    else
        return 0;
}