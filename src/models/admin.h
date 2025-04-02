#ifndef ADMIN_H
#define ADMIN_H

#include "../models/database.h"

// Autenticación
int autenticar_admin(MYSQL* conn);
void menu_administrativo(MYSQL* conn);

// Registros
void registrar_familia(MYSQL* conn);
void registrar_producto(MYSQL* conn);

// Inventario
void cargar_inventario(MYSQL* conn);

//Catalogo
void consultarCatalogoProductos(MYSQL* conn);

//Mostrar estadísticas
void mostrar_estadisticas(MYSQL* conn);


#endif