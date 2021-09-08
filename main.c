#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <unistd.h>
#include <stdbool.h>
#include "header.h"
#include <limits.h>
#include "history.c"

String aliasKey[10];
String aliasCommand[10];


int aliasCounter = 0;

void readerChecker(char *const *tokens, const char *theHome, int historyCount, char *const *history);

int main() {

    char *pathString = getenv("PATH");
    printf("%s", pathString);
    reader(pathString);
    setenv("PATH", pathString, 1);
    printf("Path: %s\n", getenv("PATH"));
    return 0;
}

/*
 * Gets user input
*/
void reader(char *pathString) {
    char input[512] = {0};
    char *tokens[50] = {0};
    chdir(getenv("HOME"));
    char *theHome = getenv("HOME");

    int static historyCapacity = 20;
    int historyCount = 0;
    char **history;
    history = (char **) malloc(700);

    printf("\n ** Welcome to Group 13s Simple Shell. To close this, type 'exit' or press CTRL D. **\n");
    printf("**************************************************************************************\n");
    printf("%s>", getenv("HOME"));
    fgets(input, 512, stdin);
    while (strcmp("exit\n", input) != 0) {
        if (feof(stdin)) {
            clearerr(stdin);
            setenv("PATH", pathString, 1);
            printf("Path: %s\n", getenv("PATH"));
            exit(0);
        } else {
            if (strcspn(input, "!") == 0) {
                strcpy(input, historyAt(history, input, historyCount));
            } else {
                history = addToHistory(history, input, historyCount, historyCapacity);
                historyCount = updateHistoryCount(historyCount, historyCapacity);
            }
            char *token = strtok(input, "| > < & ; \t \n");
            int i = 0;
            while (token != NULL) {
                tokens[i] = strdup(token);
                token = strtok(NULL, "| > < & ; \t \n");
                i++;
            }
            tokens[i] = NULL;

            /*alias*/
            if (getAliasPosition(tokens[0]) >= 0) {
                char *newInput = findAlias(getAliasPosition(tokens[0]));
                String newTokens[50];

                char *Ntoken = strtok(newInput, "| > < & ; \t \n");
                int i = 0;
                while (Ntoken != NULL) {
                    newTokens[i] = strdup(Ntoken);
                    Ntoken = strtok(NULL, "| > < & ; \t \n");
                    i++;
                }
                newTokens[i] = NULL;
                executeCommand(newTokens);
                printf("%s>", getenv("HOME"));
                fgets(input, 512, stdin);
                continue;
            }

            /*reader input for running commands*/
            readerChecker(tokens, theHome, historyCount, history);
            printf("%s>", getenv("HOME"));
            fgets(input, 512, stdin);
        }
    }
}


void readerChecker(char *const *tokens, const char *theHome, int historyCount, char *const *history) {
    if (tokens[0] != NULL) {
        if (strcmp(tokens[0], "setpath") == 0) { setpath(tokens); }
        else if (strcmp(tokens[0], "getpath") == 0) { getpath(tokens); }
        else if (strcmp(tokens[0], "cd") == 0) { cd(theHome, tokens); }
        else if (strcmp(tokens[0], "history") == 0) { printHistory(history, historyCount); }
        else if (strcmp(tokens[0], "alias") == 0 && tokens [1] !=NULL && tokens[2] != NULL && tokens[3] != NULL){
            printf("Please use the format: alias <alias> <command>\n>");
        } else if(strcmp(tokens[0], "alias") == 0 && tokens [1] !=NULL && tokens[2] == NULL){
            printf("Please use the format: alias <alias> <command>\n>");
        } else if (strcmp(tokens[0], "alias") == 0 && tokens[1] != NULL && tokens[2] != NULL) {
            aliasCom(tokens[1], tokens[2]);
        } else if (strcmp(tokens[0], "unalias") == 0 && tokens[1] != NULL) {
            removeAlias(tokens[1]);
            printf(">");
        } else if (strcmp(tokens[0], "alias") == 0 && tokens[1] == NULL) {
            listAliases();
        } else{
            executeCommand(tokens);
        }
    }
}



