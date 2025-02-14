#ifndef DATA_H
#define DATA_H

#define PROCESS_COUNT 10

typedef int pipefd[2];

typedef struct 
{
	pipefd parentToChild;
	pipefd childToParent;
	pid_t pid;
	bool working;
} child_t;

extern child_t child[PROCESS_COUNT];
extern int finalAnswer;


#endif  //DATA_H