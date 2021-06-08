#include "headers.h"
#include "queue.h"
#include <stdlib.h>
#include <string.h>

#define QUANTUM 3

// TODO: Finish the other scheduling algorithms in the same way as the FCFS
// TODO: Work on the output data
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

// If the PG is sending a new process to the scheduler, it signals it before sending
// Then it sends the process data via a Message queue
void new_process_handler(int signum);

// Scheduling Algorithms
void First_Come_First_Serve_Scheduling(void);
void Round_Robin_Scheduling(void);
void PreemtiveHighestPriorityFirst(void);

// Variables used for output files
bool runningProcess;
float CPU_util;
int runningTime;
int idleTime;
int prevRunningTime;
int numOfProcs;
int waiting_time;
int waiting_turnaround_time;
QUEUE* LOG;
QUEUE* PERF;

// Comparison functions
int ComparePriority(void *, void *);

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

    // Output file variables
    LOG = createQueue();
    PERF = createQueue();

    CPU_util = 0;
    idleTime = 0;
    runningTime = 0;
    prevRunningTime = 0;
    numOfProcs = atoi(argv[2]);
    waiting_time = 0;
    waiting_turnaround_time = 0;

    printf("The number of Processes to schedule is %d\n", numOfProcs);

    // TODO: dpending on the value of sigAlgo, make the required Datastructures & use them

    // theAlogrithm should be equal to the wanted Algorithm (from 0 to 4)
    // we should determine the algo from the args in process generator
    Scheduling_Algorithm_Type theAlgorithm = atoi(argv[1]);

    // This switch case will be used to make
    // The necessary initializations for each alogrithm

    void (*AlgoToRun)(void);
    switch (theAlgorithm)
    {
    case FCFS:
        PCB_Scheduling_Queue = createQueue();
        AlgoToRun = &First_Come_First_Serve_Scheduling;

        break;
    case SJF:

        break;
    case HPF:
        PCB_Scheduling_Queue = createQueue();
        AlgoToRun = &PreemtiveHighestPriorityFirst;
        break;
    case SRTN:

        break;
    case RR:
        PCB_Scheduling_Queue = createQueue();
        AlgoToRun = &Round_Robin_Scheduling;
        break;

    default:
        break;
    }
    
    int currTime = getClk();

    while (numOfProcs > 0)
    {
        AlgoToRun();
        
        // Check if the CPU was idle for the last cycle

        if (prevRunningTime == runningTime && currTime != getClk())
        {
            idleTime += 1;
        }

        if (currTime !=  getClk())
        {
        // Calculate & Report the CPU Utilization
        if (idleTime == 0)
            CPU_util = 100;
        else
            CPU_util = (runningTime / ( (float) idleTime + runningTime ) ) * 100;

        currTime = getClk();
        }
        

    }
    
    fflush(stdin);
    printf("Scheduler is done computing\n");
    printf("CPU Utilization: %f\n", CPU_util);
    printf("Average Waiting time: %f\n", (float) waiting_time / runningTime );
    printf("Average Turnaround Waiting time: %f\n", (float) waiting_turnaround_time / runningTime);
    destroyClk(true);
}


void Output_handling()
{

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
            execl("bin/process", "process", (char *)NULL);
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
        runningProcess = true;
        PCB *front_process_queue;
        dequeue(PCB_Scheduling_Queue, (void *)&front_process_queue);

        //printf("process with id %d is now running\n", front_process_queue->pid);

        int end_time = getClk() + front_process_queue->remainingtime;

        //printf("end_time=%d\n",end_time);

        kill(front_process_queue->pid, SIGCONT);

        int currTime = getClk();

        *memAdr = front_process_queue->remainingtime;

        prevRunningTime = runningTime;

        while (front_process_queue->remainingtime > 0)
        {
            if (getClk() != currTime)
            {
                front_process_queue->remainingtime -= 1;
                *memAdr = front_process_queue->remainingtime;
                currTime = getClk();
                runningTime += 1;
            }
            //printf("current_time=%d",getClk());
            // blocking
        }

        
        //fflush(stdin);

        //printf("Process with id %d finished at time %d\n", front_process_queue->pid, getClk());

        //kill(front_process_queue->pid, SIGSTOP);

        // Delete the allocated data for this PCB
        waiting_turnaround_time += getClk() - front_process_queue->arrivaltime;
        waiting_time += getClk() - front_process_queue->arrivaltime - front_process_queue->runningtime;
        //printf("This process waiting for %d cycles\n", getClk() - front_process_queue->arrivaltime - front_process_queue->runningtime);
        free(front_process_queue);
        numOfProcs--;    
    }
}

