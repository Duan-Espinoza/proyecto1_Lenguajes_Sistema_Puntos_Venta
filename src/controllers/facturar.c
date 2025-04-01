#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "facturacion.h"

void facturar(MYSQL *conn) {
    char numero_cotizacion[20], cliente[100], cedula_juridica[20], telefono[20];
    double subtotal, impuesto, total;
    int cotizacion_id;

    printf("\n--- FACTURACIÓN ---\n");
    printf("Ingrese el número de cotización: ");
    scanf("%s", numero_cotizacion);

    cotizacion_id = verificarCotizacion(conn, numero_cotizacion);
    if (cotizacion_id == -1) {
        printf("\nError: La cotización no existe o ya fue facturada.\n");
        return;
    }

    printf("Ingrese el nombre del cliente: ");
    scanf(" %[^\n]s", cliente);
    printf("Ingrese la cédula jurídica del cliente: ");
    scanf("%s", cedula_juridica);
    printf("Ingrese el teléfono del cliente: ");
    scanf("%s", telefono);

    // Obtener el subtotal de la cotización
    char query[256];
    snprintf(query, sizeof(query), "SELECT subtotal FROM cotizaciones WHERE id_cotizacion = %d", cotizacion_id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error en la consulta: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        printf("\nError al obtener el subtotal.\n");
        mysql_free_result(result);
        return;
    }

    subtotal = atof(row[0]);
    mysql_free_result(result);

    // Calcular impuesto y total
    impuesto = subtotal * 0.13;
    total = subtotal + impuesto;

    // Insertar la factura en la base de datos
    snprintf(query, sizeof(query),
             "INSERT INTO facturacion (cotizacion_id, cliente, cedula_juridica, telefono, subtotal, impuesto, total) "
             "VALUES (%d, '%s', '%s', '%s', %.2f, %.2f, %.2f)",
             cotizacion_id, cliente, cedula_juridica, telefono, subtotal, impuesto, total);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error al generar la factura: %s\n", mysql_error(conn));
        return;
    }

    printf("\nFactura generada con éxito.\n");

    // Actualizar la cotización a estado "facturado"
    snprintf(query, sizeof(query), "UPDATE cotizaciones SET estado = 'facturado' WHERE id_cotizacion = %d", cotizacion_id);
    mysql_query(conn, query);

    // Descontar stock
    descontarStock(conn, cotizacion_id);
}

int verificarCotizacion(MYSQL *conn, const char *numero_cotizacion) {
    char query[256];
    snprintf(query, sizeof(query),
             "SELECT id_cotizacion FROM cotizaciones WHERE numero_cotizacion = '%s' AND estado = 'pendiente'", numero_cotizacion);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error en la consulta: %s\n", mysql_error(conn));
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        return -1;
    }

    int id_cotizacion = atoi(row[0]);
    mysql_free_result(result);
    return id_cotizacion;
}

void descontarStock(MYSQL *conn, int cotizacion_id) {
    char query[256];
    snprintf(query, sizeof(query),
             "SELECT producto_id, cantidad FROM detalle_cotizacion WHERE cotizacion_id = %d", cotizacion_id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error al obtener detalles de cotización: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result))) {
        char producto_id[20];
        int cantidad = atoi(row[1]);

        snprintf(producto_id, sizeof(producto_id), "%s", row[0]);

        snprintf(query, sizeof(query),
                 "UPDATE productos SET stock = stock - %d WHERE id_producto = '%s'", cantidad, producto_id);

        mysql_query(conn, query);
    }

    mysql_free_result(result);
    printf("\nStock actualizado correctamente.\n");
}
