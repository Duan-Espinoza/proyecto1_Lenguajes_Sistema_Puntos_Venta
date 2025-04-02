#include "../models/productos.h"
#include "../models/database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
Nombre: registrar_familia
Descripción: Registra familias de productos en la base de datos a partir de un archivo de texto.
Entrada: Objeto MYSQL* conn
Salida: Impresión de resultados en la consola.
*/
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


/*
Nombre: familia_existe
Descripción: Verifica si una familia existe en la base de datos.
Entrada: Objeto MYSQL* conn, cadena de caracteres nombre_familia.
Salida: 1 si existe, 0 si no.
*/
int familia_existe(MYSQL* conn, const char* nombre_familia) {
    char query[100];
    sprintf(query, "SELECT id_familia FROM familias WHERE descripcion = '%s'", nombre_familia);
    
    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error de verificación: %s\n", mysql_error(conn));
        return 0;
    }
    
    MYSQL_RES* result = mysql_store_result(conn);
    int existe = mysql_num_rows(result) > 0;
    mysql_free_result(result);
    return existe;
}

/*
Nombre: producto_tiene_transacciones
Descripción: Verifica si un producto tiene transacciones asociadas.
Entrada: Objeto MYSQL* conn, cadena de caracteres id_producto.
Salida: 1 si tiene transacciones, 0 si no.
*/
int producto_tiene_transacciones(MYSQL* conn, const char* id_producto) {
    // Implementación temporal - siempre retorna falso por ahora
    return 0;
}


/*
Nombre: registrar_productos_desde_archivo
Descripción: Registra productos en la base de datos a partir de un archivo de texto.
Entrada: Objeto MYSQL* conn
Salida: Impresión de resultados en la consola.
*/
void registrar_productos_desde_archivo(MYSQL* conn) {
    char ruta_archivo[200];
    printf("\n--- REGISTRO MASIVO DE PRODUCTOS ---\n");
    printf("Ingrese la ruta del archivo (.txt): ");
    scanf(" %199[^\n]", ruta_archivo);

    FILE* archivo = fopen(ruta_archivo, "r");
    if (!archivo) {
        perror("Error al abrir el archivo");
        return;
    }

    char linea[256];
    int exitos = 0, errores = 0;
    int duplicados = 0, familias_invalidas = 0, formatos_invalidos = 0;

    printf("\nProcesando archivo...\n");
    
    while (fgets(linea, sizeof(linea), archivo)) {
        // Limpiar y validar línea
        linea[strcspn(linea, "\n")] = '\0';
        if (strlen(linea) == 0 || linea[0] == '#') continue;

        // Variables para almacenar los datos
        char id_producto[20], nombre[50], familia[50];
        float costo, precio;
        int stock;

        // Parsear línea
        if (sscanf(linea, "%19[^,],%49[^,],%49[^,],%f,%f,%d", 
                  id_producto, nombre, familia, &costo, &precio, &stock) != 6) {
            printf("Error formato: %s\n", linea);
            formatos_invalidos++;
            errores++;
            continue;
        }

        // Validar ID único
        char query_verificar_id[100];
        sprintf(query_verificar_id, "SELECT id_producto FROM productos WHERE id_producto = '%s'", id_producto);
        
        if (mysql_query(conn, query_verificar_id)) {
            fprintf(stderr, "Error verificación ID: %s\n", mysql_error(conn));
            errores++;
            continue;
        }

        MYSQL_RES* resultado_id = mysql_store_result(conn);
        if (mysql_num_rows(resultado_id) > 0) {
            printf("ID duplicado: %s\n", id_producto);
            duplicados++;
            errores++;
            mysql_free_result(resultado_id);
            continue;
        }
        mysql_free_result(resultado_id);

        // Validar existencia de familia
        if (!familia_existe(conn, familia)) {
            printf("Familia inexistente: %s\n", familia);
            familias_invalidas++;
            errores++;
            continue;
        }

        // Insertar en BD
        char query[500];
        sprintf(query, 
            "INSERT INTO productos (id_producto, nombre, familia_id, costo, precio, stock) "
            "SELECT '%s', '%s', id_familia, %.2f, %.2f, %d "
            "FROM familias WHERE descripcion = '%s'",
            id_producto, nombre, costo, precio, stock, familia
        );

        if (mysql_query(conn, query)) {
            fprintf(stderr, "Error inserción: %s\n", mysql_error(conn));
            errores++;
        } else {
            printf("Registrado: %s - %s\n", id_producto, nombre);
            exitos++;
        }
    }

    fclose(archivo);
    
    // Reporte final
    printf("\nResultado:");
    printf("\n- Líneas procesadas: %d", exitos);
    printf("\n- Errores (%d total):", errores);
    printf("\n  * Formato inválido: %d", formatos_invalidos);
    printf("\n  * IDs duplicados: %d", duplicados);
    printf("\n  * Familias inválidas: %d", familias_invalidas);
    printf("\n- Total líneas: %d\n", exitos + errores);
}


/*
Nombre: eliminar_producto_manual
Descripción: Elimina un producto de la base de datos manualmente
Entrada: Objeto MYSQL* conn
Salida: Impresión de resultados en la consola.
*/
void eliminar_producto_manual(MYSQL* conn) {
    char id_producto[20];
    printf("\n--- ELIMINAR PRODUCTO ---\n");
    printf("Ingrese el ID del producto: ");
    scanf(" %19s", id_producto);

    char query_verificar[100];
    sprintf(query_verificar, "SELECT id_producto FROM productos WHERE id_producto = '%s'", id_producto);
    
    if (mysql_query(conn, query_verificar)) {
        fprintf(stderr, "Error de verificación: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES* resultado = mysql_store_result(conn);
    if (mysql_num_rows(resultado) == 0) {
        printf("Producto no encontrado\n");
        mysql_free_result(resultado);
        return;
    }
    mysql_free_result(resultado);

    if (producto_tiene_transacciones(conn, id_producto)) {
        printf("No se puede eliminar: El producto tiene transacciones asociadas\n");
        return;
    }

    char query_eliminar[100];
    sprintf(query_eliminar, "DELETE FROM productos WHERE id_producto = '%s'", id_producto);
    
    if (mysql_query(conn, query_eliminar)) {
        fprintf(stderr, "Error eliminación: %s\n", mysql_error(conn));
    } else {
        printf("Producto %s eliminado exitosamente\n", id_producto);
    }
}


/*
Nombre: menu_gestion_productos
Descripción: Muestra el menú de gestión de productos y selecciona la opción correspondiente.
Entrada: Objeto MYSQL* conn
Salida: Ninguna
*/
void menu_gestion_productos(MYSQL* conn) {
    int opcion;
    do {
        printf("\n=== GESTIÓN DE PRODUCTOS ===");
        printf("\n1. Registrar productos desde archivo");
        printf("\n2. Eliminar producto manualmente");
        printf("\n3. Volver al menú administrativo");
        printf("\nSeleccione: ");
        scanf("%d", &opcion);

        switch(opcion) {
            case 1: 
                registrar_productos_desde_archivo(conn);
                break;
            case 2: 
                eliminar_producto_manual(conn);
                break;
            case 3: 
                printf("\nVolviendo...\n");
                break;
            default: 
                printf("\n¡Opción inválida!\n");
        }
    } while(opcion != 3);
}