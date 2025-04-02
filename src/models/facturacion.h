#ifndef FACTURACION_H
#define FACTURACION_H

#include <mysql.h>

// Estructura para almacenar los datos de facturación
typedef struct {
    int id_factura;
    int cotizacion_id;
    char cliente[100];
    char cedula_juridica[20];
    char telefono[20];
    char fecha[20];
    double subtotal;
    double impuesto;
    double total;
} Factura;

// Funciones
void facturar(MYSQL *conn);
int verificarCotizacion(MYSQL *conn, const char *numero_cotizacion);
void descontarStock(MYSQL *conn, int cotizacion_id);
void mostrarFactura(MYSQL *conn);

#endif
