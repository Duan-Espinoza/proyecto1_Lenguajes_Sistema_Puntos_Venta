CC = gcc
CFLAGS = -I"src/models" -I"C:/Program Files/MySQL/MySQL Server 8.0/include"
LDFLAGS = -L"C:/Program Files/MySQL/MySQL Server 8.0/lib" -lmysql

SRC = src/main.c \
      src/controllers/admin.c \
      src/controllers/database.c \
      src/controllers/productos.c \
      src/utils/file_manager.c

OBJ = $(SRC:.c=.o)
EXEC = sistema_ventas.exe

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@if exist $(EXEC) del /F /Q $(EXEC)
	@if exist src\*.o del /F /Q src\*.o
	@if exist src\controllers\*.o del /F /Q src\controllers\*.o
	@if exist src\utils\*.o del /F /Q src\utils\*.o

.PHONY: all clean