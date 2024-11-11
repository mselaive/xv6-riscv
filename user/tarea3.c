#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void) {
    // Obtener la dirección actual del heap antes de reservar memoria
    char *heap_start = sbrk(0);
    printf("Heap original en: %p\n", heap_start);

    // Reservar una nueva página de memoria
    sbrk(4096);

    // Proteger la nueva página reservada
    if (mprotect(heap_start, 1) == -1) {
        printf("Error al proteger la página\n");
        exit(1);
    }

    // Intentar escribir en la página protegida
    char *address = heap_start;
    printf("Intentando escribir en la dirección protegida: %p\n", address);
    printf("Valor antes de escribir: %p\n", address);
    
    // Esto debería fallar si la protección está activada correctamente
    *address = 'X';  
    printf("Valor después de intentar escribir: %p\n", address);

    // Desproteger la página para permitir la escritura
    if (munprotect(heap_start, 1) == -1) {
        printf("Error al desproteger la página\n");
        exit(1);
    }

    // Ahora escribir en la página desprotegida
    *address = 'Y';  // Esto debería ser exitoso después de desproteger
    printf("Escribiendo en la dirección desprotegida: %p\n", address);
    printf("Nuevo valor: %p\n", address);

    exit(0);
}
