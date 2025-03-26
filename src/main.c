#include <stdlib.h>
#include <string.h>

#include "models/database.h"
#include "models/admin.h"
#include <stdio.h>


void menu_principal() {
    printf("\n=== SISTEMA DE PUNTO DE VENTA ===\n");
    printf("1. Acceso administrativo\n");

    printf("2. Opciones Generales\n");
    printf("3. Salir\n");
    printf("Seleccione: ");
}

int main() {
    MYSQL* conn = conectar_db();

    int opcion;

    do {
        menu_principal();
        scanf("%d", &opcion);

        switch(opcion) {
            case 1:
                if(autenticar_admin(conn)) {
                    menu_administrativo(conn);
                } else {
                    printf("\n¡Credenciales incorrectas!\n");
                }
                break;
            case 2:
                printf("\n-- Módulo no implementado --\n");
                break;
            case 3:
                printf("Saliendo del sistema...\n");
                break;
            default:
                printf("Opción no válida\n");
        }

    } while(opcion != 3);

    desconectar_db(conn);
    }
}
