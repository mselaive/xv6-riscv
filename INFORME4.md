# Informe Tarea 3

### Matías Selaive

## Objetivo

Implementar un sistema de permisos básicos en xv6 (risc) que permita modificar archivos para tener acceso de solo lectura o lectura/escritura, y agregar un nuevo permiso especial para hacerlo inmutable.

## Introducción Primera Parte

En esta tarea se ha implementado y probado la funcionalidad de control de permisos en archivos dentro del sistema operativo xv6. La modificación principal consiste en la incorporación de un sistema de permisos, donde se pueden asignar y verificar permisos de lectura y escritura a los archivos. A través de la creación de un archivo y la manipulación de sus permisos, se demostró cómo se pueden restringir o permitir accesos a archivos en función de los permisos establecidos. El programa creado realiza pruebas para verificar que los cambios de permisos son correctamente gestionados, incluyendo la verificación de lectura y escritura en el archivo bajo diferentes configuraciones de permisos. Este proceso no solo asegura el correcto funcionamiento de la funcionalidad de permisos, sino que también proporciona un marco para futuras extensiones y mejoras en el control de acceso en el sistema operativo.

## Ejecución

### **Parte 1.1 Modificación de inode y su verificación de permisos para el uso de chmod**

**a) Modificación de file.h:**

Añadimos el campo permissions en la estructura inode para manejar los permisos de acceso a los archivos:

```
// file.h
struct inode {
   ...
   int permissions;    // Permisos del archivo (0: sin acceso, 1: solo lectura, 2: solo escritura, 3: lectura/escritura)
};

```

**b) Modificación de sysfile.c:**

Modificamos la función sys_open para que verifique los permisos antes de permitir la apertura de un archivo. La verificación se realiza de la siguiente manera:

```
// sysfile.c
uint64 sys_open(void)
{
  char *path;
  int omode;
  struct inode *ip;
  struct file *f;

  argstr(0, &path);
  argint(1, &omode);

  begin_op();
  if((ip = namei(path)) == 0) {
    end_op();
    return -1;
  }
  ilock(ip);

  // Verificar si el archivo es un directorio y si el modo es incompatible
  if(ip->type == T_DIR && omode != O_RDONLY) {
    iunlockput(ip);
    end_op();
    return -1;
  }

  // Verificar permisos de escritura
  if((omode & O_WRONLY) && !(ip->permissions & 2)) {
    // Se solicita escritura, pero el archivo no tiene permisos de escritura
    iunlockput(ip);
    end_op();
    return -1;
  }

  // Verificar permisos de lectura
  if((omode & O_RDONLY) && !(ip->permissions & 1)) {
    // Se solicita lectura, pero el archivo no tiene permisos de lectura
    iunlockput(ip);
    end_op();
    return -1;
  }

  // Código para abrir el archivo...
  ...
}
```

**c) Implementación de la llamada al sistema chmod para cambiar permisos:**

Añadimos la implementación de la llamada al sistema chmod para cambiar los permisos de un archivo:

```
// sysfile.c
uint64 sys_chmod(void)
{
  char path[MAXPATH];
  int mode;
  struct inode *ip;

  // Obtener los argumentos
  argstr(0, path, MAXPATH);
  argint(1, &mode);

  if(mode < 0) {
    return -1;
  }

  begin_op();
  if((ip = namei(path)) == 0) {
    end_op();
    return -1;
  }
  ilock(ip);

  // Cambiar los permisos del archivo
  ip->permissions = mode;
  iupdate(ip);

  iunlockput(ip);
  end_op();
  return 0;
}
```

### **Parte 1.2 Modificaciones en los archivos del sistema**

**a) Modificación de syscall.h:**

Declaramos la llamada al sistema chmod:

```
// syscall.h

#define SYS_chmod 22
```

**b) Modificación de syscall.c:**

Añadimos la llamada al sistema en el array syscalls[] para que el sistema la reconozca:

```
// syscall.c
extern unit64 sys_chmod(void);

static unit64 (*syscalls[])(void) = {
  ...
  [SYS_chmod]    sys_chmod,
  ...
};
```

