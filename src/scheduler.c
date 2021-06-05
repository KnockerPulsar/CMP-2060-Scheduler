#include "headers.h"
#include "queue.h"
#include <stdlib.h>
#include <string.h>

struct processData processGenBuffer;
int PG_S_MQid;
PCB* sendProcToAlgo = NULL;
int currProc;
// Queue to be used for first_come_first_serve, Round Robin
// Feel free to use it if you need it in any other algorithm
QUEUE *PCB_queue;
// This queue is used to transfer process data from the process handler to the current Algorithm function
QUEUE *PCBs;
// If the PG is sending a new process to the scheduler, it signals it before sending
// Via the message queue
void new_process_handler(int signum);
void First_Come_First_Serve_Scheduling();
int main(int argc, char *argv[])
{
    initClk();
    // if the scheduler receives this signal, it means that the PG is sending it a new process
    signal(SIGUSR1, new_process_handler);
    PCBs = createQueue(); 
    currProc = 0;
    // get the id of the PG_S message queue
    key_t kid = ftok(PROCGEN_SCHED_QKEY, 'A');
    PG_S_MQid = msgget(kid, 0666 | IPC_CREAT);
    printf("MQ_ID = %d\n", PG_S_MQid);
    // Just testing if the forking works fine
    printf("Scheduler spawned!\n");
    // TODO: dpending on the value of sigAlgo, make the required Datastructures & use them
    // switch case or if condition
    // Main scheduler loop

    //theAlogrithm should equal the wanted Algorithm
    // we should determine the algo from the args in process generator
    Scheduling_Algorithm_Type theAlgorithm = atoi(argv[1]);

    //this switch case will be used to make
    // necessary initalizatins for each alogrithm
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

    /*
    TODO :  make then switch case and figure out on what to switch 
    switch case
        case1: first_come_first_serve init queue 
    */
    while (1)
    {
        AlgoToRun();
    }

    //TODO: upon termination release the clock resources.

    destroyClk(true);
}

void new_process_handler(int signum)
{
    printf("New process received\n");
    int PG_S_recVal = msgrcv(PG_S_MQid, &processGenBuffer, sizeof(processGenBuffer), 0, !IPC_NOWAIT);
    if (PG_S_recVal != -1)
    {
        //Just for testing, feel free to remove this later
        int pid = fork();
        if (pid == 0) // Child
        {
            execl("bin/process", "process", (char *)NULL);
        }
        // Pause the process that we just forked
        kill(pid, SIGSTOP);
        // Create a new PCB  & fill it will info
        PCB* tempPCB = malloc(sizeof(PCB));
        tempPCB->arrivaltime = processGenBuffer.arrivaltime;
        tempPCB->id = processGenBuffer.id;
        tempPCB->pid = pid;
        tempPCB->runningtime = processGenBuffer.runningtime;
        tempPCB->remainingtime = processGenBuffer.runningtime;
        tempPCB->priority = 0; // for now
        enqueue(PCBs, (void *) tempPCB);

        // Add this PCB to the PCBs queue (This queue )

        // we may need to clear the receive buffer (not sure)
        // Afterwards, add it to different to whatever DS that's holding the process
        // This depends on which signal algo we're going to use
    }
    signal(SIGUSR1, new_process_handler);
}

void First_Come_First_Serve_Scheduling(void)
{   
    PCB *ptr_to_arriving_processes;
    
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
        int process_needed_time=front_process_queue->runningtime;
        //need here to put the execlp function but dont remmber the syntax now 
        int time_now = getClk();
        int end_time = time_now + front_process_queue->remainingtime;
        printf("end_time=%d\n",end_time);
        kill(front_process_queue->pid, SIGCONT);
        while (getClk() < end_time)
        {
            //printf("current_time=%d",getClk());
            // blocking
        }
        fflush(stdin);
        printf("THIS PROCESS FINISHED\n");
        kill(front_process_queue->pid, SIGSTOP);
        free(front_process_queue);        
    }
}

// Process gen
// Process Generator takes input (process info. & scheduler algorithm of choice)
// Creates Schedeuler process & clock process
// 1 - Send process info to sched. when it's arival time arrives (could send more than one process at the same time)
// [
//
//
//
//
//  ]
// ...
// 2 - Fork/create process & give it it's parameters
// 3 - Sorting Algos
// 4 - Create an array(or any other DS) of structs with info about each process (state, running time, remaining time etc..)
// 5 -
