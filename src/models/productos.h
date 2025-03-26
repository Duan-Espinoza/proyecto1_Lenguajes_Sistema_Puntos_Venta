#ifndef PRODUCTOS_H
#define PRODUCTOS_H

#include "../models/database.h"  // Para MYSQL*

// Registro de familias
void registrar_familia(MYSQL* conn);  // Actualizada para carga por archivo

// Menú y operaciones de productos
void menu_gestion_productos(MYSQL* conn);
void registrar_productos_desde_archivo(MYSQL* conn);
void eliminar_producto_manual(MYSQL* conn);

// Carga de inventario (si corresponde)
void cargar_inventario(MYSQL* conn);

#endif