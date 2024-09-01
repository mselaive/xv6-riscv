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
