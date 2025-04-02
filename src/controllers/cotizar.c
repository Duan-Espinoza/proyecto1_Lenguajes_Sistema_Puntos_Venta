#include "../models/cotizacion.h"
#include "../models/database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>



/**
 * @brief Crea e inicializa una nueva cotización
 */
Cotizacion* nueva_cotizacion() {
    Cotizacion *cot = malloc(sizeof(Cotizacion));
    if(!cot) {
        fprintf(stderr, "Error asignando memoria\n");
        exit(EXIT_FAILURE);
    }
    
    cot->id_cotizacion  = 0;
    strcpy(cot->numero_cotizacion, "");
    strcpy(cot->fecha, "");
    strcpy(cot->estado, "PENDIENTE");
    strcpy(cot->cliente, "");
    cot->subtotal = 0.0;
    cot->total = 0.0;
    cot->detalles = NULL;
    
    return cot;
}



/**
 * @brief Menú principal de cotización
 */
void menu_cotizacion(MYSQL* conn) {
    Cotizacion *cot = nueva_cotizacion();
    
    printf("\nIngrese nombre del cliente: ");
    scanf(" %49[^\n]", cot->cliente);
    
    int opcion;
    do {
        printf("\n=== MENÚ COTIZACIÓN ===\n");
        printf("1. Agregar producto\n");
        printf("2. Eliminar producto\n");
        printf("3. Ver cotización\n");
        printf("4. Guardar y salir\n");
        printf("5. Salir sin guardar\n");
        printf("Seleccione: ");
        scanf("%d", &opcion);

        switch(opcion) {
            case 1: agregar_detalle(cot, conn); break;
            case 2: eliminar_detalle(cot); break;
            case 3: mostrar_cotizacion(cot); break;
            case 4: 
                if(guardar_cotizacion(conn, cot)) {
                    printf("Cotización %s guardada!\n", cot->numero_cotizacion);
                }
                free(cot);
                return;
            case 5:
                free(cot);
                return;
            default: printf("Opción inválida\n");
        }
    } while(1);
}

/**
 * @brief Agrega un producto a la cotización
 */
