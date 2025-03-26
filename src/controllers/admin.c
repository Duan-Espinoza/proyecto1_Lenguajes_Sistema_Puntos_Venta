#include "../models/admin.h"
#include "../models/productos.h"
#include "../models/database.h"
#include "../models/ventas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



// Autenticación de administrador
int autenticar_admin(MYSQL* conn) {
    char usuario[50];
    char password[50];
    
    printf("\n--- AUTENTICACIÓN ADMINISTRATIVA ---\n");
    printf("Usuario: ");
    scanf("%49s", usuario);
    printf("Contraseña: ");
    scanf("%49s", password);

    char query[200];
    sprintf(query, "SELECT * FROM config WHERE admin_user = '%s' AND admin_pass = '%s'", usuario, password);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error en la consulta: %s\n", mysql_error(conn));
        return 0;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        fprintf(stderr, "Error al obtener resultados: %s\n", mysql_error(conn));
        return 0;
    }

    int num_rows = mysql_num_rows(result);
    mysql_free_result(result);
    return num_rows > 0;
}

/** 
//Registro de familias
void registrar_familia(MYSQL* conn) {
    char descripcion[50];
    printf("\n--- REGISTRAR FAMILIA ---\n");
    printf("Ingrese la descripción: ");
    scanf(" %49[^\n]", descripcion);

    char query[200];
    sprintf(query, "INSERT INTO familias (descripcion) VALUES ('%s')", descripcion);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error al registrar: %s\n", mysql_error(conn));
    } else {
        printf("¡Familia '%s' registrada!\n", descripcion);
    }
}

*/



/**
// Registro de productos
void registrar_producto(MYSQL* conn) {
    char nombre[50];
    int familia_id;
    float costo, precio;
    int stock;

    printf("\n--- REGISTRAR PRODUCTO ---\n");
    printf("Nombre: ");
    scanf(" %49[^\n]", nombre);
    printf("ID Familia: ");
    scanf("%d", &familia_id);
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

*/

/** 
// Carga de inventario
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

    while (fgets(linea, sizeof(linea), archivo)) {
        linea[strcspn(linea, "\n")] = 0;
        if (strlen(linea) == 0 || linea[0] == '#') continue;

        int id, cantidad;
        if (sscanf(linea, "%d,%d", &id, &cantidad) != 2) {
            printf("Formato inválido: %s\n", linea);
            errores++;
            continue;
        }

        // Lógica de actualización...
        // (Mantener misma implementación original)
    }

    fclose(archivo);
    printf("\nResultado: %d exitosas, %d errores\n", exitos, errores);
}


*/

// Menú administrativo
void menu_administrativo(MYSQL* conn) {
    int opcion;
    do {
        printf("\n=== MENÚ ADMINISTRATIVO ===\n");
        printf("1. Registrar familia\n");
        printf("2. Registrar producto\n");
        printf("3. Cargar inventario\n");
        printf("4. Volver\n");
        printf("Opción: ");
        scanf("%d", &opcion);

        switch(opcion) {
            case 1: registrar_familia(conn); break;
            case 2: registrar_producto(conn); break;
            case 3: cargar_inventario(conn); break;
            case 4: printf("Volviendo...\n"); break;
            default: printf("Opción inválida\n");
        }
    } while(opcion != 4);
}