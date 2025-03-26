#include "../models/ventas.h"
#include "../models/database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cargar_inventario(MYSQL* conn) {
    char ruta[200];
    printf("\n--- CARGA DE INVENTARIO ---\n");
    printf("Ruta del archivo: ");
    scanf(" %199[^\n]", ruta);

    FILE* archivo = fopen(ruta, "r");
    if (!archivo) {
        perror("Error al abrir archivo");
        return;
    }

    char linea[100];
    int exitos = 0, errores = 0;  // Se aseguran de estar declaradas antes del while.

    while (fgets(linea, sizeof(linea), archivo)) {  // Se cierra correctamente el paréntesis.
        linea[strcspn(linea, "\n")] = 0;
        if (strlen(linea) == 0 || linea[0] == '#') continue;

        int id, cantidad;
        if (sscanf(linea, "%d,%d", &id, &cantidad) != 2) {
            printf("Formato inválido: %s\n", linea);
            errores++;
            continue;
        }

        // Lógica de actualización de stock (original)
        char query_verificar[100];
        sprintf(query_verificar, "SELECT stock FROM productos WHERE id_producto = %d", id);
        
        if (mysql_query(conn, query_verificar)) {
            fprintf(stderr, "Error de consulta: %s\n", mysql_error(conn));
            errores++;
            continue;
        }

        MYSQL_RES* resultado = mysql_store_result(conn);
        if (mysql_num_rows(resultado) == 0) {
            printf("Producto ID %d no existe\n", id);
            errores++;
            mysql_free_result(resultado);
            continue;
        }

        MYSQL_ROW fila = mysql_fetch_row(resultado);
        int stock_actual = atoi(fila[0]);
        mysql_free_result(resultado);

        int nuevo_stock = stock_actual + cantidad;
        if (nuevo_stock < 0) {
            printf("Stock negativo para ID %d: %d\n", id, nuevo_stock);
            errores++;
            continue;
        }

        char query_actualizar[200];
        sprintf(query_actualizar, 
            "UPDATE productos SET stock = %d WHERE id_producto = %d",
            nuevo_stock, id
        );

        if (mysql_query(conn, query_actualizar)) {
            fprintf(stderr, "Error al actualizar: %s\n", mysql_error(conn));
            errores++;
        } else {
            exitos++;
        }
    }

    fclose(archivo);
    printf("\nResultado: %d exitosas, %d errores\n", exitos, errores);
}
