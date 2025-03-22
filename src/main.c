#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "models/database.h"
#include "models/admin.h"
#include <stdio.h>

void menu_principal() {
    // ... código existente ...
}

int main() {
    MYSQL* conn = conectar_db();
    int opcion;
    
    do {
        menu_principal();
        scanf("%d", &opcion);
        
        switch (opcion) {
            case 1:
                if (autenticar_admin(conn)) {
                    menu_administrativo(conn);
                } else {
                    printf("\n¡Credenciales incorrectas!\n");
                }
                break;
            case 2:
                // Consultar catálogo (implementar después)
                break;
            case 3:
                printf("Saliendo...\n");
                break;
            default:
                printf("Opción inválida.\n");
        }
    } while (opcion != 3);

    desconectar_db(conn);
    return 0;
}