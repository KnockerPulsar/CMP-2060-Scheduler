#include "headers.h"

/* Modify this file as needed*/

// Variables for communication between the scheduler & each process


int main(int agrc, char *argv[])
{
    initClk();

    fflush(stdin);
    printf("Process with PID %d initialized", getpid());

    int S_P_MQid = msgget(ftok(SCHED_PROC_QKEY, 'B'), 0666 |IPC_CREAT);

    S_P_MB recvBuff;
    recvBuff.remaining_time  = 1; // initial value
    recvBuff.mtype = getpid() % 10000;

    while (recvBuff.remaining_time > 0)
    {
        msgrcv(S_P_MQid, &recvBuff, sizeof(recvBuff.remaining_time), recvBuff.mtype, IPC_NOWAIT);
    }

    destroyClk(false);


    fflush(stdin);
    printf("Terminating process with PID %d at time %d\n", getpid());
    // Terminate this process
    raise(SIGKILL);

    return 0;
}
