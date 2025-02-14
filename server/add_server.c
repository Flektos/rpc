#include "add.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "data.h"

void parentCommunication(child_t child[PROCESS_COUNT], numbers *argp)
{
	fd_set readFD, writeFD;
    int maxFD = 0;

    for(int i=0; i<PROCESS_COUNT; i++)
	{
        if(child[i].childToParent[0] > maxFD) 
		{
            maxFD = child[i].childToParent[0];
        }
        if(child[i].parentToChild[1] > maxFD) 
		{
            maxFD = child[i].parentToChild[1];
        }
    }

	int childAnswer  = 0;
	int childAnswerCounter = 0;
	int stillWriting = 0;

    while (true) 
	{
        FD_ZERO(&readFD);
        FD_ZERO(&writeFD);

        for (int i=0; i<PROCESS_COUNT; i++)
		{
            FD_SET(child[i].childToParent[0], &readFD);  
            FD_SET(child[i].parentToChild[1], &writeFD); 
        }

        int ready = select(maxFD + 1, &readFD, &writeFD, NULL, NULL);
        if(ready < 0) 
		{
			fprintf(stderr, "Select error\n");
            break;
        }

        for(int i=0; i<PROCESS_COUNT; i++) 
		{
            if(FD_ISSET(child[i].childToParent[0], &readFD)) 
			{
				int rb = read(child[i].childToParent[0], &childAnswer, sizeof(childAnswer));
                if(rb > 0) 
				{
					child[i].working = false;
					childAnswerCounter++;
					finalAnswer += childAnswer;
                }
            }

			if(childAnswerCounter == PROCESS_COUNT) return;

            if(stillWriting < PROCESS_COUNT)
			{
				int number = argp->nums[i];
				child[i].working = true;
                write(child[i].parentToChild[1], &number, sizeof(number));
				stillWriting++;
			}
			     
        }
		sleep(1);
    }
}


int *
add_1_svc(numbers *argp, struct svc_req *rqstp)
{
	static int  result;

	parentCommunication(child, argp);

	result = finalAnswer;

	finalAnswer = 0;

	return &result;
}