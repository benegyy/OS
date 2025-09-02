#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstring>
#include "parser.h"
#include <vector>
#include <signal.h>
#include <errno.h>
#define PROMPT "/>"
#define INPUT_BUFFER 1024*200
void execute_subshell(parsed_input *input);
void execute_sequential(parsed_input *input);
void execute_parallel(parsed_input *input);
void execute_subshell(char *subshell_input);
void execute_pipeline(parsed_input *input);
void execute_pipeline(pipeline *pipex);

void sigpipe_handler(int signum) {
    // Do nothing
}

void repeater(int num_pipes, int pipes[][2]) {
    // Close unused pipe ends
    for (int i = 0; i < num_pipes; i++) {
        close(pipes[i][1]); // Close write end
    }

    char buffer[INPUT_BUFFER];
    ssize_t bytes_read;

    // Read from stdin and write to pipes
    while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
        // Write to pipes
        for (int i = 0; i < num_pipes; i++) {
            ssize_t bytes_written = write(pipes[i][1], buffer, bytes_read);
            if (bytes_written != bytes_read) {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }
    }

    // Check if read encountered an error
    if (bytes_read == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    // Close remaining open pipe ends
    for (int i = 0; i < num_pipes; i++) {
        close(pipes[i][1]); // Close write end
    }
}



void execute_command(command *cmd) {
        execvp(cmd->args[0], cmd->args);
        perror("execvp");
        exit(EXIT_FAILURE);
}

void execute_sequential(parsed_input *input) {
    int i = 0;
    pid_t pid;
    while (i < input->num_inputs) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { 
            if (input->separator == SEPARATOR_SEQ) {
                if (input->inputs[i].type == INPUT_TYPE_COMMAND) {
                    execute_command(&input->inputs[i].data.cmd);
                } else if (input->inputs[i].type == INPUT_TYPE_PIPELINE) {
                    execute_pipeline(&input->inputs[i].data.pline);
                }
                exit(EXIT_SUCCESS);
            }
        }
                 
         i++;
        while (wait(NULL) > 0);
    }
}


void execute_parallel(parsed_input *input) {
    int i = 0;
    pid_t pid;
    while (i < input->num_inputs) {
        // Create a child process for each command or pipeline
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { // Child process
            // Execute the command or pipeline
            if (input->inputs[i].type == INPUT_TYPE_COMMAND) {
                execute_command(&input->inputs[i].data.cmd);
            } else if (input->inputs[i].type == INPUT_TYPE_PIPELINE) {
                execute_pipeline(&input->inputs[i].data.pline);
            }
            exit(EXIT_SUCCESS);
        }
        i++;

    }
    while (wait(NULL) > 0);
}

