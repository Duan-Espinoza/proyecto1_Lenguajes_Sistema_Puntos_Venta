#ifndef DATABASE_H
#define DATABASE_H

#include <mysql.h>
/**
 * Función que conecta a la base de datos 'sistema_ventas' en MySQL.
 * Para compilar este programa, se debe tener instalado el paquete de desarrollo de MySQL.
 */
MYSQL* conectar_db();
void desconectar_db(MYSQL* conn);

#endif