void Round_Robin_Scheduling(void)
{
    PCB *ptr_to_arriving_processes;
    if (!emptyQueue(PCB_Scheduling_Queue))
    {
        PCB *front_process_queue;
        dequeue(PCB_Scheduling_Queue, (void *)&front_process_queue);

        //printf("process with id %d is now running\n", front_process_queue->pid);

        //int end_time = getClk() + front_process_queue->remainingtime;

        //printf("end_time=%d\n",end_time);

        int time_at_entry = getClk();
        int currTime = getClk();
        kill(front_process_queue->pid, SIGCONT);
        *memAdr = front_process_queue->remainingtime;

        // the loop will comtmiue until the QUANTUM is finished or the time of queue is finished
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
            kill(front_process_queue->pid, SIGSTOP);
        }
        else //free the memory if the process is finished
        {
            free(front_process_queue);
        }
        currTime = getClk();
        //get all the newly added processes while the Quantum was running
        while (!emptyQueue(PCBs))
        {
            dequeue(PCBs, (void *)&ptr_to_arriving_processes);
            if (ptr_to_arriving_processes->arrivaltime < currTime)
            {
                enqueue(PCB_Scheduling_Queue, (void *)ptr_to_arriving_processes);
            }

            //if there is newly added process in the same time
            // the previously running process ends
            // then the priorituy goes to the previoulsy running process
            else if (flag)
            {
                enqueue(PCB_Scheduling_Queue, (void *)front_process_queue);
                enqueue(PCB_Scheduling_Queue, (void *)ptr_to_arriving_processes);
                flag = 0;
            }
            else
            {
                enqueue(PCB_Scheduling_Queue, (void *)ptr_to_arriving_processes);
            }
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

void PreemtiveHighestPriorityFirst()
{
    static PCB *newProc, *currentRunning, *dequeuePtr;
    static int currTime = 0;

    // Check for an incoming process
    // If the process queue has a process, consume it
    if (!emptyQueue(PCBs))
    {
        dequeue(PCBs, (void *)(&newProc));
        printf("Process with ID: %d, remaining time: %d, priority: %d arrived. ", newProc->id, newProc->remainingtime, newProc->priority);
        currTime = getClk();

        if (currTime != getClk())
        {
            printf("[%d] ", currTime);
            fflush(stdout);
        }

        // If first process
        if (emptyQueue(PCB_Scheduling_Queue))
        {
            printf("%s", "The process is alone, inserting at the front of the queue. ");
            enqueue(PCB_Scheduling_Queue, (void *)newProc);
            currentRunning = newProc;
            *memAdr = currentRunning->remainingtime;
            kill(currentRunning->pid, SIGCONT);
        }
        else
        {
            printf("%s", "There are other processes, inserting somewhere in the queue. ");
            // If there's one, check if its priority is higher than the current priority
            if (ComparePriority(newProc, currentRunning) == 1)
            // If so, send SIGSTOP to the current process, place the new one in the queue, and send SIGSTRT to it if needed
            {
                printf("%s", "Process has higher priority than the current running process, replacing. ");
                kill(currentRunning->pid, SIGSTOP);
                currentRunning = newProc;
                *memAdr = currentRunning->remainingtime;
                kill(currentRunning->pid, SIGCONT);
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

        // If the process's time is out, remove it from the queue
        if (currentRunning->remainingtime == 0)
        {
            printf("%s", "A process has finished running, removing from the queue. ");
            dequeue(PCB_Scheduling_Queue, (void *)(&dequeuePtr));

            if (queueFront(PCB_Scheduling_Queue, (void *)(&dequeuePtr)))
            {
                printf("%s", "There are other processes left, dequeuing one. ");
                currentRunning = dequeuePtr;
                *memAdr = currentRunning->remainingtime;
                kill(currentRunning->pid, SIGCONT);
            }
        }
        printf("%d\n", *memAdr);
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