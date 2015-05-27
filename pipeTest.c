#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include <signal.h>
#include <sys/select.h>
#define BUFFER_SIZE 128

FILE *fp;  //output file
struct timeval starttime;
int timeout=0;
// The SIGALRM interrupt handler.
void SIGALRM_handler(int current_signal)
{
    assert(current_signal == SIGALRM);
    printf("\nTime is up!\n");
    
    timeout = 1;
    exit(0);
}

int main (int argc, char** argv)
{
    fp = fopen("Output.txt", "w");
    fprintf(fp, "\n ~~~~~~~~~~~~~~~~~~~Read Time| Wrote Time | Message ~~~~~~~~~~~~~\n");
    char buffer[BUFFER_SIZE];
    struct itimerval tval;
    timerclear(& tval.it_interval);
    timerclear(& tval.it_value);
    tval.it_value.tv_sec = 30;    // 30 second timeout
    tval.it_value.tv_usec = 0;
    int fd[5][2];
    time_t startTime;
    time(&startTime); //set start time
    gettimeofday(&starttime, NULL);
    setitimer(ITIMER_REAL, &tval, NULL);  // set timer
    srand(time(NULL)); // set the random seed
    fd_set set,fileset;
    FD_ZERO(&set);
    FD_SET(0, &set); 
    int i, pid;
    for(i=0; i<5; i++)
    {
        pipe(fd[i]);
        FD_SET(fd[i][0], &set);
        pid = fork();
        if(pid == 0)
        {
			fflush(stdout);
			break;
        }
    }
    
    (void) signal(SIGALRM, SIGALRM_handler);
    
    //Creating a child process
    if (pid == 0)
    {
        int messageCount = 0;
        while(!timeout)
        {
            fileset = set;
            //Child process
            if(i == 4)
            {
                struct timeval currenttime;
                gettimeofday(&currenttime, NULL);
                printf("Please Enter a value:");
                char message[BUFFER_SIZE];
                fgets(message, BUFFER_SIZE, stdin);
                float curtime = (float)((currenttime.tv_sec - starttime.tv_sec) + (currenttime.tv_usec - starttime.tv_usec)/1000000.);
                snprintf(buffer, BUFFER_SIZE, "%6.3f  %s\n",curtime, message);
                if(!timeout)
                {
                    close(fd[4][0]);
                    write(fd[4][1], buffer, strlen(buffer));
                }
            }
            else
            {
	            int r = rand()%3;
                sleep(r);
                struct timeval currenttime;
                gettimeofday(&currenttime, NULL);
                float curtime = (float)((currenttime.tv_sec - starttime.tv_sec) + (currenttime.tv_usec - starttime.tv_usec)/1000000.);
                snprintf(buffer, BUFFER_SIZE, "%6.3f  child %d message %d",curtime, i, messageCount++);
                if(!timeout)
                {
                    close(fd[i][0]);
                    write(fd[i][1], buffer, strlen(buffer));
                }
            }
        }
	    exit(0);
    }
    
    else //I am in the Parent Process
    {
	    while(!timeout)
        {
            fileset = set;
            int returnNum = select(FD_SETSIZE, &fileset, NULL, NULL, NULL);
            if(returnNum<0)
                exit(0);
            else if(returnNum > 0)
            {
                fflush(stdout);
                for (int j = 0; j < 5; ++j)
                {
                    if (FD_ISSET(fd[j][0], &fileset))
                    {
                        //Clearing the message buffer
                        if (!timeout)
                        {
                            struct timeval currenttime;
                            read(fd[j][0], buffer, BUFFER_SIZE);
                            gettimeofday(&currenttime, NULL);
                            float curtimeRead = (float)((currenttime.tv_sec - starttime.tv_sec) + ((currenttime.tv_usec - starttime.tv_usec)/1000000.));
                            fprintf(fp, "%6.3f %s\n", curtimeRead, buffer);
                        }
                    }
                }
            }
            else
                printf("No data\n");
        }
    }
    return 0;
}
