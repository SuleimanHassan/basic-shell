#include <unistd.h>
#include <string.h>
#include <sys/stat.h> // used for S_IRUSR and S_IWUSR
#include <sys/shm.h>  // used for shmget, shmat...
#include <stdio.h>    // used for printf
#include <unistd.h>   // used for fork(), usleep;
#include <stdlib.h>   // used for exit();
#include <sys/wait.h> // used for wait(); WEXITSTATUS


char *commands[] = {"ls", "generate", "sum", "subtract", "clear"};

void process_command(char *command);
void exec_command(char* command);
void exec_tee_command(char* command);
int is_valid(char *command);

int main(int argc, char const *argv[]) {
    char input[100];
    printf("\nWelcome to Suleiman's shell v1.0.2\n");
    printf("Type exit in order to exit, type clear to clear the screan :).\n\n");
    while (1) {
        printf("basic272-shell> ");
        fgets(input, 99, stdin);
        input[strlen(input) - 1] = '\0';

        if (strcmp(input, "exit") == 0) {
            break;
        }   

        process_command(input);
    }

    return 0;
}



void exec_tee_command(char* command) {
    char *output_filename = strrchr(command, ' ');
    output_filename = output_filename + 1;
    char *first_command = strtok(command, " ");

    if (!is_valid(first_command)) {
        printf("'%s | %s' is not recognized as a valid command.\n\n", first_command, output_filename);
        return;
    }


    int link[2];
    char foo[4096];
    pipe(link);

    pid_t pid = fork();
    if(pid == 0) {
        dup2 (link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        execl("/usr/bin/java", "java", first_command, NULL);
    } else {
        close(link[1]);

        FILE *fp = fopen(output_filename,"w+");
        int nbytes = 0;
        char str[1000] = "";

        // while not end of file stdin, read, write to screen and write to file
        while(0 != (nbytes = read(link[0], foo, sizeof(foo)))) {
            sprintf(str, "%.*s", nbytes, foo);
            printf("%s", str);
            fputs(str,fp);
        }
        fclose(fp);
        wait(NULL);
        printf("\n");
    }
}



void process_command(char *command) {
    if (strstr(command, "|") == NULL) {
        exec_command(command);
        return;
    }

    if (strstr(command, "tee") != NULL) {
        exec_tee_command(command);
        return;
    }

    // If there is pip but no tee:
    char *second_command = strrchr(command, ' ');
    second_command = second_command + 1;
    char *first_command = strtok(command, " ");

    if (!is_valid(first_command) || !is_valid(second_command)) {
        printf("'%s | %s' is not recognized as a valid command.\n\n", first_command, second_command);
        return;
    }
    
    int pipefd[2];
    pipe(pipefd);
    pid_t pid1 = fork();
    if(pid1 == 0) {
        close(pipefd[0]);
        close(1);
        dup2(pipefd[1], 1);
        close(pipefd[1]);
        execl("/usr/bin/java", "java", first_command, NULL);
    }
    else
    {
        close(pipefd[1]);
        pid_t pid2 = fork();
        if(pid2 == 0) {
            close(0);
            dup2(pipefd[0], 0);
            close(pipefd[0]);
            execl("/usr/bin/java", "java", second_command, NULL);
        }
        else {
            close(pipefd[0]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        }
    }
    printf("\n");
    
}



void exec_command(char* command) {
    if (!is_valid(command)) {
        printf("'%s' is not recognized as a valid command.\n\n", command);
        return;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        if (strcmp(command, "clear") == 0) {
            char *const args[] = {"clear", NULL};
            execv("/usr/bin/clear", args);
        }
        
        execl("/usr/bin/java", "java", command, NULL);
    }
    else {
        wait(NULL);
        printf("\n");
    }
}



int is_valid(char *command) {
    int len = sizeof(commands)/sizeof(commands[0]);

    for(int i = 0; i < len; ++i) {
        if(!strcmp(commands[i], command)) {
            return 1;
        }
    }
    return 0;

}