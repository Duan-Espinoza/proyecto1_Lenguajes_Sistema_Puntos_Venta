#include "../models/productos.h"
#include "../models/database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Implementación de registro de familias
void registrar_familia(MYSQL* conn) {
    char ruta_archivo[200];
    printf("\n--- REGISTRO MASIVO DE FAMILIAS ---\n");
    printf("Ingrese la ruta del archivo (.txt): ");
    scanf(" %199[^\n]", ruta_archivo);

    FILE* archivo = fopen(ruta_archivo, "r");
    if (!archivo) {
        perror("Error al abrir el archivo");
        return;
    }

    char linea[100];
    int lineas_procesadas = 0;
    int lineas_erroneas = 0;
    int duplicados = 0;

    printf("\nProcesando archivo...\n");
    
    while (fgets(linea, sizeof(linea), archivo)) {
        // Limpiar y validar línea
        linea[strcspn(linea, "\n")] = '\0';
        if (strlen(linea) == 0 || linea[0] == '#') continue;

        // Extraer ID y descripción
        char id_familia[20];
        char descripcion[50];
        if (sscanf(linea, "%19[^,],%49[^\n]", id_familia, descripcion) != 2) {
            printf("Error formato: %s\n", linea);
            lineas_erroneas++;
            continue;
        }

        // Validar ID único
        char query_verificar[100];
        sprintf(query_verificar, "SELECT id_familia FROM familias WHERE id_familia = '%s'", id_familia);
        
        if (mysql_query(conn, query_verificar)) {
            fprintf(stderr, "Error verificación: %s\n", mysql_error(conn));
            lineas_erroneas++;
            continue;
        }

        MYSQL_RES* resultado = mysql_store_result(conn);
        if (mysql_num_rows(resultado) > 0) {
            printf("ID duplicado: %s - %s\n", id_familia, descripcion);
            duplicados++;
            lineas_erroneas++;
            mysql_free_result(resultado);
            continue;
        }
        mysql_free_result(resultado);

        // Insertar en BD
        char query_insertar[200];
        sprintf(query_insertar, 
            "INSERT INTO familias (id_familia, descripcion) VALUES ('%s', '%s')",
            id_familia, descripcion
        );

        if (mysql_query(conn, query_insertar)) {
            fprintf(stderr, "Error inserción: %s\n", mysql_error(conn));
            lineas_erroneas++;
        } else {
            printf("Registrada: %s - %s\n", id_familia, descripcion);
            lineas_procesadas++;
        }
    }

    fclose(archivo);
    
    // Reporte final
    printf("\nResultado:");
    printf("\n- Líneas procesadas: %d", lineas_procesadas);
    printf("\n- Errores:");
    printf("\n  * Formato inválido: %d", lineas_erroneas - duplicados);
    printf("\n  * IDs duplicados: %d", duplicados);
    printf("\n- Total líneas: %d\n", lineas_procesadas + lineas_erroneas);
}

// Implementación de registro de productos
void registrar_producto(MYSQL* conn) {
    char nombre[50];
    int familia_id;
    float costo, precio;
    int stock;

    printf("\n--- REGISTRAR PRODUCTO ---\n");
    printf("Nombre: ");
    scanf(" %49[^\n]", nombre);
    printf("ID Familia: ");
    if(scanf("%d", &familia_id) != 1) {
        printf("Error: ID de familia inválido\n");
        while(getchar() != '\n');  // Limpiar buffer
        return;
    }
    printf("Costo: ");
    scanf("%f", &costo);
    printf("Precio: ");
    scanf("%f", &precio);
    printf("Stock: ");
    scanf("%d", &stock);

    char query[300];
    sprintf(query, 
        "INSERT INTO productos (nombre, familia_id, costo, precio, stock) "
        "VALUES ('%s', %d, %.2f, %.2f, %d)",
        nombre, familia_id, costo, precio, stock
    );

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error: %s\n", mysql_error(conn));
    } else {
        printf("¡Producto '%s' registrado!\n", nombre);
    }
}