void execute_parallel_with_input(parsed_input *input, FILE *stdin_data) {
    int i = 0;
    pid_t pid;
    while (i < input->num_inputs) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { // Child process
            // Redirect stdin if needed
            if (stdin_data != NULL) {
                dup2(fileno(stdin_data), STDIN_FILENO);
                fclose(stdin_data);
            }

            // Execute the command or pipeline
            if (input->inputs[i].type == INPUT_TYPE_COMMAND) {
                execute_command(&input->inputs[i].data.cmd);
            } else if (input->inputs[i].type == INPUT_TYPE_PIPELINE) {
                execute_pipeline(&input->inputs[i].data.pline);
            }
            exit(EXIT_SUCCESS);
        }
        i++;
    }
}
void print_pipes(int pipes[][2], int num_inputs) {
    char buffer[INPUT_BUFFER];
    ssize_t bytes_received;

    printf("Contents of pipes:\n");
    for (int i = 0; i < num_inputs; i++) {
        printf("Pipe %d:\n", i);
        while ((bytes_received = read(pipes[i][0], buffer, INPUT_BUFFER)) > 0) {
            write(STDOUT_FILENO, buffer, bytes_received);
        }
        printf("\n");
    }
}
void read_stdin_to_multiple_pipes(int num_pipes, int pipes[][2]) {
    char buffer[INPUT_BUFFER];
    ssize_t bytes_read;

    // Read from stdin until EOF
    while ((bytes_read = read(STDIN_FILENO, buffer, INPUT_BUFFER)) > 0) {
        // Write to each pipe
        for (int i = 0; i < num_pipes; i++) {
            ssize_t bytes_written = 0;
            // Write to the pipe until all bytes are written
            while (bytes_written < bytes_read) {
                ssize_t written = write(pipes[i][1], buffer + bytes_written, bytes_read - bytes_written);
                if (written == -1) {
                    perror("write");
                    exit(EXIT_FAILURE);
                }
                bytes_written += written;
            }
        }
    }

    if (bytes_read == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    // Close the write ends of all pipes
    for (int i = 0; i < num_pipes; i++) {
        if (close(pipes[i][1]) == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }
    }
}

void execute_parallel2(parsed_input *input) {
    int num_inputs = input->num_inputs;
    pid_t pid;
    int inputpipe[2];
    int pipes[num_inputs][2];
    char buffer[INPUT_BUFFER];
    ssize_t bytes_read;
    ssize_t total_bytes_read = 0;
    for (int i = 0; i < num_inputs; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }
    pipe(inputpipe);
    /* while ((bytes_read = read(STDIN_FILENO, buffer, INPUT_BUFFER)) > 0) {
         for (int i = 0; i < num_inputs; i++) {
             if (write(pipes[i][1], buffer, bytes_read) != bytes_read) {
                 perror("write");
                 exit(EXIT_FAILURE);
             }

         }
     }*/
    read_stdin_to_multiple_pipes(num_inputs, pipes);


    //print_pipes(pipes, num_inputs);

    for (int i = 0; i < num_inputs; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { // Child process
            // Read from pipe and execute the command
            // Close unused read ends of pipes
            if (dup2(pipes[i][0], STDIN_FILENO) == -1) {
                perror("dup2");
                exit(EXIT_FAILURE);
            }

            // Execute the command or pipeline
            if (input->inputs[i].type == INPUT_TYPE_COMMAND) {
                execute_command(&input->inputs[i].data.cmd);
            } else if (input->inputs[i].type == INPUT_TYPE_PIPELINE) {
                execute_pipeline(&input->inputs[i].data.pline);
            }
        }
    }

    for (int i = 0; i < num_inputs; i++) {
        close(pipes[i][0]); // Close read end
    }



    // Close read ends of all pipes in the parent process
    for (int i = 0; i < num_inputs; i++) {
        close(pipes[i][1]); // Close write end
    }


    // Wait for all child processes to finish
    while (wait(NULL) > 0);
    exit(EXIT_SUCCESS);
}



/*
void execute_subshell_parallel(char *subshell_input,char* stdinn) {
    parsed_input sub_input;
    if (parse_line(subshell_input, &sub_input)) {
        switch (sub_input.separator) {
            case SEPARATOR_PARA:
                execute_parallel2(&sub_input);
                break;
            default:
                fprintf(stderr, "Invalid separator type in subshell\n");
                break;
        }
        free_parsed_input(&sub_input);
    } else {
        fprintf(stderr, "Invalid input in subshell\n");
    }

    exit(EXIT_SUCCESS);
*/



void execute_pipeline(parsed_input *input) {
    int num_commands = input->num_inputs;
    int pipes[num_commands - 1][2];
    pid_t pid;
    int child_count = num_commands;

    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }


    for (int i = 0; i < num_commands; i++) {
        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { // Child process
            if (i > 0) {
                if (dup2(pipes[i - 1][0], STDIN_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            if (i < num_commands - 1) {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            for (int j = 0; j < num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            if (input->inputs[i].type == INPUT_TYPE_SUBSHELL) {

                execute_subshell(input->inputs[i].data.subshell);

                }

            else if (input->inputs[i].type == INPUT_TYPE_COMMAND) {
                execvp(input->inputs[i].data.cmd.args[0], input->inputs[i].data.cmd.args);
                perror("execvp");
                exit(EXIT_FAILURE);
            }
            else {
                fprintf(stderr, "Invalid input type\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    for (int i = 0; i < num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    while (child_count > 0) {
        int status;
        pid_t terminated_pid = wait(&status);
        if (terminated_pid == -1) {
            perror("wait");
            exit(EXIT_FAILURE);
        } else {
            child_count--;
        }
    }
}





void execute_pipeline(pipeline *pipex) {
    int num_commands = pipex->num_commands;
    pid_t pid;
    int i;
    int child_count = num_commands; // Initialize child count
    int pipe_fds[num_commands - 1][2];

    // Create pipes
    for (i = 0; i < num_commands - 1; i++) {
        if (pipe(pipe_fds[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    // Execute commands
    for (i = 0; i < num_commands; i++) {
        pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { // Child process
            if (i > 0) {
                if (dup2(pipe_fds[i - 1][0], STDIN_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            if (i < num_commands - 1) {
                if (dup2(pipe_fds[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }
            for (int j = 0; j < num_commands - 1; j++) {
                close(pipe_fds[j][0]);
                close(pipe_fds[j][1]);
            }
            // Execute command
            execvp(pipex->commands[i].args[0], pipex->commands[i].args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }

    }

// Close all pipe file descriptors
    for (int j = 0; j < num_commands - 1; j++) {
        close(pipe_fds[j][0]);
        close(pipe_fds[j][1]);
    }
    for (int i = 0; i < num_commands; i++) {
        int status;
        if (wait(&status) == -1) {
            //printf("c1\n");
            perror("wait");

            exit(EXIT_FAILURE);
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Command execution failed1\n");
            exit(EXIT_FAILURE);
        }
    }
}

void execute_subshell_1(char *subshell_input) {
    pid_t pid = fork();
    if (pid == 0) {
        parsed_input sub_input;
        if (parse_line(subshell_input, &sub_input)) {
            switch (sub_input.separator) {
                case SEPARATOR_PIPE:
                    execute_pipeline(&sub_input);
                    break;
                case SEPARATOR_SEQ:
                    execute_sequential(&sub_input);
                    break;
                case SEPARATOR_PARA:
                    execute_parallel(&sub_input);
                    break;
                case SEPARATOR_NONE:
                    execute_command(&sub_input.inputs[0].data.cmd);
                    break;
                default:
                    fprintf(stderr, "Invalid separator type in subshell\n");
                    break;
            }
            free_parsed_input(&sub_input);
        } else {
            fprintf(stderr, "Invalid input in subshell\n");
        }
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);
    } else {
        perror("fork");
    }

}

void execute_subshell_one(parsed_input *input) {
    int num_subshells = input->num_inputs;
    if (num_subshells > 1) {
        // If there are multiple subshells, execute them sequentially
        for (int i = 0; i < num_subshells; i++) {
            execute_subshell(input->inputs[i].data.subshell);
        }
    } else {
        // If there's only one subshell, execute it directly
        execute_subshell_1(input->inputs[0].data.subshell);
    }
}



void execute_subshell(char *subshell_input) {
        parsed_input sub_input;
        if (parse_line(subshell_input, &sub_input)) {
            switch (sub_input.separator) {
                case SEPARATOR_PIPE:
                    execute_pipeline(&sub_input);
                    break;
                case SEPARATOR_SEQ:
                    execute_sequential(&sub_input);
                    break;
                case SEPARATOR_PARA:
                    execute_parallel2(&sub_input);
                    break;
                case SEPARATOR_NONE:
                    execute_command(&sub_input.inputs[0].data.cmd);
                    break;
                default:

                    fprintf(stderr, "Invalid separator type in subshell\n");
                    break;
            }

            free_parsed_input(&sub_input);
        } else {
            fprintf(stderr, "Invalid input in subshell\n");
        }

    exit(EXIT_SUCCESS);

}

void reap_children() {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}
int main() {
                        char input_line[INPUT_BUFFER_SIZE];
                        parsed_input input;

                        while (1) {
                            if (signal(SIGPIPE, sigpipe_handler) == SIG_ERR) {
                                perror("signal");
                                exit(EXIT_FAILURE);
                            }
                            printf(PROMPT);
                            fflush(stdin);
                            fflush(stdout);

                            if (fgets(input_line, sizeof(input_line), stdin) == NULL) {
                                break;
                            }
                            input_line[strcspn(input_line, "\n")] = '\0';


                            if (strcmp(input_line, "quit") == 0) {
                                //printf("Exiting eshell.\n");
                                break;
                            }


                            if (parse_line(input_line, &input)) {
                                //pretty_print(&input);
                                reap_children();
                                switch (input.separator){
                                    case SEPARATOR_PIPE:
                                        //     printf("Executing pipe\n");
                                        //      printf("number of command is %d \n", input.num_inputs);
                                        execute_pipeline(&input);
                                        break;
                                    case SEPARATOR_SEQ:
                                        //       printf("Executing seq\n");
                                        execute_sequential(&input);
                                        break;
                                    case SEPARATOR_PARA:
                                        //      printf("Executing para\n");
                                        execute_parallel(&input);
                                        break;
                                    case SEPARATOR_NONE:
                                        // printf("4  \n");
                                        if (input.inputs[0].type == INPUT_TYPE_SUBSHELL) {
                                            execute_subshell_one(&input);
                                        }
                                        else if(input.num_inputs > 0 && input.inputs[0].type == INPUT_TYPE_COMMAND){
                                            execute_command(&input.inputs[0].data.cmd);
                                        }
                                        else {
                        fprintf(stderr, "Invalid input\n");
                    }
                    break;
                default:
                    fprintf(stderr, "Invalid separator type\n");
                    break;
            }
            free_parsed_input(&input);
        } else {
            printf("Invalid input\n");
        }

    }

    return 0;
}
