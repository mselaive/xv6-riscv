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
    if(write(fd, "CHMOD Prueba\n", 12) != 12){
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
    if(write(fd, "CHMOD Funcionando\n", 17) != 17){
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
