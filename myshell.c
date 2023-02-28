/**
 * CS2106 AY22/23 Semester 2 - Lab 2
 *
 * This file contains function definitions. Your implementation should go in
 * this file.
 */

#include "myshell.h"

#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define EXITED 1
#define RUNNING 2
#define TERMINATING 3
#define STOPPED 4

struct PCBTable pcb_table[MAX_PROCESSES];
int pcb_table_count;

static bool ends_with_ampersand(char** args, int num_args) {
    return num_args > 0 && args[num_args - 1] && strcmp(args[num_args - 1], "&") == 0;
}

static bool contains_output_redirect(char** args, int num_args) {
    
    
    
    for (int i = 0; i < num_args; i++) {
        if (args[i] && strcmp(args[i], ">") == 0) {
            return true;
        }
    }
    return false;
}

static bool contains_error_redirect(char** args, int num_args) {
    for (int i = 0; i < num_args; i++) {
        if (args[i] && strcmp(args[i], "2>") == 0) {
            return true;
        }
    }
    return false;
}

static bool contains_input_redirect(char** args, int num_args) {
    for (int i = 0; i < num_args; i++) {
        if (args[i] && strcmp(args[i], "<") == 0) {
            return true;
        }
    }
    return false;
}

static int get_index_of_token(char** args, int num_args, char* token) {
    // returns index of token in args
    for (int i = 0; i < num_args; i++) {
        if (args[i] && strcmp(args[i], token) == 0) {
            return i;
        }
    }
    return -1;
}

static void add_new_proc(pid_t pid) {
    pcb_table[pcb_table_count].pid = pid;
    pcb_table[pcb_table_count].status = RUNNING;
    pcb_table[pcb_table_count].exitCode = -1;
    pcb_table_count++;
}

/*******************************************************************************
 * Signal handler : ex4
 ******************************************************************************/

// static void signal_handler(int signo) {

// Use the signo to identy ctrl-Z or ctrl-C and print “[PID] stopped or print “[PID] interrupted accordingly.
// Update the status of the process in the PCB table

// }

/// TODO when piping file output, rememeber to make created file readable globally

static void proc_update_status(pid_t pid, int status, int exitCode) {
    /******* FILL IN THE CODE *******/

    // Call everytime you need to update status and exit code of a process in PCBTable

    // May use WIFEXITED, WEXITSTATUS, WIFSIGNALED, WTERMSIG, WIFSTOPPED

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (pcb_table[i].pid == pid) {
            pcb_table[i].status = status;
            pcb_table[i].exitCode = exitCode;
            return;
        }
    }
}

static void handle_child_process_exited_or_stopped() {
    pid_t child_pid;
    int w_status;
    child_pid = wait(&w_status);
    // Child exited under control
    if (WIFEXITED(w_status)) {
        proc_update_status(child_pid, EXITED, WEXITSTATUS(w_status));
    }
    // Child did not exit normally
    if (WIFSIGNALED(w_status)) {
        proc_update_status(child_pid, EXITED, WTERMSIG(w_status));
    }
}

/*******************************************************************************
 * Built-in Commands
 ******************************************************************************/

static void command_info(char** command, int num_tokens) {
    /******* FILL IN THE CODE *******/
    // If option is 0
    // Print details of all processes in the order in which they were run. You will need to print their process IDs, their current status (Exited, Running, Terminating, Stopped)
    // For Exited processes, print their exit codes.
    // If option is 1
    // Print the number of exited process.
    // If option is 2
    // Print the number of running process.
    // If option is 3
    // Print the number of terminating process.
    // If option is 4
    // Print the number of stopped process.

    // For all other cases print “Wrong command” to stderr.
    if (num_tokens != 2) {
        fprintf(stderr, "Wrong command\n");
        return;
    }

    int option = atoi(command[1]);

    if (option == 0) {
        for (int i = 0; i < pcb_table_count; i++) {
            switch (pcb_table[i].status) {
                case EXITED:
                    printf("[%d] Exited %d", pcb_table[i].pid, pcb_table[i].exitCode);
                    break;
                case RUNNING:
                    printf("[%d] Running", pcb_table[i].pid);
                    break;
                case TERMINATING:
                    printf("[%d] Terminating", pcb_table[i].pid);
                    break;
                case STOPPED:
                    printf("[%d] Stopped", pcb_table[i].pid);
                    break;
                default:
                    break;
            }
            printf("\n");
        }
    } else if (option == 1) {
        int count = 0;
        for (int i = 0; i < pcb_table_count; i++) {
            if (pcb_table[i].status == EXITED) {
                count++;
            }
        }
        printf("Total exited processes: %d", count);
        printf("\n");
    } else if (option == 2) {
        int count = 0;
        for (int i = 0; i < pcb_table_count; i++) {
            if (pcb_table[i].status == RUNNING) {
                count++;
            }
        }
        printf("Total running processes: %d", count);
        printf("\n");
    } else if (option == 3) {
        int count = 0;
        for (int i = 0; i < pcb_table_count; i++) {
            if (pcb_table[i].status == TERMINATING) {
                count++;
            }
        }
        printf("Total terminating processes: %d", count);
        printf("\n");
    } else if (option == 4) {
        int count = 0;
        for (int i = 0; i < pcb_table_count; i++) {
            if (pcb_table[i].status == STOPPED) {
                count++;
            }
        }
        printf("Total stopped processes: %d", count);
        printf("\n");
    } else {
        fprintf(stderr, "Wrong command\n");
    }
}

