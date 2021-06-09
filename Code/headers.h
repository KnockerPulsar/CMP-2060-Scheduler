#include <stdio.h> //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "definitions.h" // Custom definitions

typedef short bool;
#define true 1
#define false 0

#define SHKEY 300

///==============================
//don't mess with this variable//
int *shmaddr; //
//===============================

int getClk()
{
    return *shmaddr;
}

/*
 * All processes call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *)shmat(shmid, (void *)0, 0);
}

/*
 * All processes call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}

typedef struct processData
{
    int arrivaltime;
    int priority;
    int runningtime;  
    int id;
    int memsize;
    //int remaining_time
} processData;

// Message buffer for communication between Process Generator & the Scheduler
typedef struct PG_S_MB   // Process Generator Scheduler Message Buffer 
{
    long mtype;
    processData P;
}PG_S_MB;

// For the scheduler to store the processes' data in
typedef struct PCB
{
    int id; // not pid, this is the number that we get from the process generator
    int pid; // actual pid from getpid()
    int arrivaltime;
    int priority;
    int runningtime;
    int remainingtime;
    int memsize;
    void * memoryNode;
}PCB;


//this enum needed to be sent to the scheduler 
// so the scheduler will know which algo to perform
typedef enum Scheduling_Algorithm_Type
{
    PLACEHOLDER,
    FCFS,
    SJF,
    HPF,
    SRTN,
    RR
}Scheduling_Algorithm_Type;


// Struct for communication between the scheduler & the process
// Currently being used to send the remaining time
typedef struct S_P_MB
{
    int remaining_time;
    long mtype;
}S_P_MB;




typedef enum mem_state
{
    GAP, // so its free fragmen
    PROCESS  // so tis taken fragment
}mem_state;

typedef struct memory_fragment
{
    int start_position;
    int length;
    mem_state theState;
    int id;

}memory_fragment;

typedef enum MemoryAlgorithm
{
    PLACEHOLDER1,
    FF,
    NF,
    BF,
    BSA
}MemoryAlgorithm;


