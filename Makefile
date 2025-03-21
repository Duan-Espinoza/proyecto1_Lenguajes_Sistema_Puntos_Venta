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
    $(CC) $(CFLAGS) $(MYSQL_INCLUDE) $^ -o $@ $(LDFLAGS)

%.o: %.c
    $(CC) $(CFLAGS) $(MYSQL_INCLUDE) -c $< -o $@

clean:
    del /Q $(OBJ) $(EXEC)

.PHONY: all clean