void executeCommand(char *tokens[]) {
    printf("Tokens 0 is: %s\n", tokens[0]);
    printf("Tokens 1 is %s\n", tokens[1]);
    printf("Alias position: %d\n", getAliasPosition(tokens[0]));
    pid_t c_pid, pid;
    int status;
    c_pid = fork();
    if (c_pid == -1) {
        perror("fork failed");
        _exit(1);
    }
    if (c_pid == 0) {
        execvp(tokens[0], tokens);
        perror(tokens[0]);
        _exit(1);
    } else if (c_pid > 0) {
        pid = wait(&status);
        if (pid < 0) {
            perror("wait failed");
            _exit(1);
        }
    }
}


void aliasCom(String name, String command) {

    int pos = getAliasPosition(name);
    if (pos != -1) {
        printf("Alias '%s' has been overwritten.\n", name);
        aliasCommand[pos] = command;
    } else if (aliasCounter >= 10) {
        printf("There are 10 aliases, and you can only have maximum 10 aliases.\n");
    } else {
        aliasKey[aliasCounter] = name;
        aliasCommand[aliasCounter] = command;
        printf("Alias '%s' has been added successfully.\n", name);
        aliasCounter++;
    }

}

void removeAlias(String alias) {
    if (getAliasPosition(alias) >= 0) {
        printf("Alias '%s' has been removed\n", alias);
        for (int i = getAliasPosition(alias); i < aliasCounter - 1; i++) {
            strcpy(aliasKey[i], aliasKey[i + 1]);
            strcpy(aliasCommand[i], aliasCommand[i + 1]);
        }
        aliasCounter--;
    }   else{
        printf("Alias '%s' cannot be found. Please try again!\n", alias);
    }
}

void getpath(char *tokens[]) {
    if (tokens[1] != NULL) {
        printf("This call cannot have parameters.\n");
    } else {
        printf("Path: %s\n", getenv("PATH"));
    }
}

void setpath(char *tokens[]) {
    if (tokens[1] == NULL) {
        printf("This call needs one parameter.\n");
    } else if (tokens[2] != NULL) {
        printf("This call can only have one parameter.\n");
    } else {
        setenv("PATH", tokens[1], 1);
    }
}

void cd(char *theHome, char *tokens[]) {
    if (tokens[1] == NULL) {
        if (chdir(theHome) == 0) {
            setenv("HOME", theHome, 1);
        } else {
            perror(getenv("HOME"));
        }
    } else if (tokens[2] != NULL) {
        printf("This call can only have one parameter.\n");
    } else {

        if (chdir(tokens[1]) == 0) {
            char cwd[PATH_MAX];
            //maybe remove else part
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                setenv("HOME", cwd, 1);
            } else {
                perror("cwd error");
            }
        } else {
            char *error = strcat(tokens[1], ":No such file or directory");
            perror(error);
        }

    }

}

int getAliasPosition(String x) {             // tests if a name is already in the alias arraylist
    for (int i = 0; i < aliasCounter; i++) {
        if (strcmp(x, aliasKey[i]) == 0) {
            return i;
        }
    }
    return -1;
}

String findAlias(int x) {
    return aliasCommand[x];
}

String getCommand(String key){
    for(int i=0;i<aliasCounter;i++){
        if(strcmp(key, aliasKey[i]) == 0){
            return aliasCommand[i];
        }
    }
    return "\0";
}

void listAliases() {
    if(aliasCounter == 0){
        printf("There are no aliases at the moment! Use format: alias <alias> <shell command>\n");
    }
    for (int i = 0; i < aliasCounter; i++) {
        printf("Original Command: %s    Alias: %s\n", aliasCommand[i], aliasKey[i]);
    }
}
