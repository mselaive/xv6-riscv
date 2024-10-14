# Informe Tarea 2

## Introducción

En esta tarea se modificó el sistema operativo xv6 para implementar un planificador de procesos con prioridades dinámicas y ajuste de boost, permitiendo controlar el orden de ejecución de los procesos en función de su prioridad. Se añadieron las funciones getpriority y getboost para que los procesos pudieran consultar sus niveles de prioridad y ajustes de boost, respectivamente. Además, se creó un programa de prueba que lanza 20 procesos para observar cómo el planificador maneja el cambio en las prioridades y el boost, mostrando el comportamiento del sistema ante múltiples tareas y mejorando la comprensión de la gestión del tiempo de CPU.

## Cambios en nuestro XV6

En esta tarea implementaremos un sistema de prioridades dinámicas con ajuste de boost para la gestión de procesos en xv6. La idea principal es agregar dos nuevos atributos a la estructura de los procesos: priority y boost, los cuales permitirán controlar la ejecución de los procesos en función de su nivel de prioridad, que se incrementa o decrementa de acuerdo a un valor de ajuste (boost). El objetivo es que los procesos con mayor prioridad se ejecuten antes, mientras que el boost ayudará a regular los cambios de prioridad para evitar que un proceso permanezca en un nivel bajo de forma indefinida.

La lógica a seguir consistirá en modificar el planificador para que, durante cada iteración, se ajuste la prioridad de los procesos en función de su boost y se decida qué proceso ejecutar con base en su nivel de prioridad. Además, se implementarán dos funciones de sistema, getpriority y getboost, que permitirán a los programas de usuario consultar estos atributos para cada proceso. Finalmente, se creará un programa de prueba que genere múltiples procesos, muestre su prioridad y boost, y demuestre la funcionalidad del sistema implementado.

### Aquí está la documentación de los cambios realizados para agregar la funcionalidad de priority y boost en xv6, junto con la creación de un programa de prueba:

### 1.- Modificación de la estructura struct proc en proc.h

Se añadieron los campos priority y boost a la estructura proc para permitir que cada proceso tenga un nivel de prioridad y un valor de ajuste dinámico de la misma.

```
// En proc.h, dentro de la definición de la estructura proc:

int priority; // Nivel de prioridad del proceso

int boost;    // Valor de ajuste de la prioridad
```

### 2.- Inicialización de los campos priority y boost en allocproc() en proc.c

En la función allocproc(), que se encarga de asignar memoria para un nuevo proceso, se inicializaron los valores de priority y boost para los procesos recién creados.

```
// En proc.c, dentro de la función allocproc():
p->priority = 1; // Inicialización con prioridad 1 por defecto
p->boost = 1;    // Inicialización con un boost de 1
```

### 3.- Implementación de las funciones getpriority y getboost en proc.c

Se añadieron las funciones para obtener la prioridad y el boost de un proceso según su PID. Estas funciones recorren la lista de procesos para buscar el proceso correspondiente y devolver el valor solicitado.

```
// En proc.c:
int getpriority(int pid) {
    struct proc *p;
    for(p = proc; p < &proc[NPROC]; p++) {
        if(p->pid == pid) {
            return p->priority;
        }
    }
    return -1; // Si no se encuentra el proceso
}

int getboost(int pid) {
    struct proc *p;
    for(p = proc; p < &proc[NPROC]; p++) {
        if(p->pid == pid) {
            return p->boost;
        }
    }
    return -1; // Si no se encuentra el proceso
}
```

### 4.- Definición de las funciones de sistema sys_getpriority y sys_getboost en sysproc.c

Se añadieron las funciones de sistema que permiten a los programas de usuario invocar las funciones getpriority y getboost. Estas funciones reciben el PID del proceso y devuelven la prioridad y el boost.

```
// En sysproc.c:
uint64 sys_getpriority(void) {
    int pid;
    argint(0, &pid);
    if(pid < 0)
        return -1;
    return getpriority(pid);
}

uint64 sys_getboost(void) {
    int pid;
    argint(0, &pid);
    if(pid < 0)
        return -1;
    return getboost(pid);
}
```

### 5.- Modificación de syscall.h para agregar las nuevas funciones de sistema

Se añadieron los identificadores para SYS_getpriority y SYS_getboost.

```
// En syscall.h:
#define SYS_getpriority 22 // Ejemplo, dependiendo de la numeración de funciones
#define SYS_getboost 23
```

### 6.- Modificación de syscall.c para registrar las nuevas funciones de sistema

Se añadió el mapeo de SYS_getpriority y SYS_getboost con sus funciones correspondientes en la tabla de sistema.

```
// En syscall.c:
extern uint64 sys_getpriority(void);
extern uint64 sys_getboost(void);

static uint64 (*syscalls[])(void) = {
    ...
    [SYS_getpriority] sys_getpriority,
    [SYS_getboost] sys_getboost,
};
```

### 7.- Actualización de usys.pl para generar envoltorios de llamadas al sistema

Se añadió getpriority y getboost a usys.pl para que los programas de usuario puedan invocar las funciones.

```
# En usys.pl:
entry("getpriority");
entry("getboost");
```

### 8.- Modificación del scheduler

