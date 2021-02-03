
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 80

typedef struct HistoryNode {
	char command_name[MAX_LINE];
	struct HistoryNode * next;
} HistoryNode;

typedef struct HistoryQueue {
	int max_size;
	int size;
	struct HistoryNode * head;
	struct HistoryNode * tail;
} HistoryQueue;



HistoryNode * createNode(char command[]) {
	HistoryNode * newNode = (HistoryNode *)malloc(sizeof(HistoryNode *));
	strcpy(newNode->command_name, command);
	newNode->next = NULL;
	return newNode;
}

void push(HistoryQueue * historyQueue, char command[]) {
	if(historyQueue->size == 0) {
		historyQueue->head = createNode(command);
		historyQueue->tail = historyQueue->head;
		historyQueue->size++;

		return ;
	}

	if(historyQueue->size < historyQueue->max_size) {
		historyQueue->tail->next = createNode(command);
		historyQueue->tail = historyQueue->tail->next;
		historyQueue->size++;
		return;
	}
	
	historyQueue->head = historyQueue->head->next;
	historyQueue->tail->next = createNode(command);
	historyQueue->tail = historyQueue->tail->next;
	historyQueue->size++;
}

void print_history(HistoryQueue * historyQueue) {
	HistoryNode * head = historyQueue->head;
	int i;
	if(historyQueue->size < historyQueue->max_size) {
		i = 1;
	} else {
		i = historyQueue->size - historyQueue->max_size + 1;
	}
	printf("\nHistory\n");
	while (head != NULL)
	{
		printf("%d. %s \n", i, head->command_name);
		i++;
		head = head->next;
	}
}

void last_one(HistoryQueue * historyQueue) {
	if(historyQueue->tail == NULL) {
		printf("No command in history!\n");
		return;
	}

	printf("%d. %s \n", historyQueue->size, historyQueue->tail->command_name);
}

void nth_one(HistoryQueue * historyQueue, int n) {

	if(n < 0) {
		printf("The count starts from zero!");
		return;
	}

	if(historyQueue->size < n) {
		printf("There is no %dth command!\n", n);
		return;
	}

	int i;
	if(historyQueue->size < historyQueue->max_size) {
		i = 1;
	} else {
		i = historyQueue->size - historyQueue->max_size + 1;
	}
	HistoryNode * head = historyQueue->head;

	while (head != NULL)
	{
		if(i == n) {
			printf("%d. %s\n", i, head->command_name);
			return;
		}
		i++;
		head = head->next;
	}

	printf("There is no %dth command!\n", n);
	
}

int return_command_args(char inputBuffer[], char * args[], int *flag, HistoryQueue * historyQueue) {
    int length;

    length = read(STDIN_FILENO, inputBuffer, MAX_LINE);

    if(length < 0) {
        printf("Command not read...");
        exit(-1);
    }

    if(length == 0) {
        exit(0);
    }

    int count = 0;
    int start = -1;
    
	for(int i=0;i<length;i++)
	{
		switch(inputBuffer[i])
		{
			case ' ':
			case '\t':
				if(start != -1)
				{	
					args[count]=&inputBuffer[start];
					count++;
				}
				inputBuffer[i]='\0';
				start = -1;
				break;

			case '\n':
				if(start != -1)
				{
					args[count] = &inputBuffer[start];
					count++;
				}
				inputBuffer[i]='\0';
				start = -1;
				break;

			default :
				if(start == -1)
					start = i;
				if(inputBuffer[i] == '&')
				{
					*flag=1;
					inputBuffer[i] ='\0';
				}
        }
    }


    args[count] = NULL;

	if( strcmp(args[0], "history") == 0) {
		print_history(historyQueue);
		return -1;
	}

	if( strcmp(args[0], "!") == 0) {
		if(count > 1) {
			nth_one(historyQueue, atoi(args[1]));
		}
		
		return -1;
	}

	if( strcmp(args[0], "!!") == 0) {
		last_one(historyQueue);
		return -1;
	}

	char commands[MAX_LINE] = "";
	for(int i = 0; i < count; i++) {
		strcat(commands, args[i]);
		strcat(commands, " ");
	}
	
	push(historyQueue, commands);

	return 0;
}

int main(void)
{

	char inputBuffer[MAX_LINE];
	int flag;
	char *args[MAX_LINE/2 + 1];
	int should_run=1;

	pid_t pid;
	HistoryQueue * historyQueue = (HistoryQueue *)malloc(sizeof(HistoryQueue *));
	historyQueue->size = 0;
	historyQueue->max_size = 10;
	historyQueue->head = NULL;
	historyQueue->tail = NULL;
	while(should_run)
	{

		flag=0;
		printf("osh>");
		fflush(stdout); 
        pid_t child;
        if(return_command_args(inputBuffer,args, &flag, historyQueue) != -1) {
			child = fork();
			if(child < 0) 
			{
				printf("Fork failed!");
				exit(1);
			}
			else if(child == 0) 
			{
				if(execvp(args[0], args) == -1)
				{
					printf("Error executing unix command\n");
				}
			} 
			else 
			{
				if(flag == 0) {
					wait(&child);
				}
			}
		}
    }
}

