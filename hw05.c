//2022427833 Lath Nishtha
//The program creates a parent and child process. The parent process sends a SIGALRM signal every 2 seconds and the child process sends a SIGALRM signal every 5 seconds. 
//The parent process also handles SIGINT signal to exit the program. The child process exits after 5 iterations. The parent process also handles SIGCHLD signal to print the child process id and the exit status of the child process.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>

// Global variables for tracking elapsed time
int parent_elapsed = 0;
int child_elapsed = 0;
pid_t child_pid;

// Signal handler for parent process alarm
void handle_parent_alarm(int sig) {
    parent_elapsed += 2;
    printf("<Parent> time out: 2, elapsed time: %d seconds\n", parent_elapsed);
}

// Signal handler for child process alarm
void handle_child_alarm(int sig) {
    child_elapsed += 5;
    static int count = 1;
    printf("[Child] time out: 5, elapsed time: %d seconds(%d)\n", child_elapsed, count);
    count++;
    if (count > 5) {
        exit(5); // Child process exits after 5 iterations
    }
}

// Signal handler for SIGCHLD (child process termination)
void handle_sigchld(int sig) {
    int status;
    pid_t pid = waitpid(-1, &status, WNOHANG);
    if (pid > 0) {
        printf("Child id: %d, sent: %d\n", pid, WEXITSTATUS(status));
    }
}

// Signal handler for SIGINT (interrupt signal)
void handle_sigint(int sig) {
    char response;
    printf("SIGINT: Do you want to exit (y or Y to exit)? ");
    scanf(" %c", &response);
    if (response == 'y' || response == 'Y') {
        printf("Exiting program.\n");
        exit(0);
    }
}

int main() {
    struct sigaction sa_parent, sa_child, sa_chld, sa_int;

    // Parent process signal handler setup for SIGALRM
    sa_parent.sa_handler = handle_parent_alarm;
    sa_parent.sa_flags = 0;
    sigemptyset(&sa_parent.sa_mask);
    sigaction(SIGALRM, &sa_parent, NULL);

    // Child process signal handler setup for SIGALRM
    sa_child.sa_handler = handle_child_alarm;
    sa_child.sa_flags = 0;
    sigemptyset(&sa_child.sa_mask);

    // SIGCHLD handler setup
    sa_chld.sa_handler = handle_sigchld;
    sa_chld.sa_flags = 0;
    sigemptyset(&sa_chld.sa_mask);
    sigaction(SIGCHLD, &sa_chld, NULL);

    // SIGINT handler setup
    sa_int.sa_handler = handle_sigint;
    sa_int.sa_flags = 0;
    sigemptyset(&sa_int.sa_mask);
    sigaction(SIGINT, &sa_int, NULL);

    // Forking to create child process
    child_pid = fork();

    if (child_pid < 0) {
        // Fork failed
        perror("fork failed");
        exit(1);
    } else if (child_pid == 0) {
        // Child process code
        printf("Child process created.\n");
        sigaction(SIGALRM, &sa_child, NULL); // Set up child alarm handler

        while (1) {
            alarm(5); // Set alarm for 5 seconds
            pause(); // Wait for signal
        }
    } else {
        // Parent process code
        printf("Parent process created.\n");

        while (1) {
            alarm(2); // Set alarm for 2 seconds
            pause(); // Wait for signal
        }
    }

    return 0;
}