Se realizo modificaciones en la función scheduler en proc.c para implementar la lógica de actualización de priority y boost para los procesos en el sistema. El objetivo era permitir que los procesos ajustaran su prioridad de forma dinámica en función de sus ejecuciones previas, logrando un comportamiento de oscilación entre diferentes niveles de prioridad.

```
void
scheduler(void)
{
    struct proc *p;
    struct cpu *c = mycpu();

    c->proc = 0;
    for(;;){
        // Habilitar interrupciones para evitar deadlocks.
        intr_on();

        int found = 0;
        for(p = proc; p < &proc[NPROC]; p++) {
            acquire(&p->lock);
            if(p->state == RUNNABLE) {
                // Actualizar la prioridad y el boost de los procesos RUNNABLE.
                p->priority += p->boost;

                // Cambiar el boost según la prioridad.
                if (p->priority >= 9) {
                    p->boost = -1; // Disminuir la prioridad si alcanza 9 o más.
                }
                if (p->priority <= 0) {
                    p->boost = 1;  // Aumentar la prioridad si alcanza 0 o menos.
                }

                // Cambiar el estado del proceso a RUNNING.
                p->state = RUNNING;
                c->proc = p;
                swtch(&c->context, &p->context);

                // El proceso termina su ejecución por ahora.
                c->proc = 0;
                found = 1;
            }
            release(&p->lock);
        }

        // Si no se encontró un proceso RUNNABLE, poner el CPU en modo espera.
        if(found == 0) {
            intr_on();
            asm volatile("wfi");
        }
    }
}
```

### 9.- Creación del programa de prueba tarea_t2.c

El programa de prueba crea 20 procesos utilizando fork, donde cada proceso obtiene su priority y boost usando las nuevas funciones. Además, imprime esta información y luego espera unos segundos antes de finalizar.

```
// tarea_t2.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NUM_PROCESSES 20

void create_processes() {
    for (int i = 0; i < NUM_PROCESSES; i++) {
        int pid = fork();
        if (pid < 0) {
            printf("Error: fork failed\n");
            exit(1);
        } else if (pid == 0) {
            // Código del proceso hijo
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
```

### 9.- Inclusión del programa tarea_t2 en el archivo Makefile

Para compilar el programa, se añadió tarea_t2 a la lista de programas de usuario en el Makefile

# En Makefile:

```
UPROGS = ... $/_t2\
```

## Ahora con esto creado pasaremos a correr nuestro xv6 con QEMU, para así compilarlo

```
make
make qemu
```

![alt text](<imagespruebas/tarea 2/Captura de pantalla 2024-10-14 a la(s) 2.35.24 a. m..png>)

## Ahora correremos nuestro nuevo programa "t2"

Ahora corriendo nuestro programa escribiendo t2 en nuestra terminal conseguiremos crear los 20 procesos, donde se mencionara tanto su prioridad como su boost

```
matiaas@MacBook-Air-de-Matias xv6-riscv % make 
qemu
qemu-system-riscv64 -machine virt -bios none -kernel kernel/kernel -m 128M -smp 3 -nographic -global virtio-mmio.force-legacy=false -drive file=fs.img,if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

xv6 kernel is booting

hart 1 starting
hart 2 starting
init: starting sh
$ t2
Ejecutando proceso con PID: 4, Prioridad: 1, Boost: 1
Ejecutando proceso con PID: 5, Prioridad: 2, Boost: 1
Ejecutando proceso con PID: 6, Prioridad: 3, Boost: 1
Ejecutando proceso con PID: 7, Prioridad: 4, Boost: 1
Ejecutando proceso con PID: 8, Prioridad: 5, Boost: 1
Ejecutando proceso con PID: 9, Prioridad: 6, Boost: 1
Ejecutando proceso con PID: 10, Prioridad: 7, Boost: 1
Ejecutando proceso con PID: 11, Prioridad: 8, Boost: 1
Ejecutando proceso con PID: 12, Prioridad: 9, Boost: -1
Ejecutando proceso con PID: 13, Prioridad: 8, Boost: -1
Ejecutando proceso con PID: 14, Prioridad: 7, Boost: -1
Ejecutando proceso con PID: 15, Prioridad: 6, Boost: -1
Ejecutando proceso con PID: 16, Prioridad: 5, Boost: -1
Ejecutando proceso con PID: 17, Prioridad: 4, Boost: -1
Ejecutando proceso con PID: 18, Prioridad: 3, Boost: -1
Ejecutando proceso con PID: 19, Prioridad: 2, Boost: -1
Ejecutando proceso con PID: 20, Prioridad: 1, Boost: -1
Ejecutando proceso con PID: 21, Prioridad: 0, Boost: 1
Ejecutando proceso con PID: 22, Prioridad: 1, Boost: 1
Ejecutando proceso con PID: 23, Prioridad: 2, Boost: 1
$ 
```

## Explicación

Los resultados muestran la ejecución de 20 procesos hijos, cada uno indicando su PID, prioridad y boost. Los PIDs incrementan secuencialmente de 4 a 23, reflejando el orden de creación de los procesos. Las prioridades empiezan en 1 y aumentan hasta 9 con un boost de 1, momento en el cual el boost cambia a -1, haciendo que las prioridades disminuyan gradualmente hasta 0. Al llegar a la prioridad 0, el boost vuelve a ser 1, lo que permite que las prioridades aumenten nuevamente, mostrando el comportamiento dinámico del sistema de ajuste de prioridades y boosts implementado en el scheduler.