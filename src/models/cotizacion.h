#ifndef COTIZACION_H
#define COTIZACION_H

#include <mysql.h>
#include <stdbool.h>
//Structs para la gestión de cotizaciones:
typedef struct DetalleCotizacion {
    char id_producto[20];
    char nombre[50];
    int cantidad;
    double precio_negociado;
    struct DetalleCotizacion* siguiente;
} DetalleCotizacion;

typedef struct {
    int id_cotizacion;
    char numero_cotizacion[20];
    char fecha[30];
    char estado[20];
    char cliente[100];
    double subtotal;
    double total;
    DetalleCotizacion* detalles;
} Cotizacion;

// Prototipos de funciones para la gestión de cotizaciones:
bool actualizar_cotizacion(MYSQL* conn, Cotizacion *cotizacion);
// Prototipos de funciones
Cotizacion* nueva_cotizacion();
void menu_cotizacion(MYSQL* conn);
void agregar_detalle(Cotizacion *cotizacion, MYSQL* conn);
bool guardar_cotizacion(MYSQL* conn, Cotizacion *cotizacion);
void mostrar_cotizacion(Cotizacion *cotizacion);
void eliminar_detalle(Cotizacion *cotizacion);
void mostrar_catalogo(MYSQL* conn, const char* familia);
void actualizar_totales(Cotizacion *cotizacion);
// Prototipos
void modificar_cotizacion(MYSQL* conn);
Cotizacion* cargar_cotizacion(MYSQL* conn, const char* numero_cotizacion);


#endif