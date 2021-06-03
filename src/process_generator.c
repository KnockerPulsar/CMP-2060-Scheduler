#include "headers.h"
#include <stdlib.h>
#include <string.h>

/* 
The process generator should simulate a real operating systems as follows:
    •[DONE]It reads the input files containing the information about processes (check the
    input/output section below).

    • It gets the chosen scheduling algorithm and its parameters, if any, as
    command line arguments. These arguments are specified after the name of the
    program in the system command line, and their values are passed to the
    program during execution.

    •[DONE] It initiates and creates the scheduler and clock processes.

    •[DONE] It creates a data structure for processes and fills it with its parameters (e.g.
    arrival time, running time, etc.)

    • It sends the information to the scheduler at the appropriate time (when a
    process arrives), so that the scheduler places it correctly in its turn among
    the existing processes. A process should not be sent to the scheduler until it
    arrives to the system.
    
    • At the end of simulation, the process generator should clear all IPC resources.
 */

/* 
To view a dynamic array in VSCode, you can do *ptr@numElements in the watch window.
So for a dynamic string called "line" with a length of 12, you do *line@12.
If you ask for more elements than the pointer has allocated for itself, the extra elements
wwill be garbage. 
*/

/* ============================================================================================= */

// To kill the scheduler and clock in addition to the process generator upon ctrl+c
void clearResources(int sig_num);

// To read the process data from disk
processData *ReadSimData(char *filePath);

/* ============================================================================================= */

// Assuming the process data file and scheduling algorithm number will be passed to this file.
// To debug this, go into /.vscode/launch.json, locate the args array, write each arg as a string.

/*
    Try to run through the console using ./process_generator
    since Ctrl+C to SIGINT doesn't work well in VSCode's console.
*/
int main(int argc, char *argv[])
{
    // Input checking
    if (argc < 3)
    {
        perror("\nPROCESS GENERATOR ERROR: TOO FEW PARAMETERS, PLEASE SPECIFY THE PROCESS DATA FILE AND THE SCHEDULING ALGORITHM NUMBER");
        exit(-1);
    }
    signal(SIGINT, clearResources);

    // TODO Initialization
    // Initialization of what exactly? Hmm..

    // 1. Read the input files.]
    /* ============================================================================================= */

    char *pDataPath = argv[1];

    // File path should be ().txt, so at least 5 chars
    if (strlen(pDataPath) < 5)
    {
        perror("\nPROCESS GENERATOR ERRO: INVALID FILE PATH");
        exit(1);
    }

    processData *pData = ReadSimData(pDataPath);

    // 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.
    /* ============================================================================================= */

    int schedulingAlg = atoi(argv[2]);

    // TODO: PUT A FUNCTION HERE TO READ IN THE REQUIRED PARAMETERS FOR EACH SCHEDULING ALGORITHM.


    // Before forking the scheduler, let's create the message queue we will send the process data over

    // Added a cast to please the compiler, hope it doesn't cause issues
    int procGenSchedMsqQID = msgget(ftok(PROCGEN_SCHED_QKEY, 'A'), 0666 | IPC_CREAT); 
    if (procGenSchedMsqQID == -1)
    {
        perror("PROCESS GENERATOR: Error in creating the PG_S MQ\n");
        exit(-1);
    }
    else
    {
        printf("PROCESS GENERATOR: SCHEDULER MESSAGE QUEUE CREATED SUCCESSFULLY\n");
    }
    

    // 3. Initiate and create the scheduler and clock processes.
    /* ============================================================================================= */

    // Child 0 should be the clock, child 1 should be the scheduler process
    int childIndex, pID;
    const int NUM_CHILDREN = 2;

    for (int i = 0; i < NUM_CHILDREN; i++)
    {
        childIndex = i;
        pID = fork();
        if (pID == 0)
        {
            switch (childIndex)
            {
            case ClockChild:
            {
                printf("Clock child!\n");
                execl("bin/clk", "clk", (char *)NULL);
                break;
            }
            case SchedChild:
            {
                printf("Scheduler child!\n");
                execl("bin/scheduler", "scheduler", schedulingAlg ,(char *)NULL);
                break;
            }
            default:
                break;
            }
        }
    }

    /* ============================================================================================= */
    // TODO Generation Main Loop
    // 4. Use this function after creating the clock process to initialize clock.
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.

    initClk();

    // Checks every 0.5 seconds if the clock changed
    // Don't have another way of checking the clock on my mind right now

    // Maybe we can cause the clock to signal to the scheduler and process generator on ticking?
    // Another solution is to use a busy wait loop
    while (1)
    {
        static int prevTime = 0, currProcess = 0;
        int x = getClk(); // To get time use this function.

        if (prevTime != x)
        {
            printf("\n[%d] ", x);
            fflush(stdout);
        }
        prevTime = x;

        // If a proccess arrives (arrivalTime == currentTime)
        if (x == pData[currProcess].arrivaltime)
        {
            // Send the process data up to the scheduler
        }

        usleep(0.1 * 10e5); // Sleep for 0.1 seconds
    }

    // 7. Clear clock resources
    destroyClk(true);
}

//TODO Clears all resources in case of interruption
void clearResources(int sig_num)
{
    printf("\nINTERRUPTED\n");
    killpg(getpgrp(), SIGINT);
    exit(0);
}

processData *ReadSimData(char *filePath)
{
    // Read through processes.txt to find out how many processes there are.
    // The line doesn't count if the first char is #
    FILE *pFile;
    pFile = fopen(filePath, "r");
    if (pFile == NULL)
    {
        perror("PROCESS GENERATOR: ERROR OPENING FILE");
        exit(-1);
    }

    const int buffSize = 32;   // For readiblility
    int lines = 0, readChars;  // To store the number of lines and how many chars were read
    size_t lineLen = buffSize; // To tell getline() the size of our buffer.

    // Allocating the buffer and creating a pointer to the start since getline() doesn't like passing
    // the char array...
    char line[buffSize], *linePtr = line;

    // First parameter is a reference to a string, where the line will be returned
    // Second one is the length of the string of avoid overflows I guess
    // Third parameter is the filestream we're reading from
    // Returns the number of chars actually read.
    // A better approach would be to do it dynamically (ie. read line and add to the process data
    // array), but no dynamic arrays :(

    while ((readChars = getline(&linePtr, &lineLen, pFile)) != -1)
    {
        if (line[0] == '#')
            continue;
        lines++;
    }

    // Allocating memory for the process data
    processData *pData = malloc(sizeof(processData) * lines);
    rewind(pFile); // Reset the file pointer to the start of the file.
    int pIndex = 0;
    while ((readChars = getline(&linePtr, &lineLen, pFile)) != -1)
    {
        if (line[0] == '#')
            continue;
        char *splitPtr = strtok(line, "\t");
        pData[pIndex].id = atoi(splitPtr);

        splitPtr = strtok(NULL, "\t");
        pData[pIndex].arrivaltime = atoi(splitPtr);

        splitPtr = strtok(NULL, "\t");
        pData[pIndex].runningtime = atoi(splitPtr);

        splitPtr = strtok(NULL, "\t");
        pData[pIndex].priority = atoi(splitPtr);

        pIndex++;
    }
    fclose(pFile);
    return pData;
}
