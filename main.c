#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

volatile sig_atomic_t terminate_flag = 0;

typedef int pipefd[2];

typedef struct 
{
	pipefd parentToChild;
	pipefd childToParent;
	pid_t pid;
	bool working;
} child_t;

void terminate_children(child_t* child, int n) 
{
    for(int i=0; i<n; i++) 
	{
        kill(child[i].pid, SIGTERM);
    }

    for(int i=0; i<n; i++) 
	{
        waitpid(child[i].pid, NULL, 0);
    }
}

void handle_signal(int sig) {
	terminate_flag = 1;
}

bool fileExists(const char* fileName)
{
	struct stat st;

	if(stat(fileName, &st) < 0)
		return false;

	return true;
}

int createChildAndPipes(child_t* child, int processCount, char* filePath)
{
	for(int i=0; i<processCount; i++)
	{
		if(pipe(child[i].childToParent) == -1)
		{
			fprintf(stderr, "%s", strerror(errno));
			return -1;
		}
		if(pipe(child[i].parentToChild) == -1)
		{
			fprintf(stderr, "%s", strerror(errno));
			return -1;
		}
		

		child[i].pid = fork();
		if(child[i].pid == -1)
		{
			fprintf(stderr, "%s", strerror(errno));
			return -1;
		}else if(child[i].pid == 0)
		{
			FILE *fp = fopen(filePath, "a+");
			char message[200];
			char parent[100];
			char response[] = "done";

			while(true)
			{
				int b = read(child[i].parentToChild[0], parent, sizeof(parent));
				if(b <= 0) continue;

				sprintf(message, "<%d> -> <%s>\n", getpid(), parent);

				fwrite(message, sizeof(char), strlen(message), fp);
				fflush(fp);
				
				write(child[i].childToParent[1], response, sizeof(response));
			}
			
			fclose(fp);
						
			close(child[i].parentToChild[0]);
			close(child[i].childToParent[1]);

			exit(EXIT_SUCCESS);
		}

		child[i].working = false;
	}
}

/*void writeToChild(child_t* child, int processCount)
{
	for(int i=0; i<processCount; i++)
	{
		char message[] = "Hello child, I am your father and i call you: <childName>";
		write(child[i].parentToChild[1], message, sizeof(message));
	}

	fd_set readFD;
	int maxFD = 0;
	for(int i=0; i<n; i++)
	{
		if(child[i].childToParent[1] > maxFD)
		{
			maxFD = child[i].childToParent[1]; 
		}
	}


}*/

void readFromChild(child_t* child, int processCount)
{
	for(int i=0; i<processCount; i++)
	{
		char message[64];
		read(child[i].childToParent[0], message, sizeof(message));
		printf("%s\n", message);
	}
}

void parentCommunication(child_t* child, int processCount)
{
	fd_set readFD, writeFD;
    int maxFD = 0;

    for(int i=0; i<processCount; i++)
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

    char buffer[64];
    int counter = 0;

    while (1) 
	{
        FD_ZERO(&readFD);
        FD_ZERO(&writeFD);

        for (int i=0; i<processCount; i++)
		{
            FD_SET(child[i].childToParent[0], &readFD);  
            FD_SET(child[i].parentToChild[1], &writeFD); 
        }

        int ready = select(maxFD + 1, &readFD, &writeFD, NULL, NULL);
        if(ready < 0) 
		{
            perror("select");
            break;
        }

        for(int i=0; i<processCount; i++) 
		{
            if(FD_ISSET(child[i].childToParent[0], &readFD)) 
			{
                int bytes_read = read(child[i].childToParent[0], buffer, sizeof(buffer));
                if(bytes_read > 0) 
				{
                    buffer[bytes_read] = '\0';
					child[i].working = false;
                }else 
				{
					continue;
				}
            }

            if(FD_ISSET(child[i].parentToChild[1], &writeFD))
			{
				child[i].working = true;
				char message[] = "Hello child, I am your father and i call you: <childName>";
                write(child[i].parentToChild[1], message, sizeof(message));
            }
        }

		if(terminate_flag)
		{
			terminate_children(child, processCount);
			exit(EXIT_SUCCESS);
		}

        sleep(1);
    }
}



int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		fprintf(stderr, "Usage: ./rpc [file name] [process count]");
		return -1;
	}

	char *fileName = NULL;
	int processCount = 0;

	if(fileExists(argv[1]))
	{
		int nameLength = strlen(argv[1]);
		fileName = (char*)malloc(sizeof(char) * (nameLength + 1));
		strcpy(fileName, argv[1]);
		fileName[nameLength] = 0;
	}
	else
	{
		fprintf(stderr, "%s", strerror(errno));
		free(fileName);
		return -1;
	}

	if((processCount = atoi(argv[2])) == 0)
	{
		fprintf(stderr, "Not a number\n");
		return -1;
	}

    signal(SIGINT, handle_signal);

	FILE *fp = fopen(argv[1], "w");

	char message[100];
	sprintf(message, "[PARENT] -> <%d>\n", getpid());
	fwrite(message, sizeof(char), strlen(message), fp);
	fclose(fp);

	child_t* child = (child_t*)malloc(sizeof(child_t) * processCount);

	if(createChildAndPipes(child, processCount, fileName) == -1)
	{
		return -1;
	}

	parentCommunication(child, processCount);
	//writeToChild(child, processCount);
	//readFromChild(child, processCount);	

	for(int i=0; i<processCount; i++)
	{
	    wait(NULL);
 	}

	free(fileName);	
	return 0;
}
