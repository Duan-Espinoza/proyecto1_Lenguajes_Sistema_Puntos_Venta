#include "../models/productos.h"
#include "../models/database.h"
#include <stdio.h>
#include <string.h>

// Implementación de registro de familias
void registrar_familia(MYSQL* conn) {
    char descripcion[50];
    printf("\n--- REGISTRAR FAMILIA ---\n");
    printf("Ingrese la descripción: ");
    scanf(" %49[^\n]", descripcion);

    char query[200];
    sprintf(query, "INSERT INTO familias (descripcion) VALUES ('%s')", descripcion);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error al registrar: %s\n", mysql_error(conn));
    } else {
        printf("¡Familia '%s' registrada!\n", descripcion);
    }
}

// Implementación de registro de productos
void registrar_producto(MYSQL* conn) {
    char nombre[50];
    int familia_id;
    float costo, precio;
    int stock;

    printf("\n--- REGISTRAR PRODUCTO ---\n");
    printf("Nombre: ");
    scanf(" %49[^\n]", nombre);
    printf("ID Familia: ");
    if(scanf("%d", &familia_id) != 1) {
        printf("Error: ID de familia inválido\n");
        while(getchar() != '\n');  // Limpiar buffer
        return;
    }
    printf("Costo: ");
    scanf("%f", &costo);
    printf("Precio: ");
    scanf("%f", &precio);
    printf("Stock: ");
    scanf("%d", &stock);

    char query[300];
    sprintf(query, 
        "INSERT INTO productos (nombre, familia_id, costo, precio, stock) "
        "VALUES ('%s', %d, %.2f, %.2f, %d)",
        nombre, familia_id, costo, precio, stock
    );

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error: %s\n", mysql_error(conn));
    } else {
        printf("¡Producto '%s' registrado!\n", nombre);
    }
}