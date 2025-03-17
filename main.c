#include <stdio.h>
#include <stdlib.h>
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
                // Lógica de autenticación
                break;
            case 2:
                // Consultar catálogo
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