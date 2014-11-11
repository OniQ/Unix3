#include "taskManager.h"

char **splitLine(char *line, const char *delimiter)
{
    char **lineWords = NULL;
    char *pch = strtok(line, delimiter);
    int wordCounter = 0;
    lineWords = malloc(sizeof(char *));
    while (pch != NULL) {
        lineWords = (void *) realloc(lineWords,
                        sizeof(char *) *wordCounter +
                        sizeof(char *));
    lineWords[wordCounter] = (char *) malloc(strlen(pch) + 1);
    lineWords[wordCounter] = pch;
    wordCounter++;
    pch = strtok(NULL, delimiter);
    };
    lineWords[wordCounter] = NULL;
    return lineWords;
}

int createTask(char* name, char* content){
    mkdir("tasks", 0700);
    char taskFile[30] = "tasks/";
    if (name != NULL)
        strcat (taskFile, name);
    int task = open(taskFile, O_RDWR | O_CREAT | O_SYNC, 0700);
    if (task == -1)
        return task;
    if (content != NULL){
        write(task, content, strlen(content));
    }
    return 1;
}

char* getTask(char* name){
    FILE * task;
    char* taskBuffer;
    printf("getting %s", name);
    char taskFile[30] = "tasks/";
    if (name != NULL){
        strcat (taskFile, name);
        task = fopen(taskFile, O_RDONLY);
    }
    else 
        return NULL;
    if (task == NULL)
        return NULL;
    fseek(task, 0L, SEEK_END);
    int fileSize = ftell(task);
    printf("File size: %d", fileSize);
    taskBuffer = malloc(fileSize);
    if (taskBuffer == NULL){
        fprintf(stderr, "malloc for file buffer failed\n");
        exit(EXIT_FAILURE);
    }
    fseek(task, 0L, SEEK_SET);
    fread(taskBuffer, 1, fileSize, task);
    printf("File: %s", taskBuffer);
    return taskBuffer;
}

char* processCommand(char* request){
    char** commands = splitLine(request, " ");
    if (*commands != NULL && strcmp("create", commands[0]) == 0){
        int res = createTask(commands[1], commands[2]);
        if (res == 1)
            return "Task created successfully";
        return "Task creation failed";
    }
    /*else if (*commands != NULL && commands[1] != NULL && strcmp("get", commands[0]) == 0){
        printf("Getting\n");
        char* task = getTask(commands[1]);
        if (task != NULL)
            return task;
        return "Failed to get task";
    }*/
    else
        return "Unknown command";
}
