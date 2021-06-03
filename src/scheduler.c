#include "headers.h"
#include <stdlib.h>
#include <string.h>
struct processData processGenBuffer;
int PG_S_MQid;
int sigAlgo;
// If the PG is sending a new process to the scheduler, it signals it before sending
// Via the message queue
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
    while (1)
    {
       
    }

    //TODO: upon termination release the clock resources.

    destroyClk(true);
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
