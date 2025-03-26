#include "models/database.h"
#include "models/admin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void mostrarMenuPrincipal() {
    printf("\n--- Sistema de Punto de Venta ---\n");
    printf("1. Opciones Administrativas\n");
    printf("2. Opciones Generales\n");
    printf("3. Salir\n");
}

void menuOpcionesGenerales() {
    int opcion;
    while (1) {
        printf("\n--- Opciones Generales ---\n");
        printf("1. Consultar Catálogo de Productos\n");
        printf("2. Cotizar Productos\n");
        printf("3. Modificar Cotización\n");
        printf("4. Facturar Venta\n");
        printf("5. Volver al menú principal\n");
        printf("Seleccione una opción: ");
        scanf("%d", &opcion);
        getchar(); 

        switch (opcion) {
            case 1:
                // consultarCatalogoProductos();
                break;
            case 2:
                // cotizarProductos();
                break;
            case 3:
                // modificarCotizacion();
                break;
            case 4:
                // facturarVenta();
                break;
            case 5:
                return;
            default:
                printf("Opción inválida\n");
        }
    }
}

void menuOpcionesAdministrativas(MYSQL* conn) {
    if (autenticar_admin(conn)) {
        menu_administrativo(conn);
    } else {
        printf("\n¡Credenciales incorrectas!\n");
    }
}

int main() {
    MYSQL* conn = conectar_db();
    int opcion;

    while (1) {
        mostrarMenuPrincipal();
        printf("Seleccione una opción: ");
        scanf("%d", &opcion);
        getchar();

        switch (opcion) {
            case 1:
                menuOpcionesAdministrativas(conn);
                break;
            case 2:
                menuOpcionesGenerales();
                break;
            case 3:
                printf("Saliendo del sistema...\n");
                desconectar_db(conn);
                return 0;
            default:
                printf("Opción inválida\n");
        }
    }
}
