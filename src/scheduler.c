#include "headers.h"
#include "queue.h"
#include <stdlib.h>
#include <string.h>

struct processData processGenBuffer;
int PG_S_MQid;
int sigAlgo;

// Queue to be used for first_come_first_serve, Round Robin
// Feel free to use it if you need it in any other algorithm
QUEUE *PCB_queue;

// If the PG is sending a new process to the scheduler, it signals it before sending
// Via the message queue
void new_process_handler(int signum);
void First_Come_First_Serve_Scheduling();
int main(int argc, char *argv[])
{
    initClk();
    // if the scheduler receives this signal, it means that the PG is sending it a new process
    signal(SIGUSR1, new_process_handler);
    // get the id of the PG_S message queue
    key_t kid = ftok(PROCGEN_SCHED_QKEY, 'A');
    PG_S_MQid = msgget(kid, 0666 | IPC_CREAT);
    printf("MQ_ID = %d\n", PG_S_MQid);
    sigAlgo = atoi(argv[1]);
    // Just testing if the forking works fine
    printf("Scheduler spawned!\n");
    // TODO: dpending on the value of sigAlgo, make the required Datastructures & use them
    // switch case or if condition
    // Main scheduler loop

    //theAlogrithm should equal the wanted Algorithm
    // we should determine the algo from the args in process generator
    Scheduling_Algorithm_Type theAlgorithm = FCFS;

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
    int PG_S_recVal = msgrcv(PG_S_MQid, &processGenBuffer, sizeof(processGenBuffer), 0, !IPC_NOWAIT);
    if (PG_S_recVal != -1)
    {
        //Just for testing, feel free to remove this later
        int pid = fork();
        if (pid == 0) // Child
        {
            execl("bin/process", "process", (char *)NULL);
        }
        // TODO: Pause the current process  (probably though a signal)
        // TODO: create the new PCB for this process (PCB can be found in headers.h) change it as you like

        // we may need to clear the receive buffer (not sure)
        // Afterwards, add it to different to whatever DS that's hodling tthe process
        // This depends on which signal algo we're going to use
    }
    signal(SIGUSR1, new_process_handler);
}

void First_Come_First_Serve_Scheduling(void)
{

    PCB *ptr_to_arriving_processes;

    while (!emptyQueue(PCB_queue))
    {
        PCB process_to_perform_now;
        PCB *front_process_queue;
        dequeue(PCB_queue, (void *)&front_process_queue);
        int time_now;
        int process_needed_time=front_process_queue->runningtime;
        //need here to put the execlp function but dont remmber the syntax now 
        for(int i=0;i<process_needed_time;i++)
        {
            time_now=getClk;
            while(time_now==getClk()); //stuck until the time step if finished

        }
       
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
