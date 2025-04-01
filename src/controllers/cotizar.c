#include "../models/cotizacion.h"
#include "../models/database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>



/**
 * @brief Crea una nueva cotización y la inicializa.
 * @return Puntero a la nueva cotización creada.
 */
Cotizacion* nueva_cotizacion() {
    // Asigna memoria para la nueva cotización
    Cotizacion *cotizacion = malloc(sizeof(Cotizacion));
    if(cotizacion == NULL) {
        fprintf(stderr, "Error al asignar memoria para la cotización\n");
        exit(1);
    }
    
    // Inicializa los campos de la estructura
    cotizacion->id = 0;  // Se asigna 0 o el valor que consideres predeterminado
    strcpy(cotizacion->numero_cotizacion, ""); // Inicialmente vacío, se llenará al guardar
    strcpy(cotizacion->fecha, "");             // Se puede asignar la fecha al momento de guardar
    strcpy(cotizacion->estado, "PENDIENTE");     // Estado inicial
    strcpy(cotizacion->cliente, "");           // El cliente se asignará luego, según entrada del usuario
    cotizacion->subtotal = 0.0;                // Inicialmente sin subtotal
    cotizacion->total = 0.0;                   // Inicialmente sin total
    cotizacion->detalles = NULL;               // La lista de detalles está vacía al comienzo

    return cotizacion;
}



/**
 * @brief Muestra el menú de cotización y permite al usuario interactuar con él.
 * @param conn Conexión a la base de datos MySQL.
 */
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
            case 1: 
                agregar_detalle(cotizacion_actual, conn); 
                break;
            case 2: 
                // Aún por implementar: función eliminar_detalle(cotizacion_actual);
                printf("Funcionalidad de eliminar producto por implementar.\n");
                break;
            case 3: 
                mostrar_cotizacion(cotizacion_actual); 
                break;
            case 4: 
                if (guardar_cotizacion(conn, cotizacion_actual)) {
                    printf("Cotización guardada exitosamente.\n");
                } else {
                    printf("Error al guardar la cotización.\n");
                }
                return;
            case 5: 
                free(cotizacion_actual);
                return;
            default: 
                printf("Opción inválida\n");
        }
    } while(1);
}

/**
 * @brief Crea una nueva cotización y la inicializa.
 * @return Puntero a la nueva cotización creada.
 */
