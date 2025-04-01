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
    int exitos = 0, errores = 0;
    int num_linea = 0;

    while (fgets(linea, sizeof(linea), archivo)) {
        num_linea++;
        linea[strcspn(linea, "\n")] = 0;
        if (strlen(linea) == 0 || linea[0] == '#') continue;

        char id_producto[20];
        int cantidad;
        if (sscanf(linea, "%19[^,],%d", id_producto, &cantidad) != 2) {
            printf("Línea %d: Formato inválido\n", num_linea);
            errores++;
            continue;
        }

        // Paso 1: Verificar si el producto existe
        MYSQL_STMT *stmt_check = mysql_stmt_init(conn);
        const char *query_check = "SELECT stock FROM productos WHERE id_producto = ?";
        mysql_stmt_prepare(stmt_check, query_check, strlen(query_check));

        MYSQL_BIND param_check = {0};
        param_check.buffer_type = MYSQL_TYPE_STRING;
        param_check.buffer = id_producto;
        param_check.buffer_length = strlen(id_producto);
        mysql_stmt_bind_param(stmt_check, &param_check);
        mysql_stmt_execute(stmt_check);

        MYSQL_RES *resultado = mysql_stmt_result_metadata(stmt_check);
        if (mysql_stmt_fetch(stmt_check) != 0) {
            printf("Línea %d: Producto %s no existe\n", num_linea, id_producto);
            errores++;
            mysql_stmt_close(stmt_check);
            continue;
        }
        mysql_stmt_close(stmt_check);

        // Paso 2: Actualizar stock con consulta preparada
        MYSQL_STMT *stmt = mysql_stmt_init(conn);
        const char *query = "UPDATE productos SET stock = stock + ? WHERE id_producto = ?";
        mysql_stmt_prepare(stmt, query, strlen(query));

        MYSQL_BIND params[2];
        memset(params, 0, sizeof(params));
        params[0].buffer_type = MYSQL_TYPE_LONG;
        params[0].buffer = &cantidad;
        params[1].buffer_type = MYSQL_TYPE_STRING;
        params[1].buffer = id_producto;
        params[1].buffer_length = strlen(id_producto);

        mysql_stmt_bind_param(stmt, params);
        mysql_stmt_execute(stmt);

        if (mysql_stmt_errno(stmt) != 0) {
            fprintf(stderr, "Línea %d: Error SQL: %s\n", num_linea, mysql_stmt_error(stmt));
            errores++;
        } else {
            exitos++;
        }

        mysql_stmt_close(stmt);
    }

    fclose(archivo);
    printf("\nResultado: %d líneas exitosas, %d errores\n", exitos, errores);
}