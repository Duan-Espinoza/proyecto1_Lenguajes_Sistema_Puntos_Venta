#include <stdlib.h>
#include <string.h>
#include "models/database.h"
#include "models/admin.h"
#include <stdio.h>

/**
 * Función que muestra el menú principal del sistema de punto de venta.
 * Permite al usuario seleccionar entre acceso administrativo, opciones generales o salir.
 */
void menu_principal() {
    printf("\n=== SISTEMA DE PUNTO DE VENTA ===\n");
    printf("1. Acceso administrativo\n");
    printf("2. Opciones Generales\n");
    printf("3. Salir\n");
    printf("Seleccione: ");
}

/**
 * Función que muestra el menú de opciones generales.
 * Permite al usuario consultar catálogo de productos, cotizar productos, modificar cotización o facturar venta.
 */
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
                //cotizarProductos();
                break;
            case 3:
                //modificarCotizacion();
                break;
            case 4:
                //facturarVenta();
                break;
            case 5:
                return;
            default:
                printf("Opción inválida\n");
        }
    }
}


/**
 * Función principal del programa.
 * Conecta a la base de datos, muestra el menú principal y gestiona las opciones seleccionadas por el usuario.
 */
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
                menuOpcionesGenerales();
                break;
            case 3:
                printf("Saliendo del sistema...\n");
                break;
            default:
                printf("Opción no válida\n");
        }
    } while(opcion != 3);

    desconectar_db(conn);
    return 0;
}