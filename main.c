#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>

MYSQL *conectar_db() {
    MYSQL *conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "localhost", "root", "root123", "sistema_ventas", 3306, NULL, 0)) {
        fprintf(stderr, "Error de conexión: %s\n", mysql_error(conn));
        mysql_close(conn);
        exit(1);
    }
    return conn;
}

int autenticar_admin(MYSQL *conn) {
    char usuario[50];
    char password[50];
    
    printf("\n--- AUTENTICACIÓN ADMINISTRATIVA ---\n");
    printf("Usuario: ");
    scanf("%s", usuario);
    printf("Contraseña: ");
    scanf("%s", password);

    char query[200];
    sprintf(query, "SELECT * FROM config WHERE admin_user = '%s' AND admin_pass = '%s'", 
            usuario, password);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error en la consulta: %s\n", mysql_error(conn));
        return 0;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) {
        fprintf(stderr, "Error al obtener resultados: %s\n", mysql_error(conn));
        return 0;
    }

    int num_rows = mysql_num_rows(result);
    mysql_free_result(result);

    return num_rows > 0;
}

//Función que registra la familia de productos a la database
void registrar_familia(MYSQL *conn) {
    char descripcion[50];
    
    printf("\n--- REGISTRAR FAMILIA ---\n");
    printf("Ingrese la descripción de la familia: ");
    scanf(" %49[^\n]", descripcion);  // Leer hasta 49 caracteres incluyendo espacios

    char query[200];
    sprintf(query, "INSERT INTO familias (descripcion) VALUES ('%s')", descripcion);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error al registrar familia: %s\n", mysql_error(conn));
    } else {
        printf("¡Familia '%s' registrada exitosamente!\n", descripcion);
    }
}

//Función que registra los productos a la database
void registrar_producto(MYSQL *conn) {
    char nombre[50];
    int familia_id;
    float costo, precio;
    int stock;

    printf("\n--- REGISTRAR PRODUCTO ---\n");
    printf("Nombre del producto: ");
    scanf(" %49[^\n]", nombre);
    printf("ID de la familia: ");
    scanf("%d", &familia_id);
    printf("Costo: ");
    scanf("%f", &costo);
    printf("Precio de venta: ");
    scanf("%f", &precio);
    printf("Stock inicial: ");
    scanf("%d", &stock);

    char query[300];
    sprintf(query, 
        "INSERT INTO productos (nombre, familia_id, costo, precio, stock) "
        "VALUES ('%s', %d, %.2f, %.2f, %d)",
        nombre, familia_id, costo, precio, stock
    );

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error al registrar producto: %s\n", mysql_error(conn));
    } else {
        printf("¡Producto '%s' registrado exitosamente!\n", nombre);
    }
}


//Este es el menú que mostrará las opciones administrativas
void menu_administrativo(MYSQL *conn) {
    int opcion_admin;
    do {
        printf("\n--- MENÚ ADMINISTRATIVO ---\n");
        printf("1. Registrar familia de productos\n");
        printf("2. Registrar producto\n");
        printf("3. Volver al menú principal\n");
        printf("Seleccione una opción: ");
        scanf("%d", &opcion_admin);

        switch (opcion_admin) {
            case 1:
                registrar_familia(conn);
                break;
            case 2:
                registrar_producto(conn);
                break;
            case 3:
                printf("Volviendo al menú principal...\n");
                break;
            default:
                printf("Opción inválida.\n");
        }
    } while (opcion_admin != 3);
}

void menu_principal() {
    printf("\n--- MENÚ PRINCIPAL ---\n");
    printf("1. Opciones Administrativas\n");
    printf("2. Consultar Catálogo\n");
    printf("3. Salir\n");
    printf("Seleccione una opción: ");
}

int main() {
    MYSQL *conn = conectar_db();
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

    mysql_close(conn);
    return 0;
}