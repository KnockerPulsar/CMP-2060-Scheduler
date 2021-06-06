#include "headers.h"
#include "queue.h"
#include <stdlib.h>
#include <string.h>

// TODO: Finish the other scheduling algorithms in the same way as the FCFS
// TODO: Work on the output data
// TODO: in PG, clear all IPC recources

// Variables for communication between the PG & the scheduler
struct PG_S_MB processGenBuffer;
int PG_S_MQid;

// Variables for communication between the scheduler & each process
int S_P_MQid;
struct S_P_MB processSchedBuffer;

// Queue to be used for first_come_first_serve, Round Robin
// Feel free to use it if you need it in any other algorithm
QUEUE *PCB_queue;

// This queue is used to transfer process data from the new process handler to the current Algorithm function
QUEUE *PCBs;

// If the PG is sending a new process to the scheduler, it signals it before sending
// Then it sends the process data via a Message queue
void new_process_handler(int signum);

// Scheduling Algorithms
void First_Come_First_Serve_Scheduling();


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
    S_P_MQid = msgget(kid2, 0666 |IPC_CREAT);
    printf("S_P_MQid = %d\n", S_P_MQid);


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
        PCB_queue=createQueue();
        AlgoToRun=&First_Come_First_Serve_Scheduling;

        break;
    case SJF:

        break;
    case HPF:

        break;
    case SRTN:

        break;
    case RR:

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
        PCB* tempPCB = malloc(sizeof(PCB));
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
        enqueue(PCBs, (void *) tempPCB);

    }

    // Reassign this function as SIGUSR1 handler
    signal(SIGUSR1, new_process_handler);
}

void First_Come_First_Serve_Scheduling(void)
{   
    PCB *ptr_to_arriving_processes;
    
    // Check if there is a new process from the new process handler
    // If yes, add it to the ready queue (PCB_queue)
    if (!emptyQueue(PCBs))
    {
        dequeue(PCBs,(void *) &ptr_to_arriving_processes);
        enqueue(PCB_queue, (void *) ptr_to_arriving_processes);
    }
    
    if (!emptyQueue(PCB_queue))
    {

        PCB *front_process_queue;
        dequeue(PCB_queue, (void *)&front_process_queue);

        printf("process with id %d is now running\n", front_process_queue->pid);

        int end_time = getClk() + front_process_queue->remainingtime;

        //printf("end_time=%d\n",end_time);

        kill(front_process_queue->pid, SIGCONT);

        int currTime = getClk();

        processSchedBuffer.mtype = front_process_queue->pid % 10000;

        while (front_process_queue->remainingtime > 0)
        {
            if (getClk() != currTime)
            {
                front_process_queue->remainingtime -= 1;
                processSchedBuffer.remaining_time = front_process_queue->remainingtime;
                msgsnd(S_P_MQid, &processSchedBuffer, sizeof(processSchedBuffer), !IPC_NOWAIT);
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