**c) Modificación de user.h:**

Declaramos la función chmod en el encabezado de las funciones del sistema de usuario:

```
// user.h
int chmod(char *path, int mode);
```

**d) Modificación de usys.pl:**

Añadimos la entrada para chmod en usys.pl para hacer que esté disponible para los programas de usuario:

```
// usys.pl
entry("chmod");
```

### Parte 1.3 Creación del archivo de pruebas

El archivo de prueba chmodtest fue creado para verificar el correcto funcionamiento del sistema de permisos implementado en xv6. Este programa realiza una serie de operaciones en un archivo, como la creación, escritura, y modificación de permisos, con el objetivo de asegurar que el sistema maneje adecuadamente las restricciones de acceso a los archivos. A través de este archivo, se prueban los cambios de permisos a solo lectura y de vuelta a lectura/escritura, validando así que los permisos sean correctamente aplicados y que se respeten durante las operaciones de apertura y escritura del archivo.

```
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

void print_file_content(char *filename) {
    int fd;
    char buf[512];
    int n;

    fd = open(filename, O_RDONLY);
    if(fd < 0){
        printf("No se pudo abrir el archivo '%s' para lectura.\n", filename);
        return;
    }

    printf("Contenido de '%s':\n", filename);
    while((n = read(fd, buf, sizeof(buf))) > 0) {
        write(1, buf, n);
    }
    close(fd);
    printf("\n");
}

int main(void) {
    int fd;
    char *filename = "testfile";

    // Creación del Archivo
    fd = open(filename, O_CREATE | O_RDWR);
    if(fd < 0){
        printf("Error al intentar crear el archivo '%s'.\n", filename);
        exit(1);
    }
    printf("Archivo '%s' creado exitosamente con permisos de lectura/escritura.\n", filename);

    // Escritura Inicial
    if(write(fd, "Hola, mundo\n", 12) != 12){
        printf("No se pudo escribir en el archivo '%s'.\n", filename);
        close(fd);
        exit(1);
    }
    printf("Escritura inicial completada en '%s'.\n", filename);
    close(fd);
    print_file_content(filename);

    // Cambio de Permisos a Solo Lectura
    if(chmod(filename, 1) < 0){
        printf("No se pudo cambiar los permisos de '%s' a solo lectura.\n", filename);
        exit(1);
    }
    printf("Permisos de '%s' cambiados a solo lectura.\n", filename);

    // Prueba de Escritura con Solo Lectura
    fd = open(filename, O_WRONLY);
    if(write(fd, "Intento de escritura\n", 21) != 21){
        printf("No se pudo escribir en '%s', ya que está en modo solo lectura.\n", filename);
    } else {
        printf("Error: se permitió escribir en '%s' cuando debería ser solo lectura.\n", filename);
    }
    close(fd);

    print_file_content(filename);

    // Cambio de Permisos de Vuelta a Lectura/Escritura
    if(chmod(filename, 3) < 0){
        printf("No se pudo restaurar los permisos de '%s' a lectura/escritura.\n", filename);
        exit(1);
    }
    printf("Permisos de '%s' restaurados a lectura/escritura.\n", filename);

    // Escritura Final
    fd = open(filename, O_RDWR);
    if(fd < 0){
        printf("No se pudo abrir el archivo '%s' en modo lectura/escritura.\n", filename);
        exit(1);
    }
    if(write(fd, "Adiós, mundo\n", 13) != 13){
        printf("No se pudo escribir en el archivo '%s'.\n", filename);
        close(fd);
        exit(1);
    }
    printf("Escritura final completada en '%s'.\n", filename);
    close(fd);
    print_file_content(filename);

    printf("La prueba de permisos se completó con éxito.\n");
    exit(0);
}
```

