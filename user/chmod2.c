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
