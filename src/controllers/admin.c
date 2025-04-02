#include "../models/admin.h"
#include "../models/productos.h"
#include "../models/database.h"
#include "../models/ventas.h"
#include "../models/facturacion.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/*
Nombre: conectar_db
Descripción: Conecta a la base de datos 'sistema_ventas' en MySQL.
Entrada: Objeto MYSQL* conn
Salida: Objeto MYSQL* conn conectado a la base de datos.
*/
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

/*
Nombre: menu_administrativo
Descripción: Muestra el menú administrativo y permite al usuario seleccionar una opción.
Entrada: Objeto MYSQL* conn
Salida: Ninguna.
*/
void menu_administrativo(MYSQL* conn) {
    int opcion;
    do {
        printf("\n=== MENÚ ADMINISTRATIVO ===\n");
        printf("1. Registrar familia\n");
        printf("2. Gestión de productos\n");
        printf("3. Cargar inventario\n");
        printf("4. Consulta de facturas\n");
        printf("5. Volver\n");
        printf("Opción: ");
        scanf("%d", &opcion);

        switch(opcion) {
            case 1: registrar_familia(conn); break;
            case 2: menu_gestion_productos(conn); break;
            case 3: cargar_inventario(conn); break;
            case 4: consultaDeFacturas(conn); break;
            case 5: printf("Volviendo...\n"); break;
            default: printf("Opción inválida\n");
        }
    } while(opcion != 5);
}

/*
Nombre: limpiarBuffer
Descripción: Limpia el buffer de entrada para evitar problemas con fgets.
Entrada: Ninguna.
Salida: Ninguna.
*/

void limpiarBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}


/*
Nombre: existeFamilia
Descripción: Verifica si una familia existe en la base de datos.
Entrada: Objeto MYSQL* conn, cadena de caracteres familia.
Salida: 1 si existe, 0 si no.
*/

int existeFamilia(MYSQL* conn, const char* familia) {
    char query[200];
    snprintf(query, sizeof(query), "SELECT COUNT(*) FROM familias WHERE id_familia = '%s'", familia);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error en la validación de la familia: %s\n", mysql_error(conn));
        return 0;
    }

    MYSQL_RES* result = mysql_store_result(conn);
    if (!result) {
        fprintf(stderr, "Error al obtener resultados: %s\n", mysql_error(conn));
        return 0;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    int existe = row ? atoi(row[0]) : 0;

    mysql_free_result(result);
    return existe;
}

/*
Nombre: consultarCatalogoProductos
Descripción: Consulta el catálogo de productos en la base de datos.
Entrada: Objeto MYSQL* conn
Salida: Impresión del catálogo de productos en la consola.
*/
void consultarCatalogoProductos(MYSQL* conn) {
    MYSQL_RES* result;
    MYSQL_ROW row;
    char query[300];
    char familia[50];
    char familiaEscapada[100];

    printf("\n--- CONSULTAR CATÁLOGO DE PRODUCTOS ---\n");
    printf("Filtrar por familia (deje en blanco para mostrar todos): ");

    // Limpiar el buffer antes de leer la entrada
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    if (fgets(familia, sizeof(familia), stdin) == NULL) {
        fprintf(stderr, "Error al leer la entrada.\n");
        return;
    }

    // Eliminar el salto de línea si existe
    familia[strcspn(familia, "\n")] = '\0';

    if (strlen(familia) > 0) {
        mysql_real_escape_string(conn, familiaEscapada, familia, strlen(familia));

        if (!existeFamilia(conn, familiaEscapada)) {
            printf("\nError: La familia ingresada no existe.\n");
            return;
        }

        snprintf(query, sizeof(query),
                "SELECT id_producto, nombre, familia_id, costo, precio, stock FROM productos WHERE familia_id = '%s'",
                familiaEscapada);
    } else {
        snprintf(query, sizeof(query),
                "SELECT id_producto, nombre, familia_id, costo, precio, stock FROM productos");
    }

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error en la consulta: %s\n", mysql_error(conn));
        return;
    }

    result = mysql_store_result(conn);
    if (!result) {
        fprintf(stderr, "Error al obtener resultados: %s\n", mysql_error(conn));
        return;
    }

    int num_rows = mysql_num_rows(result);
    if (num_rows == 0) {
        printf("\nNo se encontraron productos.\n");
        mysql_free_result(result);
        return;
    }

    printf("\n%-5s | %-20s | %-10s | %-10s | %-5s\n", "ID", "Nombre", "Familia", "Precio", "Stock");
    printf("-------------------------------------------------------------\n");

    while ((row = mysql_fetch_row(result))) {
        printf("%-5s | %-20s | %-10s | %-10s | %-5s\n",
               row[0], row[1], row[2], row[4], row[5]);  // Ajuste en índices para evitar errores
    }

    mysql_free_result(result);
}
