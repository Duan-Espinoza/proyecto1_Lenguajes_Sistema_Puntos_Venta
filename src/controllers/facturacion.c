#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "../models/facturacion.h"

void ajustarCantidades(MYSQL *conn, int cotizacion_id) {
    char query[256];
    snprintf(query, sizeof(query),
             "SELECT dc.producto_id, dc.cantidad, p.stock "
             "FROM detalle_cotizacion dc "
             "JOIN productos p ON dc.producto_id = p.id_producto "
             "WHERE dc.cotizacion_id = %d", cotizacion_id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error al obtener detalles de cotización: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        fprintf(stderr, "Error al almacenar el resultado: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_ROW row;
    printf("\n--- Ajuste de Cantidades ---\n");

    while ((row = mysql_fetch_row(result))) {
        int producto_id = atoi(row[0]);
        int cantidad_solicitada = atoi(row[1]);
        int stock_disponible = atoi(row[2]);

        if (cantidad_solicitada > stock_disponible) {
            if (stock_disponible > 0) {
                snprintf(query, sizeof(query),
                         "UPDATE detalle_cotizacion SET cantidad = %d "
                         "WHERE cotizacion_id = %d AND producto_id = %d",
                         stock_disponible, cotizacion_id, producto_id);
                printf("Producto ID: %d, Cantidad ajustada de %d a %d\n",
                       producto_id, cantidad_solicitada, stock_disponible);
            } else {
                snprintf(query, sizeof(query),
                         "DELETE FROM detalle_cotizacion "
                         "WHERE cotizacion_id = %d AND producto_id = %d",
                         cotizacion_id, producto_id);
                printf("Producto ID: %d eliminado por falta de stock.\n", producto_id);
            }
            mysql_query(conn, query);
        }
    }
    
    mysql_free_result(result);
}

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

    ajustarCantidades(conn, cotizacion_id);

    printf("Ingrese el nombre del cliente: ");
    scanf(" %[^\n]", cliente); 
    
    printf("Ingrese la cédula jurídica del cliente: ");
    scanf("%s", cedula_juridica);
    
    printf("Ingrese el teléfono del cliente: ");
    scanf("%s", telefono);
    
    char query[256];
    snprintf(query, sizeof(query), "SELECT SUM(dc.cantidad * p.precio) "
                                   "FROM detalle_cotizacion dc "
                                   "JOIN productos p ON dc.producto_id = p.id_producto "
                                   "WHERE dc.cotizacion_id = %d", cotizacion_id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error en la consulta: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row || !row[0]) {
        printf("\nError al obtener el subtotal.\n");
        mysql_free_result(result);
        return;
    }

    subtotal = atof(row[0]);
    mysql_free_result(result);

    impuesto = subtotal * 0.13;
    total = subtotal + impuesto;

    snprintf(query, sizeof(query),
             "INSERT INTO facturacion (cotizacion_id, cliente, cedula_juridica, telefono, subtotal, impuesto, total) "
             "VALUES (%d, '%s', '%s', '%s', %.2f, %.2f, %.2f)",
             cotizacion_id, cliente, cedula_juridica, telefono, subtotal, impuesto, total);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error al generar la factura: %s\n", mysql_error(conn));
        return;
    }

    printf("\nFactura generada con éxito.\n");

    snprintf(query, sizeof(query), "UPDATE cotizaciones SET estado = 'facturado' WHERE id_cotizacion = %d", cotizacion_id);
    mysql_query(conn, query);

    descontarStock(conn, cotizacion_id);
    mostrarFactura(conn);
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

void mostrarFactura(MYSQL *conn) {
    char query[256];
    snprintf(query, sizeof(query), "SELECT id_factura, cotizacion_id, cliente, cedula_juridica, telefono, subtotal, impuesto, total FROM facturacion ORDER BY id_factura DESC LIMIT 1");

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error en la consulta: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        fprintf(stderr, "Error al obtener el resultado: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (!row) {
        printf("\nError: No se encontró ninguna factura.\n");
        mysql_free_result(result);
        return;
    }

    int id_factura = atoi(row[0]);
    int cotizacion_id = atoi(row[1]);
    char *cliente = row[2];
    char *cedula_juridica = row[3];
    char *telefono = row[4];
    double subtotal = atof(row[5]);
    double impuesto = atof(row[6]);
    double total = atof(row[7]);

    printf("\n--- DETALLES DE LA FACTURA ---\n");
    printf("ID Factura: %d\n", id_factura);
    printf("ID Cotización: %d\n", cotizacion_id);
    printf("Cliente: %s\n", cliente);
    printf("Cédula Jurídica: %s\n", cedula_juridica);
    printf("Teléfono: %s\n", telefono);
    printf("Subtotal: %.2f\n", subtotal);
    printf("Impuesto: %.2f\n", impuesto);
    printf("Total: %.2f\n", total);

    mysql_free_result(result);
}


/**
 * Nombre: consultaDeFacturas
 * Descripción: Esta función consulta y muestra una lista de facturas almacenadas en la base de datos.
 * Entrada: Un puntero a la conexión de MySQL.
 * Salida: Ninguna.
 */

void consultaDeFacturas(MYSQL *conn) {
    char query[256];

    // Obtener lista de facturas
    snprintf(query, sizeof(query), "SELECT id_factura, fecha, subtotal, total FROM facturacion ORDER BY id_factura DESC");

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error en la consulta de facturas: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        fprintf(stderr, "Error al obtener las facturas: %s\n", mysql_error(conn));
        return;
    }

    MYSQL_ROW row;
    printf("\n--- LISTADO DE FACTURAS ---\n");
    printf("%-10s | %-20s | %-10s | %-10s\n", "ID Factura", "Fecha y Hora", "Subtotal", "Total");
    printf("-------------------------------------------------------------\n");

    while ((row = mysql_fetch_row(result))) {
        printf("%-10s | %-20s | %-10s | %-10s\n", row[0], row[1], row[2], row[3]);
    }

    mysql_free_result(result);

    // Pedir al usuario que seleccione una factura
    int id_factura;
    printf("\nIngrese el ID de la factura para ver los detalles: ");
    if (scanf("%d", &id_factura) != 1) {
        printf("Entrada inválida.\n");
        return;
    }

    // Obtener detalles del encabezado de la factura
    snprintf(query, sizeof(query),
             "SELECT id_factura, cotizacion_id, cliente, cedula_juridica, telefono, subtotal, impuesto, total, fecha "
             "FROM facturacion WHERE id_factura = %d", id_factura);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error al obtener detalles de la factura: %s\n", mysql_error(conn));
        return;
    }

    result = mysql_store_result(conn);
    if (!result) {
        fprintf(stderr, "Error al obtener el encabezado de la factura: %s\n", mysql_error(conn));
        return;
    }

    row = mysql_fetch_row(result);
    if (!row) {
        printf("Factura con ID %d no encontrada.\n", id_factura);
        mysql_free_result(result);
        return;
    }

    // Mostrar encabezado de la factura
    printf("\n--- DETALLES DE LA FACTURA %d ---\n", id_factura);
    printf("Cliente: %s\n", row[2]);
    printf("Cédula Jurídica: %s\n", row[3]);
    printf("Teléfono: %s\n", row[4]);
    printf("Fecha y Hora: %s\n", row[8]);
    printf("Subtotal: %.2f\n", atof(row[5]));
    printf("Impuesto: %.2f\n", atof(row[6]));
    printf("Total: %.2f\n", atof(row[7]));

    mysql_free_result(result);

    // Obtener detalles de los productos en la cotización (detalle_cotizacion en lugar de detalle_factura)
    snprintf(query, sizeof(query),
             "SELECT p.id_producto, p.nombre, dc.cantidad, dc.precio_negociado "
             "FROM detalle_cotizacion dc "
             "JOIN productos p ON dc.producto_id = p.id_producto "
             "WHERE dc.cotizacion_id = %d", id_factura);  // Usamos cotizacion_id

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error al obtener detalles de los productos: %s\n", mysql_error(conn));
        return;
    }

    result = mysql_store_result(conn);
    if (!result) {
        fprintf(stderr, "Error al obtener los productos de la cotización: %s\n", mysql_error(conn));
        return;
    }

    printf("\n--- PRODUCTOS EN LA FACTURA ---\n");
    printf("%-10s | %-20s | %-10s | %-10s\n", "ID Producto", "Nombre", "Cantidad", "Precio Unitario");
    printf("-------------------------------------------------------------\n");

    while ((row = mysql_fetch_row(result))) {
        printf("%-10s | %-20s | %-10s | %-10s\n", row[0], row[1], row[2], row[3]);
    }
    
    mysql_free_result(result);
}
