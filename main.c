#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>

/**
 * Función que conecta a la base de datos 'sistema_ventas' en MySQL.
 * Para compilar este programa, se debe tener instalado el paquete de desarrollo de MySQL.
 * Acá se encuentran las credenciales que solicita MySQL
 * Despues de root, cambiar a la contraseña asignada
 */
MYSQL *conectar_db() {
    MYSQL *conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "localhost", "root", "26e$pinozaolivare$92000", "sistema_ventas", 3306, NULL, 0)) {
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
     //Esta funcionalidad debe cambiar
    // Debe solicitar una ruta, y segun el archivo de texto (csv) preferiblemente, incluirlo la información a la base de datos
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

    //Esta funcionalidad debe cambiar
    // Debe solicitar una ruta, y segun el archivo de texto (csv) preferiblemente, incluirlo la información a la base de datos
    

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

/**
 * Función que carga un inventario desde un archivo de texto.
 * El archivo debe tener el siguiente formato:
 * ID,CANTIDAD
 */
void cargar_inventario(MYSQL *conn) {
    char ruta_archivo[200];
    printf("\n--- CARGA DE INVENTARIO ---\n");
    printf("Ingrese la ruta del archivo: ");
    scanf(" %199[^\n]", ruta_archivo);

    FILE *archivo = fopen(ruta_archivo, "r");
    if (!archivo) {
        perror("Error al abrir el archivo");
        return;
    }

    char linea[100];
    int lineas_procesadas = 0;
    int lineas_erroneas = 0;

    while (fgets(linea, sizeof(linea), archivo)) {
        // Eliminar salto de línea
        linea[strcspn(linea, "\n")] = 0;

        // Saltar líneas vacías o comentarios
        if (strlen(linea) == 0 || linea[0] == '#') continue;

        // Parsear ID y cantidad
        int id_producto, cantidad;
        if (sscanf(linea, "%d,%d", &id_producto, &cantidad) != 2) {
            printf("Error en formato: %s\n", linea);
            lineas_erroneas++;
            continue;
        }

        // Validar existencia del producto
        char query_verificar[100];
        sprintf(query_verificar, "SELECT stock FROM productos WHERE id_producto = %d", id_producto);
        
        if (mysql_query(conn, query_verificar)) {
            fprintf(stderr, "Error de consulta: %s\n", mysql_error(conn));
            lineas_erroneas++;
            continue;
        }

        MYSQL_RES *resultado = mysql_store_result(conn);
        if (mysql_num_rows(resultado) == 0) {
            printf("Producto ID %d no existe\n", id_producto);
            lineas_erroneas++;
            mysql_free_result(resultado);
            continue;
        }

        // Calcular nuevo stock
        MYSQL_ROW fila = mysql_fetch_row(resultado);
        int stock_actual = atoi(fila[0]);
        mysql_free_result(resultado);

        int nuevo_stock = stock_actual + cantidad;
        if (nuevo_stock < 0) {
            printf("Stock negativo para ID %d: %d\n", id_producto, nuevo_stock);
            lineas_erroneas++;
            continue;
        }

        // Actualizar base de datos
        char query_actualizar[200];
        sprintf(query_actualizar, 
            "UPDATE productos SET stock = %d WHERE id_producto = %d",
            nuevo_stock, id_producto
        );

        if (mysql_query(conn, query_actualizar)) {
            fprintf(stderr, "Error al actualizar: %s\n", mysql_error(conn));
            lineas_erroneas++;
        } else {
            lineas_procesadas++;
        }
    }

    fclose(archivo);
    printf("\nResultado:\n");
    printf("- Líneas procesadas: %d\n", lineas_procesadas);
    printf("- Líneas con errores: %d\n", lineas_erroneas);
}


//Este es el menú que mostrará las opciones administrativas
/**
 * Función que muestra el menú administrativo.
 * Permite registrar familias de productos, productos y cargar inventario.
 * @param conn Conexión a la base de datos.
 */
void menu_administrativo(MYSQL *conn) {
    int opcion_admin;
    do {
        printf("\n--- MENÚ ADMINISTRATIVO ---\n");
        printf("1. Registrar familia de productos\n");
        printf("2. Registrar producto\n");
        printf("3. Cargar inventario\n");
        printf("4. Volver al menú principal\n");
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
                cargar_inventario(conn);
                break;
            case 4:
                printf("Volviendo al menú principal...\n");
                break;
            default:
                printf("Opción inválida.\n");
        }
    } while (opcion_admin != 4);
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