void agregar_detalle(Cotizacion *cotizacion, MYSQL* conn) {
    // Paso 1: Mostrar catálogo filtrado
    char familia[50] = "";
    printf("\nFiltrar por familia (dejar vacío para todas): ");
    scanf(" %49[^\n]", familia);
    
    char query[512];
    snprintf(query, sizeof(query),
        "SELECT p.id_producto, p.nombre, f.descripcion, p.precio, p.stock "
        "FROM productos p "
        "JOIN familias f ON p.familia_id = f.id_familia %s",
        (strlen(familia) > 0 ? "WHERE f.descripcion = ?" : ""));
    
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    mysql_stmt_prepare(stmt, query, strlen(query));
    //
    // Ejecuta la consulta para obtener el catálogo
    mysql_stmt_execute(stmt);

    // Preparar para recibir los resultados
    MYSQL_RES *result = mysql_stmt_result_metadata(stmt);
    if(result) {
        printf("\n=== PRODUCTOS DISPONIBLES ===\n");
        printf("%-5s %-20s %-15s %-10s\n", "ID", "Nombre", "Familia", "Precio");

        // Definir buffers para los resultados
        MYSQL_BIND bind[5];
        memset(bind, 0, sizeof(bind));
        char id[20], nombre[50], familia[50];
        double precio;
        int stock;  // Opcional si deseas mostrar stock

        // Configurar cada bind
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = id;
        bind[0].buffer_length = sizeof(id);

        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = nombre;
        bind[1].buffer_length = sizeof(nombre);

        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = familia;
        bind[2].buffer_length = sizeof(familia);

        bind[3].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[3].buffer = &precio;

        bind[4].buffer_type = MYSQL_TYPE_LONG;
        bind[4].buffer = &stock;

        mysql_stmt_bind_result(stmt, bind);

        // Recorrer los resultados e imprimirlos
        while(mysql_stmt_fetch(stmt) == 0) {
            printf("%-5s %-20s %-15s $%-9.2f\n", id, nombre, familia, precio);
        }
        mysql_free_result(result);
    }

    //

    // Vincular parámetro de familia si se proporcionó
    if(strlen(familia) > 0) {
        MYSQL_BIND param = {0};
        param.buffer_type = MYSQL_TYPE_STRING;
        param.buffer = familia;
        param.buffer_length = strlen(familia);
        mysql_stmt_bind_param(stmt, &param);
    }
    
    // Ejecuta y muestra los resultados (este paso lo debes implementar)
    // Por ejemplo, recorrer el resultado y mostrar cada producto disponible.
    
    mysql_stmt_close(stmt); // Cerrar el statement después de usarlo
    
    // Paso 2: Seleccionar producto y cantidad
    char id_producto[20];
    int cantidad;
    printf("\nIngrese ID del producto: ");
    scanf("%19s", id_producto);
    printf("Ingrese cantidad: ");
    scanf("%d", &cantidad);
    
    // Validar stock en tiempo real (Ejemplo simplificado)
    int stock_disponible = 0;
    MYSQL_STMT *stmt_stock = mysql_stmt_init(conn);
    const char *stock_query = "SELECT stock, precio FROM productos WHERE id_producto = ?";
    mysql_stmt_prepare(stmt_stock, stock_query, strlen(stock_query));
    
    MYSQL_BIND param_stock = {0};
    param_stock.buffer_type = MYSQL_TYPE_STRING;
    param_stock.buffer = id_producto;
    param_stock.buffer_length = strlen(id_producto);
    mysql_stmt_bind_param(stmt_stock, &param_stock);
    
    mysql_stmt_execute(stmt_stock);
    //MYSQL_RES *result = mysql_stmt_result_metadata(stmt_stock);
    result = mysql_stmt_result_metadata(stmt_stock);
    if(result) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if(row) {
            stock_disponible = atoi(row[0]);
            // Puedes también obtener el precio y almacenarlo en una variable.
        }
        mysql_free_result(result);
    }
    mysql_stmt_close(stmt_stock);
    
    if(cantidad > stock_disponible) {
        printf("Error: Stock insuficiente (Disponible: %d)\n", stock_disponible);
        return;
    }
    
    // Paso 3: Agregar a la estructura temporal
    DetalleCotizacion *nuevo = malloc(sizeof(DetalleCotizacion));
    strcpy(nuevo->id_producto, id_producto);
    // Aquí podrías buscar y asignar el nombre y el precio unitario si lo deseas.
    nuevo->cantidad = cantidad;
    nuevo->precio_negociado = 0; // Se puede calcular o pedir al usuario.
    nuevo->siguiente = cotizacion->detalles;
    cotizacion->detalles = nuevo;
}




