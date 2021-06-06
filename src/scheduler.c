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
void First_Come_First_Serve_Scheduling();

void Round_Robin_Scheduling(void);

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

    while (1)
    {
        AlgoToRun();
    }

    destroyClk(true);
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

        PCB *front_process_queue;
        dequeue(PCB_Scheduling_Queue, (void *)&front_process_queue);

        //printf("process with id %d is now running\n", front_process_queue->pid);

        int end_time = getClk() + front_process_queue->remainingtime;

        //printf("end_time=%d\n",end_time);

        kill(front_process_queue->pid, SIGCONT);

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

        //printf("Process with id %d finished at time %d\n", front_process_queue->pid, getClk());

        //kill(front_process_queue->pid, SIGSTOP);

        // Delete the allocated data for this PCB
        free(front_process_queue);
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
