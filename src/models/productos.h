#ifndef PRODUCTOS_H
#define PRODUCTOS_H

#include "../models/database.h"  // Para MYSQL*

// Registro de familias
void registrar_familia(MYSQL* conn);

// Registro de productos
void registrar_producto(MYSQL* conn);

// Carga de inventario (si corresponde)
void cargar_inventario(MYSQL* conn);

#endif