void agregar_detalle(Cotizacion *cot, MYSQL* conn) {
    char familia[50] = {0};
    printf("\nFiltrar por familia (vacío=todas): ");
    scanf(" %49[^\n]", familia);
    
    mostrar_catalogo(conn, familia);

    char id_producto[20] = {0};
    printf("\nID Producto: ");
    scanf("%19s", id_producto);

    // Escapar entrada del usuario
    char id_escapado[40] = {0};
    mysql_real_escape_string(conn, id_escapado, id_producto, strlen(id_producto));

    // Obtener datos del producto
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    const char *query = "SELECT nombre, precio, stock FROM productos WHERE id_producto = ?";
    if(mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "Error preparando consulta: %s\n", mysql_stmt_error(stmt));
        return;
    }

    MYSQL_BIND param = {0};
    param.buffer_type = MYSQL_TYPE_STRING;
    param.buffer = id_escapado;
    param.buffer_length = strlen(id_escapado);
    
    if(mysql_stmt_bind_param(stmt, &param)) {
        fprintf(stderr, "Error vinculando parámetro: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    char nombre[50] = {0};
    double precio = 0.0;
    int stock = 0;
    MYSQL_BIND result_bind[3] = {0};
    
    result_bind[0].buffer_type = MYSQL_TYPE_STRING;
    result_bind[0].buffer = nombre;
    result_bind[0].buffer_length = sizeof(nombre);
    
    result_bind[1].buffer_type = MYSQL_TYPE_DOUBLE;
    result_bind[1].buffer = &precio;
    
    result_bind[2].buffer_type = MYSQL_TYPE_LONG;
    result_bind[2].buffer = &stock;

    if(mysql_stmt_bind_result(stmt, result_bind)) {
        fprintf(stderr, "Error vinculando resultados: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    if(mysql_stmt_execute(stmt)) {
        fprintf(stderr, "Error ejecutando consulta: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    if(mysql_stmt_fetch(stmt)) {
        printf("Producto no encontrado!\n");
        mysql_stmt_close(stmt);
        return;
    }
    mysql_stmt_close(stmt);

    // Validar cantidad
    int cantidad = 0;
    do {
        printf("Cantidad (Stock disponible: %d): ", stock);
        scanf("%d", &cantidad);
        if(cantidad <= 0) printf("La cantidad debe ser mayor a cero\n");
    } while(cantidad <= 0);

    // Buscar producto existente
    DetalleCotizacion *actual = cot->detalles;
    while(actual) {
        if(strcmp(actual->id_producto, id_producto) == 0) {
            if((actual->cantidad + cantidad) > stock) {
                printf("Error: Supera el stock disponible (%d + %d > %d)\n",
                      actual->cantidad, cantidad, stock);
                return;
            }
            actual->cantidad += cantidad;
            actualizar_totales(cot);
            printf("Cantidad actualizada: %d\n", actual->cantidad);
            return;
        }
        actual = actual->siguiente;
    }

    // Validar stock para nuevo producto
    if(cantidad > stock) {
        printf("Error: Stock insuficiente (%d > %d)\n", cantidad, stock);
        return;
    }

    // Crear nuevo detalle
    DetalleCotizacion *nuevo = malloc(sizeof(DetalleCotizacion));
    strcpy(nuevo->id_producto, id_producto);
    strcpy(nuevo->nombre, nombre);
    nuevo->cantidad = cantidad;
    nuevo->precio_negociado = precio;
    nuevo->siguiente = cot->detalles;
    cot->detalles = nuevo;
    
    actualizar_totales(cot);
    printf("Producto agregado correctamente\n");
}


/**
 * @brief Actualiza los totales de la cotización
 */
void actualizar_totales(Cotizacion *cot) {
    cot->subtotal = 0.0;
    DetalleCotizacion *det = cot->detalles;
    while(det) {
        cot->subtotal += det->cantidad * det->precio_negociado;
        det = det->siguiente;
    }
    cot->total = cot->subtotal * 1.16;  // IVA del 16%
}


/**
 * @brief Guarda la cotización en la base de datos
 */
bool guardar_cotizacion(MYSQL* conn, Cotizacion *cot) {
    // 1. Iniciar transacción
    mysql_autocommit(conn, 0);  // Desactivar autocommit
    if (mysql_query(conn, "START TRANSACTION")) {
        fprintf(stderr, "Error iniciando transacción: %s\n", mysql_error(conn));
        return false;
    }

    // 2. Bloquear y obtener secuencia (¡CRÍTICO!)
    if (mysql_query(conn, "SELECT secuencia_cotizacion FROM config FOR UPDATE")) {  // ¡FOR UPDATE!
        fprintf(stderr, "Error bloqueando secuencia: %s\n", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        return false;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (!res || mysql_num_rows(res) == 0) {
        fprintf(stderr, "Error: No se encontró la secuencia\n");
        if (res) mysql_free_result(res);
        mysql_query(conn, "ROLLBACK");
        return false;
    }

    MYSQL_ROW row = mysql_fetch_row(res);
    int secuencia = atoi(row[0]);
    mysql_free_result(res);

    // 3. Actualizar secuencia PRIMERO (evita duplicados)
    char update[50];
    snprintf(update, sizeof(update), "UPDATE config SET secuencia_cotizacion = %d", secuencia + 1);
    if (mysql_query(conn, update)) {
        fprintf(stderr, "Error actualizando secuencia: %s\n", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        return false;
    }

    // 4. Generar número de cotización CON secuencia actualizada
    snprintf(cot->numero_cotizacion, 20, "COT-%03d", secuencia + 1);  // ¡Usar secuencia + 1!

    // 5. Insertar cabecera
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    const char *query = 
        "INSERT INTO cotizaciones (numero_cotizacion, cliente, subtotal, estado, fecha) "
        "VALUES (?, ?, ?, 'PENDIENTE', NOW())";
    
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "Error preparando query: %s\n", mysql_stmt_error(stmt));
        mysql_query(conn, "ROLLBACK");
        return false;
    }

    MYSQL_BIND params[3] = {0};
    params[0].buffer_type = MYSQL_TYPE_STRING;
    params[0].buffer = cot->numero_cotizacion;
    params[0].buffer_length = strlen(cot->numero_cotizacion);
    
    params[1].buffer_type = MYSQL_TYPE_STRING;
    params[1].buffer = cot->cliente;
    params[1].buffer_length = strlen(cot->cliente);
    
    params[2].buffer_type = MYSQL_TYPE_DOUBLE;
    params[2].buffer = &cot->subtotal;

    if (mysql_stmt_bind_param(stmt, params) || mysql_stmt_execute(stmt)) {
        fprintf(stderr, "Error insertando cabecera: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_query(conn, "ROLLBACK");
        return false;
    }
    mysql_stmt_close(stmt);

    // 6. Insertar detalles usando LAST_INSERT_ID()
    DetalleCotizacion *det = cot->detalles;
    while (det) {
        MYSQL_STMT *stmt_det = mysql_stmt_init(conn);
        const char *query_det = 
            "INSERT INTO detalle_cotizacion (cotizacion_id, producto_id, cantidad, precio_negociado) "
            "VALUES (LAST_INSERT_ID(), ?, ?, ?)";
        
        if (mysql_stmt_prepare(stmt_det, query_det, strlen(query_det))) {
            fprintf(stderr, "Error preparando detalle: %s\n", mysql_stmt_error(stmt_det));
            mysql_query(conn, "ROLLBACK");
            return false;
        }

        MYSQL_BIND params_det[3] = {0};
        params_det[0].buffer_type = MYSQL_TYPE_STRING;
        params_det[0].buffer = det->id_producto;
        params_det[0].buffer_length = strlen(det->id_producto);
        
        params_det[1].buffer_type = MYSQL_TYPE_LONG;
        params_det[1].buffer = &det->cantidad;
        
        params_det[2].buffer_type = MYSQL_TYPE_DOUBLE;
        params_det[2].buffer = &det->precio_negociado;

        if (mysql_stmt_bind_param(stmt_det, params_det) || mysql_stmt_execute(stmt_det)) {
            fprintf(stderr, "Error insertando detalle: %s\n", mysql_stmt_error(stmt_det));
            mysql_stmt_close(stmt_det);
            mysql_query(conn, "ROLLBACK");
            return false;
        }
        mysql_stmt_close(stmt_det);
        det = det->siguiente;
    }

    // 7. Confirmar transacción
    if (mysql_query(conn, "COMMIT")) {
        fprintf(stderr, "Error en COMMIT: %s\n", mysql_error(conn));
        mysql_query(conn, "ROLLBACK");
        return false;
    }
    
    mysql_autocommit(conn, 1);  // Reactivar autocommit
    return true;
}

/**
 * @brief Muestra la cotización actual
 */
void mostrar_cotizacion(Cotizacion *cot) {
    printf("\n=== COTIZACIÓN [%s] ===\n", cot->numero_cotizacion);
    printf("Cliente: %s\nEstado: %s\n", cot->cliente, cot->estado);
    printf("\n%-4s %-20s %-8s %-12s\n", "#", "Producto", "Cant.", "P.Unit.");

    DetalleCotizacion *det = cot->detalles;
    int contador = 1;
    while(det) {
        printf("%-4d %-20s %-8d $%-10.2f\n", 
              contador++, det->nombre, det->cantidad, det->precio_negociado);
        det = det->siguiente;
    }

    printf("\nSUBTOTAL: $%.2f\n", cot->subtotal);
    printf("TOTAL:    $%.2f\n", cot->total);
}

void eliminar_detalle(Cotizacion *cotizacion) {
    if (!cotizacion->detalles) {
        printf("No hay productos para eliminar!\n");
        return;
    }

    mostrar_cotizacion(cotizacion);  // Mostrar numeración actual
    printf("Número de línea a eliminar: ");
    int num_linea;
    scanf("%d", &num_linea);

    DetalleCotizacion *actual = cotizacion->detalles;
    DetalleCotizacion *anterior = NULL;
    int contador = 1;

    while(actual && contador < num_linea) {
        anterior = actual;
        actual = actual->siguiente;
        contador++;
    }

    if(!actual) {
        printf("Línea inválida!\n");
        return;
    }

    if(!anterior) cotizacion->detalles = actual->siguiente;
    else anterior->siguiente = actual->siguiente;

    free(actual);
    actualizar_totales(cotizacion);  // Actualizar totales
    printf("Línea eliminada. Total actual: $%.2f\n", cotizacion->total);
}


/// Función para mostrar el catálogo de productos filtrado por familia
/// y permitir al usuario seleccionar un producto y cantidad para agregar a la cotización.

void mostrar_catalogo(MYSQL* conn, const char* familia) {
    char familia_escapada[100] = {0};
    mysql_real_escape_string(conn, familia_escapada, familia, strlen(familia));

    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    const char *query = 
        "SELECT p.id_producto, p.nombre, f.descripcion, p.precio, p.stock "
        "FROM productos p "
        "JOIN familias f ON p.familia_id = f.id_familia "
        "WHERE f.descripcion = ? OR ? = ''";

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "Error preparando consulta: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    MYSQL_BIND params[2];
    memset(params, 0, sizeof(params));
    params[0].buffer_type = MYSQL_TYPE_STRING;
    params[0].buffer = (char*)familia_escapada;
    params[0].buffer_length = strlen(familia_escapada);
    params[1] = params[0];  // Mismo valor para ambos parámetros

    if (mysql_stmt_bind_param(stmt, params)) {
        fprintf(stderr, "Error vinculando parámetros: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "Error ejecutando consulta: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return;
    }

    // Configurar bind de resultados
    char id_producto[20], nombre[50], descripcion[100];
    double precio;
    int stock;
    
    MYSQL_BIND bind[5];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = id_producto;
    bind[0].buffer_length = sizeof(id_producto);
    
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = nombre;
    bind[1].buffer_length = sizeof(nombre);
    
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = descripcion;
    bind[2].buffer_length = sizeof(descripcion);
    
    bind[3].buffer_type = MYSQL_TYPE_DOUBLE;
    bind[3].buffer = &precio;
    
    bind[4].buffer_type = MYSQL_TYPE_LONG;
    bind[4].buffer = &stock;

    mysql_stmt_bind_result(stmt, bind);

    printf("\n=== PRODUCTOS DISPONIBLES ===\n");
    printf("%-6s %-20s %-30s %-10s %-6s\n", "ID", "Nombre", "Descripción", "Precio", "Stock");

    while(mysql_stmt_fetch(stmt) == 0) {
        printf("%-6s %-20s %-30s $%-9.2f %-6d\n", 
              id_producto, nombre, descripcion, precio, stock);
    }

    mysql_stmt_close(stmt);
}


//

bool actualizar_cotizacion(MYSQL* conn, Cotizacion *cot) {
    mysql_autocommit(conn, 0);
    if(mysql_query(conn, "START TRANSACTION")) {
        fprintf(stderr, "Error iniciando transacción: %s\n", mysql_error(conn));
        return false;
    }

    // Actualizar cabecera
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    const char *query = 
        "UPDATE cotizaciones SET subtotal = ? "
        "WHERE id_cotizacion = ?";
    
    mysql_stmt_prepare(stmt, query, strlen(query));
    
    MYSQL_BIND params[2];
    memset(params, 0, sizeof(params));
    
    params[0].buffer_type = MYSQL_TYPE_DOUBLE;
    params[0].buffer = &cot->subtotal;
    
    params[1].buffer_type = MYSQL_TYPE_LONG;
    params[1].buffer = &cot->id_cotizacion;
    
    mysql_stmt_bind_param(stmt, params);
    
    if(mysql_stmt_execute(stmt)) {
        mysql_stmt_close(stmt);
        mysql_query(conn, "ROLLBACK");
        return false;
    }
    mysql_stmt_close(stmt);

    // Eliminar detalles antiguos
    char delete_query[100];
    sprintf(delete_query, "DELETE FROM detalle_cotizacion WHERE cotizacion_id = %d", cot->id_cotizacion);
    if(mysql_query(conn, delete_query)) {
        mysql_query(conn, "ROLLBACK");
        return false;
    }

    // Insertar nuevos detalles
    DetalleCotizacion *det = cot->detalles;
    while(det) {
        MYSQL_STMT *stmt_det = mysql_stmt_init(conn);
        const char *query_det = 
            "INSERT INTO detalle_cotizacion (cotizacion_id, producto_id, cantidad, precio_negociado) "
            "VALUES (?, ?, ?, ?)";
        
        mysql_stmt_prepare(stmt_det, query_det, strlen(query_det));
        
        MYSQL_BIND params_det[4];
        memset(params_det, 0, sizeof(params_det));
        
        params_det[0].buffer_type = MYSQL_TYPE_LONG;
        params_det[0].buffer = &cot->id_cotizacion;
        
        params_det[1].buffer_type = MYSQL_TYPE_STRING;
        params_det[1].buffer = det->id_producto;
        params_det[1].buffer_length = strlen(det->id_producto);
        
        params_det[2].buffer_type = MYSQL_TYPE_LONG;
        params_det[2].buffer = &det->cantidad;
        
        params_det[3].buffer_type = MYSQL_TYPE_DOUBLE;
        params_det[3].buffer = &det->precio_negociado;
        
        if(mysql_stmt_bind_param(stmt_det, params_det) || mysql_stmt_execute(stmt_det)) {
            mysql_stmt_close(stmt_det);
            mysql_query(conn, "ROLLBACK");
            return false;
        }
        mysql_stmt_close(stmt_det);
        det = det->siguiente;
    }

    mysql_query(conn, "COMMIT");
    mysql_autocommit(conn, 1);
    return true;
}
//
void modificar_cotizacion(MYSQL* conn) {
    char numero_cotizacion[20];
    printf("\nIngrese número de cotización: ");
    scanf("%19s", numero_cotizacion);

    // Cargar cotización existente
    Cotizacion *cot = cargar_cotizacion(conn, numero_cotizacion);
    if(!cot) {
        printf("Cotización no encontrada o ya fue facturada\n");
        return;
    }

    int opcion;
    do {
        printf("\n=== MODIFICANDO COTIZACIÓN %s ===", numero_cotizacion);
        printf("\n1. Agregar producto");
        printf("\n2. Eliminar producto");
        printf("\n3. Ver cotización actual");
        printf("\n4. Guardar cambios");
        printf("\n5. Salir sin guardar");
        printf("\nSeleccione: ");
        scanf("%d", &opcion);

        switch(opcion) {
            case 1: agregar_detalle(cot, conn); break;
            case 2: eliminar_detalle(cot); break;
            case 3: mostrar_cotizacion(cot); break;
            case 4: 
                if(actualizar_cotizacion(conn, cot)) {
                    printf("Cambios guardados exitosamente!\n");
                }
                free(cot);
                return;
            case 5: 
                free(cot);
                return;
            default: printf("Opción inválida\n");
        }
    } while(1);
}

Cotizacion* cargar_cotizacion(MYSQL* conn, const char* numero_cotizacion) {
    Cotizacion *cot = nueva_cotizacion();
    
    // Buscar cabecera
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    const char *query = 
        "SELECT id_cotizacion, cliente, subtotal, estado "
        "FROM cotizaciones "
        "WHERE numero_cotizacion = ? AND estado = 'PENDIENTE'";
    
    mysql_stmt_prepare(stmt, query, strlen(query));
    
    MYSQL_BIND param = {0};
    param.buffer_type = MYSQL_TYPE_STRING;
    param.buffer = (char*)numero_cotizacion;
    param.buffer_length = strlen(numero_cotizacion);
    mysql_stmt_bind_param(stmt, &param);
    
    mysql_stmt_execute(stmt);
    
    MYSQL_BIND result[4];
    memset(result, 0, sizeof(result));
    
    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &cot->id_cotizacion;
    
    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = cot->cliente;
    result[1].buffer_length = sizeof(cot->cliente);
    
    result[2].buffer_type = MYSQL_TYPE_DOUBLE;
    result[2].buffer = &cot->subtotal;
    
    result[3].buffer_type = MYSQL_TYPE_STRING;
    result[3].buffer = cot->estado;
    result[3].buffer_length = sizeof(cot->estado);
    
    mysql_stmt_bind_result(stmt, result);
    
    if(mysql_stmt_fetch(stmt) != 0) {
        free(cot);
        cot = NULL;
    }
    mysql_stmt_close(stmt);
    
    if(!cot) return NULL;

    // Cargar detalles
    stmt = mysql_stmt_init(conn);
    query = 
        "SELECT producto_id, cantidad, precio_negociado "
        "FROM detalle_cotizacion "
        "WHERE cotizacion_id = ?";
    
    mysql_stmt_prepare(stmt, query, strlen(query));
    
    MYSQL_BIND det_param = {0};
    det_param.buffer_type = MYSQL_TYPE_LONG;
    det_param.buffer = &cot->id_cotizacion;
    mysql_stmt_bind_param(stmt, &det_param);
    
    mysql_stmt_execute(stmt);
    
    MYSQL_BIND det_result[3];
    DetalleCotizacion *ultimo = NULL;
    
    while(mysql_stmt_fetch(stmt) == 0) {
        DetalleCotizacion *nuevo = malloc(sizeof(DetalleCotizacion));
        
        memset(det_result, 0, sizeof(det_result));
        det_result[0].buffer_type = MYSQL_TYPE_STRING;
        det_result[0].buffer = nuevo->id_producto;
        det_result[0].buffer_length = sizeof(nuevo->id_producto);
        
        det_result[1].buffer_type = MYSQL_TYPE_LONG;
        det_result[1].buffer = &nuevo->cantidad;
        
        det_result[2].buffer_type = MYSQL_TYPE_DOUBLE;
        det_result[2].buffer = &nuevo->precio_negociado;
        
        mysql_stmt_bind_result(stmt, det_result);
        mysql_stmt_fetch(stmt);
        
        nuevo->siguiente = NULL;
        
        if(!cot->detalles) {
            cot->detalles = nuevo;
        } else {
            ultimo->siguiente = nuevo;
        }
        ultimo = nuevo;
    }
    mysql_stmt_close(stmt);
    
    actualizar_totales(cot);
    return cot;
}

