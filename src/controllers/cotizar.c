#include "../models/cotizacion.h"
#include "../models/database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void menu_cotizacion(MYSQL* conn) {
    Cotizacion *cotizacion_actual = nueva_cotizacion();
    int opcion;
    
    do {
        printf("\n=== MENÚ DE COTIZACIÓN ===\n");
        printf("1. Agregar producto\n");
        printf("2. Eliminar producto\n");
        printf("3. Ver cotización actual\n");
        printf("4. Guardar cotización\n");
        printf("5. Salir sin guardar\n");
        printf("Seleccione: ");
        scanf("%d", &opcion);

        switch(opcion) {
            case 1: agregar_detalle(cotizacion_actual, conn); break;
            case 2: eliminar_detalle(cotizacion_actual); break;
            case 3: mostrar_cotizacion(cotizacion_actual); break;
            case 4: guardar_cotizacion(conn, cotizacion_actual); return;
            case 5: free(cotizacion_actual); return;
            default: printf("Opción inválida\n");
        }
    } while(1);
}

void agregar_detalle(Cotizacion *cotizacion, MYSQL* conn) {
    // Paso 1: Mostrar catálogo filtrado
    char familia[50];
    printf("\nFiltrar por familia (dejar vacío para todas): ");
    scanf(" %49[^\n]", familia);
    
    char query[512];
    snprintf(query, sizeof(query),
        "SELECT p.id_producto, p.nombre, f.descripcion, p.precio, p.stock "
        "FROM productos p "
        "JOIN familias f ON p.familia_id = f.id_familia "
        "%s",
        strlen(familia) > 0 ? "WHERE f.descripcion = ?" : "");

    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    mysql_stmt_prepare(stmt, query, strlen(query));
    
    // Vincular parámetro de familia si existe
    if(strlen(familia) > 0) {
        MYSQL_BIND param = {0};
        param.buffer_type = MYSQL_TYPE_STRING;
        param.buffer = familia;
        param.buffer_length = strlen(familia);
        mysql_stmt_bind_param(stmt, &param);
    }
    
    // Mostrar resultados...
    
    // Paso 2: Seleccionar producto y cantidad
    char id_producto[20];
    int cantidad;
    printf("\nIngrese ID del producto: ");
    scanf("%19s", id_producto);
    
    // Validar stock
    MYSQL_STMT *stmt_stock = mysql_stmt_init(conn);
    mysql_stmt_prepare(stmt_stock, "SELECT stock, precio FROM productos WHERE id_producto = ?", -1);
    
    MYSQL_BIND param_stock = {0};
    param_stock.buffer_type = MYSQL_TYPE_STRING;
    param_stock.buffer = id_producto;
    param_stock.buffer_length = strlen(id_producto);
    mysql_stmt_bind_param(stmt_stock, &param_stock);
    
    // Ejecutar y obtener stock...
    
    // Paso 3: Agregar a la estructura temporal
    DetalleCotizacion *nuevo = malloc(sizeof(DetalleCotizacion));
    strcpy(nuevo->id_producto, id_producto);
    nuevo->cantidad = cantidad;
    nuevo->siguiente = cotizacion->detalles;
    cotizacion->detalles = nuevo;
}



bool guardar_cotizacion(MYSQL* conn, Cotizacion *cotizacion) {
    // Obtener número de secuencia
    MYSQL_RES *res = mysql_query(conn, "SELECT secuencia_cotizacion FROM config");
    int secuencia = atoi(mysql_fetch_row(res)[0]);
    
    // Generar número de cotización (ej: 20250001)
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(cotizacion->numero_cotizacion, sizeof(cotizacion->numero_cotizacion),
        "%04d%04d", tm.tm_year + 1900, secuencia);
    
    // Insertar en cotizaciones
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    const char *query = "INSERT INTO cotizaciones (numero_cotizacion, cliente, subtotal, estado) "
                      "VALUES (?, ?, ?, 'PENDIENTE')";
    mysql_stmt_prepare(stmt, query, strlen(query));
    
    MYSQL_BIND params[3] = {0};
    params[0].buffer_type = MYSQL_TYPE_STRING;
    params[0].buffer = cotizacion->numero_cotizacion;
    // ... vincular otros parámetros
    
    // Insertar detalles
    DetalleCotizacion *detalle = cotizacion->detalles;
    while(detalle != NULL) {
        MYSQL_STMT *stmt_detalle = mysql_stmt_init(conn);
        const char *query_detalle = "INSERT INTO detalle_cotizacion "
                                   "(cotizacion_id, id_producto, cantidad, precio_negociado) "
                                   "VALUES (?, ?, ?, ?)";
        mysql_stmt_prepare(stmt_detalle, query_detalle, strlen(query_detalle));
        
        // Vincular parámetros...
        detalle = detalle->siguiente;
    }
    
    // Actualizar secuencia
    char update_seq[100];
    snprintf(update_seq, sizeof(update_seq),
        "UPDATE config SET secuencia_cotizacion = %d", secuencia + 1);
    mysql_query(conn, update_seq);
    
    return true;
}