bool guardar_cotizacion(MYSQL* conn, Cotizacion *cotizacion) {
    // Obtener número de secuencia de la configuración
    if (mysql_query(conn, "SELECT secuencia_cotizacion FROM config")) {
        fprintf(stderr, "Error al obtener la secuencia: %s\n", mysql_error(conn));
        return false;
    }
    MYSQL_RES *res = mysql_store_result(conn);
    if(!res) {
        fprintf(stderr, "Error: %s\n", mysql_error(conn));
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    int secuencia = atoi(row[0]);
    mysql_free_result(res);
    
    // Generar número de cotización (ej: 20250001)
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(cotizacion->numero_cotizacion, sizeof(cotizacion->numero_cotizacion),
        "%04d%04d", tm.tm_year + 1900, secuencia);
    
    // Insertar la cotización en la tabla 'cotizaciones'
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    const char *query = "INSERT INTO cotizaciones (numero_cotizacion, cliente, subtotal, estado) VALUES (?, ?, ?, 'PENDIENTE')";
    mysql_stmt_prepare(stmt, query, strlen(query));
    
    MYSQL_BIND params[3] = {0};
    
    // Vincular el número de cotización
    params[0].buffer_type = MYSQL_TYPE_STRING;
    params[0].buffer = cotizacion->numero_cotizacion;
    params[0].buffer_length = strlen(cotizacion->numero_cotizacion);
    
    // Vincular el cliente
    params[1].buffer_type = MYSQL_TYPE_STRING;
    params[1].buffer = cotizacion->cliente;
    params[1].buffer_length = strlen(cotizacion->cliente);
    
    // Vincular el subtotal
    params[2].buffer_type = MYSQL_TYPE_DOUBLE;
    params[2].buffer = &cotizacion->subtotal;
    
    mysql_stmt_bind_param(stmt, params);
    
    if(mysql_stmt_execute(stmt)) {
        fprintf(stderr, "Error al insertar cotización: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        return false;
    }
    mysql_stmt_close(stmt);
    
    // Recuperar el id de la cotización recién insertada (si es necesario)
    int cotizacion_id = mysql_insert_id(conn);
    
    // Insertar cada detalle de la cotización
    DetalleCotizacion *detalle = cotizacion->detalles;
    while(detalle != NULL) {
        MYSQL_STMT *stmt_detalle = mysql_stmt_init(conn);
        const char *query_detalle = "INSERT INTO detalle_cotizacion (cotizacion_id, id_producto, cantidad, precio_negociado) VALUES (?, ?, ?, ?)";
        mysql_stmt_prepare(stmt_detalle, query_detalle, strlen(query_detalle));
        
        MYSQL_BIND params_det[4] = {0};
        int cantidad = detalle->cantidad;
        double precio_negociado = detalle->precio_negociado;
        
        // Vincular parámetros para el detalle
        params_det[0].buffer_type = MYSQL_TYPE_LONG;
        params_det[0].buffer = &cotizacion_id;
        
        params_det[1].buffer_type = MYSQL_TYPE_STRING;
        params_det[1].buffer = detalle->id_producto;
        params_det[1].buffer_length = strlen(detalle->id_producto);
        
        params_det[2].buffer_type = MYSQL_TYPE_LONG;
        params_det[2].buffer = &cantidad;
        
        params_det[3].buffer_type = MYSQL_TYPE_DOUBLE;
        params_det[3].buffer = &precio_negociado;
        
        mysql_stmt_bind_param(stmt_detalle, params_det);
        if(mysql_stmt_execute(stmt_detalle)) {
            fprintf(stderr, "Error al insertar detalle: %s\n", mysql_stmt_error(stmt_detalle));
        }
        mysql_stmt_close(stmt_detalle);
        detalle = detalle->siguiente;
    }
    
    // Actualizar la secuencia de cotización
    char update_seq[100];
    snprintf(update_seq, sizeof(update_seq),
        "UPDATE config SET secuencia_cotizacion = %d", secuencia + 1);
    if(mysql_query(conn, update_seq)) {
        fprintf(stderr, "Error al actualizar secuencia: %s\n", mysql_error(conn));
    }
    
    return true;
}


void mostrar_cotizacion(Cotizacion *cotizacion) {
    printf("\n=== COTIZACIÓN %s ===\n", cotizacion->numero_cotizacion);
    printf("%-5s %-20s %-8s %-12s\n", "Código", "Producto", "Cantidad", "Precio Unit.");
    
    DetalleCotizacion *detalle = cotizacion->detalles;
    while(detalle != NULL) {
        printf("%-5s %-20s %-8d $%-10.2f\n", 
              detalle->id_producto, detalle->nombre, 
              detalle->cantidad, detalle->precio_negociado);
        detalle = detalle->siguiente;
    }
    printf("\nTotal: $%.2f\n", cotizacion->total);
}

void eliminar_detalle(Cotizacion *cotizacion) {
    int num_linea;
    printf("Número de línea a eliminar: ");
    scanf("%d", &num_linea);

    DetalleCotizacion *actual = cotizacion->detalles;
    DetalleCotizacion *anterior = NULL;
    int contador = 1;

    while(actual != NULL && contador != num_linea) {
        anterior = actual;
        actual = actual->siguiente;
        contador++;
    }

    if(actual == NULL) {
        printf("Línea no encontrada\n");
        return;
    }

    if(anterior == NULL) {
        // Si se elimina el primer elemento
        cotizacion->detalles = actual->siguiente;
    } else {
        anterior->siguiente = actual->siguiente;
    }

    free(actual);
    printf("Línea eliminada\n");
}