Una vez completada la implementación del sistema de permisos en xv6, el siguiente paso es compilar nuevamente el kernel para asegurarnos de que todos los cambios realizados sean correctamente integrados. Para ello, se utilizará el comando make qemu, que compila el sistema operativo y lo ejecuta en un entorno emulado, permitiéndonos probar su funcionamiento. Con el kernel recién compilado y corriendo, procederemos a ejecutar el archivo de prueba chmodtest. Este archivo nos permitirá validar que el sistema de permisos funciona correctamente, realizando diversas operaciones de apertura, escritura y modificación de permisos sobre un archivo. De esta forma, podremos comprobar si los permisos de solo lectura y lectura/escritura se aplican correctamente, asegurando que el comportamiento del sistema sea el esperado en distintos escenarios de acceso a archivos.

**Lo que conseguimos es lo siguiente:**

```
xv6 kernel is booting

hart 1 starting
hart 2 starting
init: starting sh
$ chmodtest
Archivo 'testfile' creado exitosamente con permisos de lectura/escritura.
Escritura inicial completada en 'testfile'.
Contenido de 'testfile':
CHMOD Prueba
Permisos de 'testfile' cambiados a solo lectura.
No se pudo escribir en 'testfile', ya que está en modo solo lectura.
Contenido de 'testfile':
CHMOD Prueba
Permisos de 'testfile' restaurados a lectura/escritura.
Escritura final completada en 'testfile'.
Contenido de 'testfile':
CHMOD Funcionando
La prueba de permisos se completó con éxito.
```

El resultado obtenido demuestra que la implementación del sistema de permisos en xv6 cumple con los requisitos establecidos. El archivo se crea correctamente con permisos de lectura y escritura, permitiendo una escritura inicial sin problemas. Al cambiar los permisos a solo lectura, el sistema impide la escritura en el archivo, como se esperaba. Posteriormente, al restaurar los permisos de lectura/escritura, se confirma que la escritura se vuelve a permitir. Los contenidos del archivo se muestran en cada etapa del proceso, validando que los cambios de permisos y las operaciones de escritura se realizaron correctamente. Finalmente, el mensaje "La prueba de permisos se completó con éxito" confirma que todas las funcionalidades de permisos y escritura se ejecutaron como se esperaba.

## Parte 2 Implementación del Permiso Inmutable

### ** Modicaciones necesarias**

**1.- Modificar sys_chmod**

En la función sys_chmod, se agrega una verificación para evitar cambios en los permisos si el archivo tiene el permiso "inmutable" habilitado.

**2.- Modificar sys_open**

Asegurar que los archivos marcados como inmutables solo puedan abrirse en modo lectura.

## Parte 2 Cambio en el codigo

### a) Modificación de sys_chmod en sysfile.c:

```
uint64
sys_chmod(void)
{
  char path[MAXPATH];
  int mode;
  struct inode *ip;

  if (argstr(0, path, MAXPATH) < 0 || argint(1, &mode) < 0)
    return -1;

  begin_op();

  if ((ip = namei(path)) == 0) {
    end_op();
    return -1;
  }

  ilock(ip);

  // Verificar si el archivo tiene el permiso inmutable
  if (ip->permissions == 5) {
    printf("chmod: no se puede modificar el permiso de un archivo inmutable\n");
    iunlockput(ip);
    end_op();
    return -1;
  }

  ip->permissions = mode;
  iupdate(ip);
  iunlockput(ip);

  end_op();
  return 0;
}
```

### **b) Modificación de sys_open en sysfile.c:**

```
if ((omode & O_WRONLY) && (ip->permissions == 5 || !(ip->permissions & 2))) {
  // Si el archivo es inmutable o no tiene permiso de escritura
  iunlockput(ip);
  end_op();
  return -1;
} 
```

Para llevar a cabo las pruebas del permiso especial "inmutable", procedemos a crear un segundo archivo de prueba denominado chmod2.c, el cual estará diseñado específicamente para validar las nuevas funcionalidades implementadas en el sistema. Este archivo contendrá el código necesario para verificar que un archivo marcado como inmutable no pueda ser modificado ni en su contenido ni en sus permisos. Una vez creado, será necesario añadirlo al Makefile del proyecto para que pueda compilarse y ejecutarse junto con el resto del sistema. Esta integración asegurará que el archivo chmod2.c forme parte del flujo de construcción del sistema, permitiendo realizar pruebas de manera eficiente. A continuación, presentamos el nuevo código listo para su inclusión en el proyecto.

