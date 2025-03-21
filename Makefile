CC = gcc
CFLAGS = -I"src/models" -I"src/controllers" -I"src/utils"
LDFLAGS = -L"C:/Program Files/MySQL/MySQL Server 8.0/lib" -lmysql
MYSQL_INCLUDE = -I"C:/Program Files/MySQL/MySQL Server 8.0/include"

SRC = src/main.c \
      src/controllers/admin.c \
      src/controllers/productos.c \
      src/models/database.c \
      src/utils/file_manager.c

OBJ = $(SRC:.c=.o)
EXEC = sistema_ventas.exe

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $(MYSQL_INCLUDE) $(OBJ) -o $@ $(LDFLAGS)

# Reglas para compilar archivos .c a .o en subdirectorios
src/%.o: src/%.c
	$(CC) $(CFLAGS) $(MYSQL_INCLUDE) -c $< -o $@

src/controllers/%.o: src/controllers/%.c
	$(CC) $(CFLAGS) $(MYSQL_INCLUDE) -c $< -o $@

src/models/%.o: src/models/%.c
	$(CC) $(CFLAGS) $(MYSQL_INCLUDE) -c $< -o $@

src/utils/%.o: src/utils/%.c
	$(CC) $(CFLAGS) $(MYSQL_INCLUDE) -c $< -o $@

clean:
	cmd /c del /Q /F $(subst /,\,$(OBJ)) $(EXEC)

.PHONY: all clean