static void command_wait(char** command, int num_tokens) {
    /******* FILL IN THE CODE *******/
    if (num_tokens != 2) {
        fprintf(stderr, "Wrong command\n");
        return;
    }

    // Find the {PID} in the PCBTable
    // If the process indicated by the process id is RUNNING, wait for it (can use waitpid()).
    // After the process terminate, update status and exit code (call proc_update_status())
    // Else, continue accepting user commands.

    pid_t pid_to_wait = atoi(command[1]);
    for (int i = 0; i < pcb_table_count; i++) {
        if (pcb_table[i].pid == pid_to_wait) {
            if (pcb_table[i].status == RUNNING) {
                int w_status;
                if (waitpid(pid_to_wait, &w_status, 0) > 0 && WIFEXITED(w_status)) {
                    int exit_code = WEXITSTATUS(w_status);
                    proc_update_status(pid_to_wait, EXITED, exit_code);
                }
            }
            break;
        }
    }
}

static void command_terminate(char** command, int num_tokens) {
    /******* FILL IN THE CODE *******/
    if (num_tokens != 2) {
        fprintf(stderr, "Wrong command\n");
        return;
    }

    // Find the pid in the PCBTable
    // If {PID} is RUNNING:
    // Terminate it by using kill() to send SIGTERM
    // The state of {PID} should be “Terminating” until {PID} exits
    pid_t pid_to_terminate = atoi(command[1]);
    for (int i = 0; i < pcb_table_count; i++) {
        if (pcb_table[i].pid == pid_to_terminate) {
            if (pcb_table[i].status == RUNNING) {
                // If kill returns 0, it means the signal was sent successfully
                kill(pid_to_terminate, SIGTERM);
                proc_update_status(pid_to_terminate, TERMINATING, -1);
            }
            break;
        }
    }
}

static void command_fg(/* pass necessary parameters*/) {
    /******* FILL IN THE CODE *******/

    printf("fg called\n");
    // if the {PID} status is stopped
    // Print “[PID] resumed”
    // Use kill() to send SIGCONT to {PID} to get it continue and wait for it
    // After the process terminate, update status and exit code (call proc_update_status())
}

/*******************************************************************************
 * Program Execution
 ******************************************************************************/