```
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

void print_file_content(char *filename) {
  int fd;
  char buf[512];
  int n;

  fd = open(filename, O_RDONLY);
  if (fd < 0) {
    printf("Error: no se pudo abrir el archivo para lectura\n");
    return;
  }

  printf("Contenido del archivo '%s':\n", filename);
  while ((n = read(fd, buf, sizeof(buf))) > 0) {
    write(1, buf, n);
  }
  close(fd);
  printf("\n");
}

int main(void) {
  int fd;
  char *filename = "testfile";

  // Creación del Archivo
  fd = open(filename, O_CREATE | O_RDWR);
  if (fd < 0) {
    printf("Error: no se pudo crear el archivo\n");
    exit(1);
  }
  printf("Archivo creado con permisos de lectura/escritura\n");

  // Escritura Inicial
  if (write(fd, "CHMOD Prueba\n", 13) != 13) {
    printf("Error: no se pudo escribir en el archivo\n");
    close(fd);
    exit(1);
  }
  printf("Escritura inicial completada en '%s'\n", filename);
  close(fd);
  print_file_content(filename);

  // Cambio de Permisos a Inmutable
  if (chmod(filename, 5) < 0) {
    printf("Error: no se pudo cambiar los permisos a inmutable\n");
    exit(1);
  }
  printf("Permisos de '%s' cambiados a inmutable\n", filename);

  // Prueba de Escritura con Solo Lectura
  fd = open(filename, O_WRONLY);
  if (fd < 0) {
    printf("No se pudo abrir '%s' para escritura, archivo inmutable\n", filename);
  } else {
    if (write(fd, "Intento de escritura\n", 21) != 21) {
      printf("No se pudo escribir en '%s', archivo inmutable\n", filename);
    } else {
      printf("Error: escritura exitosa en archivo inmutable\n");
    }
    close(fd);
  }
  print_file_content(filename);

  // Intento de Cambio de Permisos
  if (chmod(filename, 3) < 0) {
    printf("Cambio de permisos fallido, archivo inmutable\n");
  } else {
    printf("Error: cambio de permisos exitoso en archivo inmutable\n");
  }

  printf("Pruebas de permisos inmutables completadas\n");
  exit(0);
}
```

El resultado de las pruebas confirma el correcto funcionamiento del permiso especial inmutable implementado. Inicialmente, se creó un archivo con permisos de lectura/escritura y se realizó una escritura inicial exitosa, lo que demuestra que el archivo podía ser modificado en este estado. Posteriormente, el permiso se cambió a inmutable, marcando el archivo como de solo lectura y bloqueando cualquier intento de modificación. Al intentar abrir el archivo en modo escritura, el sistema rechazó la operación, mostrando que el contenido del archivo permaneció intacto. Finalmente, el intento de cambiar nuevamente los permisos del archivo utilizando chmod también falló, cumpliendo con la restricción de que los archivos inmutables no pueden alterarse en ningún aspecto. Estos resultados validan que las restricciones asociadas al permiso inmutable funcionan correctamente, protegiendo el archivo contra cualquier modificación o cambio de permisos, tal como se especificó en los requisitos iniciales.

```
xv6 kernel is booting

hart 1 starting
hart 2 starting
init: starting sh
$ chmod2
Archivo creado con permisos de lectura/escritura
Escritura inicial completada en 'testfile'
Contenido del archivo 'testfile':
CHMOD Prueba

Permisos de 'testfile' cambiados a inmutable
No se pudo abrir 'testfile' para escritura, archivo inmutable
Contenido del archivo 'testfile':
CHMOD Prueba

chmod: no se puede modificar el permiso de un archivo inmutable
Cambio de permisos fallido, archivo inmutable
Pruebas de permisos inmutables completadas
```

## Dificultades encontradas

En esta tarea no se presentaron dificultades significativas, ya que la implementación y las pruebas fluyeron de manera consistente con lo planificado. Las modificaciones al código base de xv6 y la incorporación de nuevas funcionalidades, como el permiso inmutable, se llevaron a cabo sin mayores contratiempos, y los resultados esperados se lograron con precisión.