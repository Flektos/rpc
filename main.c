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

//Create Cn process with fork

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
			FILE *fp = fopen(argv[1], "a+");
			char message[100];
			sprintf(message, "[CHILD] -> <%d>\n", getpid());
			fwrite(message, sizeof(char), strlen(message), fp);
			fclose(fp);
			break;
		}
		
		//Parent
		if(pid > 0) {}
	}

	free(fileName);	
	return 0;
}
