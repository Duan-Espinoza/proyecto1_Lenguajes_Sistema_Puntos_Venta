#ifndef COTIZACION_H
#define COTIZACION_H

#include <mysql.h>
#include <stdbool.h>

typedef struct DetalleCotizacion {
    char id_producto[20];
    char nombre[50];
    int cantidad;
    float precio_unitario;
    float precio_negociado;
    struct DetalleCotizacion* siguiente;
} DetalleCotizacion;

typedef struct {
    int id;
    char numero_cotizacion[20];
    char fecha[30];
    char estado[20];
    char cliente[100];
    float subtotal;
    float total;
    DetalleCotizacion* detalles;
} Cotizacion;

// Prototipos de funciones
Cotizacion* nueva_cotizacion();
void agregar_detalle(Cotizacion *cotizacion, MYSQL* conn);
void mostrar_cotizacion(Cotizacion *cotizacion);
bool guardar_cotizacion(MYSQL* conn, Cotizacion *cotizacion);

#endif