#include "headers.h"

/* Modify this file as needed*/
int remainingtime = 5;

int main(int agrc, char *argv[])
{
    initClk();

    //TODO The process needs to get the remaining time from somewhere
    //remainingtime = ??;
    
    // Maybe from the scheduler? Remaining time should depend on how many time steps this process was 
    // active.
    while (1/* remainingtime > 0 */)
    {
        // remainingtime = ??;
    }

    destroyClk(false);

    return 0;
}
