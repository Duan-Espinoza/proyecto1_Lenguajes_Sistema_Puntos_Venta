#include "../models/database.h"
#include <stdio.h>
/**
 * Función que conecta a la base de datos 'sistema_ventas' en MySQL.
 * Para compilar este programa, se debe tener instalado el paquete de desarrollo de MySQL.
 */

MYSQL* conectar_db() {
    MYSQL* conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, "localhost", "root", "26e$pinozaolivare$92000", 
                        "sistema_ventas", 3306, NULL, 0)) {
        fprintf(stderr, "Error de conexión: %s\n", mysql_error(conn));
        mysql_close(conn);
        exit(1);
    }
    return conn;
}


/*
Nombre: desconectar_db
Descripción: Desconecta de la base de datos MySQL.
Entrada: Objeto MYSQL* conn
Salida: Ninguna
*/
void desconectar_db(MYSQL* conn) {
    mysql_close(conn);
}