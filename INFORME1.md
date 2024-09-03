# Informe Tarea 1

## Parte 1

En x86, un proceso (en espacio de usuario) realiza una llamada al sistema configurando parámetros en los registros y activando una interrupción de software utilizando la instrucción int 0x80. El kernel, en respuesta, ejecuta la operación solicitada y devuelve el resultado al proceso. Debemos implementar ambos lados: el espacio de usuario y el espacio de kernel.

### En el espacio usuario

En el archivo user.h se encuentran las definiciones de todas las funciones de llamadas al sistema. Para añadir una nueva llamada, agregamos una definición:

```
int getppid(void);
```

Luego, es necesario incluir el código real de esta función. En usys.S, tenemos un archivo en ensamblador que genera el código de ensamblado para todas las llamadas al sistema. Esto se realiza utilizando macros para generar el mismo código repetitivo. Solo necesitamos añadir una nueva entrada en usys.S

```
.global getppid
getppid:
 li a7, SYS_getppid
 ecall
 ret
```

### En el espacio Kernel

En el archivo syscall.h se define qué número corresponde a cada llamada al sistema. Añadimos una nueva entrada:

```
#define SYS_getppid 22
```

En syscall.c, encontramos el mecanismo que despacha las llamadas al sistema. El arreglo de punteros a funciones, syscalls, asocia los números de las llamadas al sistema con las funciones correspondientes. Aquí necesitamos agregar dos entradas:

```
extern int sys_getppid(void);


syscalss
[SYS_getppid] sys_getppid,
```

Finalmente, solo nos queda agregar la función sys_getppid(void).

En sysproc.c se encuentran las llamadas al sistema relacionadas con los procesos. Aquí agregaremos una nueva función para nuestra llamada al sistema. La función myproc(void) devuelve una copia de la estructura del proceso actual que se está ejecutando. Además, como ya vimos, esta estructura contiene un puntero a la estructura del proceso padre. De esta manera, podemos retornar el PID del proceso padre.

```
int
sys_getppid(void) {
  return myproc()->parent->pid;
}
```

### Programa c para probar nuestra función

Ahora nuevamente en la carpeta user crearemos nuestra funcion yosoytupadre -> en mi caso lo deje en getppidtest para poder tener más orden en mi programa

```
#include "types.h"

#include "stat.h"

#include "user.h"

#include "fs.h"

int

main(int argc, char *argv[])

{

  int pid  = getpid();

  int ppid = getppid();

  printf("PID: %d\n", pid);

  printf("Parent PID: %d\n", ppid);

  exit(0);

}
```

Este código en C muestra el PID del proceso actual y el PID del proceso padre utilizando las llamadas al sistema getpid() y getppid(). Se imprimen ambos valores en la terminal, y luego el programa finaliza con exit(0).

Agregar este archivo a MakeFIile para que este pueda ser compilado en el inicio del sistempa operativo

```
	$U/_getppidtest\
```

Aquí dejamos como queda en la consola:

```
xv6 kernel is booting

hart 2 starting
hart 1 starting
init: starting sh
$ getppidtest
PID: 3
Parent PID: 2
$ getppidtest
PID: 4
Parent PID: 2
$ 
```

## Parte 2

Para la segunda parte del enunciado volveremos a repetir los mismos pasos, por lo que se explicara menos esta vez

Definir la nueva llamada al sistema: En syscall.h, asigna un número para la llamada al sistema getancestor:

```
#define SYS_getancestor 23
```

Agregar la función en el kernel: En sysproc.c, implementa la función del sistema que recorre la jerarquía de procesos para encontrar el ancestro correspondiente:

```
// kernel/sysproc.c

uint64

sys_getancestor(int n)

{

  argint(0, &n);

  if (n == 0) {

    return myproc()->pid;

  } else if (n == 1) {

    if (myproc()->parent == NULL) {

      return -1;

    }

    return myproc()->parent->pid;

  } else if (n == 2) {

    if (myproc()->parent == NULL || myproc()->parent->parent == NULL) {

      return -1;

    }

    return myproc()->parent->parent->pid;

  } else {

    return -1;

  }
```

Declarar la función en syscall.c: Declara la función sys_getancestor y añádela a la tabla de llamadas al sistema:

```
extern uint64 sys_getancestor(void);
[SYS_getancestor] sys_getancestor,
```

Crear el enlace con espacio de usuario: En usys.S, añade la entrada para la nueva llamada al sistema:

```
.global getancestor
getancestor:
 li a7, SYS_getancestor
 ecall
 ret
```

Definir el prototipo en user.h: Añade la definición de la función en user.h:

```
int getancestor(int);
```

Implementar el programa de usuario: Escribe el programa en C para probar la nueva llamada al sistema:

```
#include "types.h"

#include "user.h"

int main(int argc, char *argv[]) {

    if (argc != 2) {

        printf("Esta usando: getancestor <n>\n");

        exit(1);

    }

    int n = atoi(argv[1]);

    int result = getancestor(n);

    if (result == -1) {

        printf("No se encontro anestro para %d\n", n);

    } else {

        printf("Ancestro PID para el proceso %d: %d\n", n, result);

    }

    exit(0);

}
```

Por ultimo cambiar el archivo MakeFile y agregar este archivo nuevo creado para que pueda ser compilado:

```
	$U/_getancestor\
```

Ahora probaremos esta nueva función en nuestro xv6:

```
xv6 kernel is booting

hart 2 starting
hart 1 starting
init: starting sh
$ getancestor 0
Ancestro PID para el proceso 0: 3
$ get ancestor 0
exec get failed
$ getancestor 0
Ancestro PID para el proceso 0: 5
$ getancestor 1
Ancestro PID para el proceso 1: 2
$ getancestor 2
Ancestro PID para el proceso 2: 1
$ getancestor 0
Ancestro PID para el proceso 0: 8
$ 
```

En la carpeta de "imagenespruebas" podemos encontrar estas imagenes de 

## Explicación del ejercicio en las 2 partes.

### getppid: 

La llamada al sistema getppid en xv6 devuelve el PID del proceso padre del proceso actual. Se implementa accediendo al puntero del proceso padre dentro de la estructura del proceso actual (myproc()). La función simplemente retorna el PID del proceso padre, o -1 si no existe un padre, proporcionando una forma directa de obtener la relación de parentesco entre procesos.

### getancestor: 

La llamada al sistema getancestor permite obtener el PID de un ancestro del proceso actual en función del nivel indicado (0 para el proceso mismo, 1 para el padre, 2 para el abuelo). La función recorre la cadena de procesos padres hasta el ancestro solicitado, devolviendo su PID o -1 si el ancestro no existe, extendiendo la funcionalidad de getppid a múltiples generaciones de procesos.

## Complicaciones del ejercicio

En este ejercicio hecho tuve varios problemas con el uso de una función llamad argint para preguntar este argumento que nos pedia nuestro getancestor, no podía entender este problema y al intentar buscar una solución se me daba que esta función tendría que ser modificada como esta programada en el xv6.

Luego de indagar en algunos problemas parecidos en StackOverflow entendía que no es un problema de la función solo que esta no podia ser ejecutada dentro de un if, por lo que despues de muchas vueltas se logro ejecutar simplemente sacando el argint de la función principal y agregando un & en el argumento.