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


int main() {
    int opcion;

    while (1){
        mostrarMenuPrincipal();
        printf("Seleccione una opción: ");
        scanf("%d", &opcion);
        getchar();

        switch (opcion) {
            case 1:
                //menuOpcionesAdministrativas();
                break;
            case 2:
                menuOpcionesGenerales();
                break;
            case 3:
                printf("Saliendo del sistema...\n");
                return 0;
            default:
                printf("Opción inválida\n");
    }
}
    return 0;
}


