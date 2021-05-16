#include "headers.h"

int main(int argc, char *argv[])
{
    initClk();

    // Just testing if the forking works fine
    printf("Scheduler spawned!\n");

    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.

    destroyClk(true);
}
