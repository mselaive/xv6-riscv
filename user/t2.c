#include "kernel/types.h"
#include "user/user.h"

#define NUM_PROCESSES 20

void create_processes() {
    for (int i = 0; i < NUM_PROCESSES; i++) {
        int pid = fork();
        if (pid < 0) {
            printf("Error: fork failed\n");
            exit(1);
        } else if (pid == 0) {
            sleep(i);
            int priority = getpriority(getpid());
            int boost = getboost(getpid());

            printf("Ejecutando proceso con PID: %d, Prioridad: %d, Boost: %d\n", getpid(), priority, boost);
            
            sleep(10);
            exit(0);
        }
    }

    // Esperar a que todos los procesos hijos terminen
    for (int i = 0; i < NUM_PROCESSES; i++) {
        wait(0);
    }
}

int main(int argc, char *argv[]) {
    create_processes();
    
    exit(0);
}