static void command_exec(char* program, char** command, int num_tokens) {
    /******* FILL IN THE CODE *******/

    // check if program exists and is executable : use access()
    if (access(program, F_OK) != 0 && access(program, X_OK) != 0) {
        fprintf(stderr, "%s not found \n", program);
        return;
    }

    // fork a subprocess and execute the program

    pid_t pid;
    if ((pid = fork()) == 0) {
        // CHILD PROCESS

        // check file redirection operation is present : ex3

        // if < or > or 2> present:
        // use fopen/open file to open the file for reading/writing with  permission O_RDONLY, O_WRONLY, O_CREAT, O_TRUNC, O_SYNC and 0644
        // use dup2 to redirect the stdin, stdout and stderr to the files
        // call execv() to execute the command in the child process

        if (contains_input_redirect(command, num_tokens)) {
            int index = get_index_of_token(command, num_tokens, "<");
            command[index] = NULL;

            if (index == -1 || index == num_tokens - 1) {
                fprintf(stderr, "Wrong command");
                return;
            }
            int input_file = open(command[index + 1], O_RDONLY, O_SYNC);

            if (input_file == -1) {
                fprintf(stderr, "%s does not exist\n", command[index + 1]);
                return;
            }
            dup2(input_file, STDIN_FILENO);
            close(input_file);
        }

        if (contains_output_redirect(command, num_tokens)) {
            int index = get_index_of_token(command, num_tokens, ">");
            command[index] = NULL;

            if (index == -1 || index == num_tokens - 1) {
                fprintf(stderr, "Wrong command");
                return;
            }

            int output_file = open(command[index + 1], O_WRONLY | O_CREAT | O_TRUNC | O_SYNC);

            dup2(output_file, STDOUT_FILENO);
            close(output_file);

        } 

        if (contains_error_redirect(command, num_tokens)){

            int index = get_index_of_token(command, num_tokens, "2>");
            command[index] = NULL;

            if (index == -1 || index == num_tokens - 1) {
                fprintf(stderr, "Wrong command");
                return;
            }

            int error_file = open(command[index + 1], O_WRONLY | O_CREAT | O_TRUNC | O_SYNC);
            dup2(error_file, STDERR_FILENO);
            close(error_file);
        }

        // else : ex1, ex2
        // call execv() to execute the command in the child process
        if (ends_with_ampersand(command, num_tokens)) {
            command[num_tokens - 1] = NULL;
        } else {
            char** new_command = (char**)realloc(command, sizeof(command) + sizeof(NULL));
            command[num_tokens] = NULL;
            command = new_command;
        }

        execv(program, command);

        // Exit the child

    } else {
        // PARENT PROCESS
        // register the process in process table
        add_new_proc(pid);

        // If  child process need to execute in the background  (if & is present at the end )
        // print Child [PID] in background
        if (ends_with_ampersand(command, num_tokens)) {
            printf("Child [%d] in background\n", pid);
            waitpid(pid, NULL, WNOHANG);
        } else {
            // else wait for the child process to exit
            int exit_status;
            waitpid(pid, &exit_status, WUNTRACED);

            proc_update_status(pid, EXITED, WEXITSTATUS(exit_status));
        }

        // Use waitpid() with WNOHANG when not blocking during wait and  waitpid() with WUNTRACED when parent needs to block due to wait
    }
}

/*******************************************************************************
 * Command Processor
 ******************************************************************************/

static void command(char** command, int num_tokens) {
    /******* FILL IN THE CODE *******/

    char* program = command[0];
    // if command is "info" call command_info()             : ex1
    if (strcmp(program, "info") == 0) {
        command_info(command, num_tokens);
        return;
    }
    // if command is "wait" call command_wait()             : ex2
    if (strcmp(program, "wait") == 0) {
        command_wait(command, num_tokens);
        return;
    }
    // if command is "terminate" call command_terminate()   : ex2
    if (strcmp(program, "terminate") == 0) {
        command_terminate(command, num_tokens);
        return;
    }
    // if command is "fg" call command_fg()                 : ex4
    if (strcmp(program, "fg") == 0) {
        command_fg();
        return;
    }
    // call command_exec() for all other commands           : ex1, ex2, ex3
    command_exec(program, command, num_tokens);
}

/*******************************************************************************
 * High-level Procedure
 ******************************************************************************/

void my_init(void) {
    /******* FILL IN THE CODE *******/

    // use signal() with SIGTSTP to setup a signalhandler for ctrl+z : ex4
    // use signal() with SIGINT to setup a signalhandler for ctrl+c  : ex4

    // anything else you require
    pcb_table_count = 0;
    signal(SIGCHLD, handle_child_process_exited_or_stopped);
}

void my_process_command(size_t num_tokens, char** tokens) {
    /******* FILL IN THE CODE *******/

    // Split tokens at NULL or ; to get a single command (ex1, ex2, ex3, ex4(fg command))

    // for example :  /bin/ls ; /bin/sleep 5 ; /bin/pwd
    // split the above line as first command : /bin/ls , second command: /bin/pwd  and third command:  /bin/pwd
    // Call command() and pass each individual command as arguements
    char* delimiter = ";";
    size_t i = 0;
    while (i < num_tokens) {
        size_t starting_index = i;
        int num_tokens_in_current_command = 0;
        while (tokens[i] != NULL && strcmp(tokens[i], delimiter) != 0) {
            num_tokens_in_current_command++;
            i++;
        }
        char** current_command = malloc(sizeof(char*) * num_tokens_in_current_command);
        memcpy(current_command, &tokens[starting_index], sizeof(char*) * num_tokens_in_current_command);

        command(current_command, num_tokens_in_current_command);
        free(current_command);

        i++;
    }
}

void my_quit(void) {
    /******* FILL IN THE CODE *******/
    // Kill every process in the PCB that is either stopped or running

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (pcb_table[i].status == RUNNING) {
            kill(pcb_table[i].pid, SIGTERM);
            printf("Killing [%d]\n", pcb_table[i].pid);
        }
    }

    printf("\nGoodbye\n");
}
