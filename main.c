#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

bool fileExists(const char* fileName)
{
	struct stat st;

	if(stat(fileName, &st) < 0)
		return false;

	return true;
}

typedef int pipefd[2];

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

	FILE *fp = fopen(argv[1], "w");

	char message[100];
	sprintf(message, "[PARENT] -> <%d>\n", getpid());
	fwrite(message, sizeof(char), strlen(message), fp);
	fclose(fp);

	pipefd* childPipes = malloc(sizeof(pipefd) * processCount);

	for(int i=0; i<processCount; i++)
	{
	    pipe(childPipes[i]);
	}

	for(int i=0; i<processCount; i++)
	{


		pid_t pid = fork();

		if(pid < 0)
		{
			fprintf(stderr, "%s", strerror(errno));
			return -1;
		}
			
		//Child
		if(pid == 0)
		{
			close(childPipes[i][1]);

			FILE *fp = fopen(argv[1], "a+");
			char message[200];
			char parent[100];
			read(childPipes[i][0], parent, sizeof(parent));
			sprintf(message, "<%d> -> <%s>\n", getpid(), parent);

			fwrite(message, sizeof(char), strlen(message), fp);
			fclose(fp);
			close(childPipes[i][0]);
			break;
		}
	}


	for(int i=0; i<processCount; i++)
	{
		close(childPipes[i][0]);

		char message[] = "Hello child, I am your father and i call you: <childName>";

		write(childPipes[i][1], message, sizeof(message));

		close(childPipes[i][1]);
	}

	for(int i=0; i<processCount; i++)
	{
	    wait(NULL);
 	}

	free(childPipes);
	free(fileName);	
	return 0;
}
