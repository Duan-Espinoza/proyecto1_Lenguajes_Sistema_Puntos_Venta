CC = gcc
CFLAGS = -I"src/models" 
LDFLAGS = -L"C:/Program Files/MySQL/MySQL Server 8.0/lib" -lmysql
MYSQL_INCLUDE = -I"C:/Program Files/MySQL/MySQL Server 8.0/include"

SRC = src/main.c \
      src/controllers/admin.c \
      src/controllers/productos.c \
      src/controllers/database.c \  # Ruta actualizada
      src/utils/file_manager.c

OBJ = $(SRC:.c=.o)
EXEC = sistema_ventas.exe

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $(MYSQL_INCLUDE) $(OBJ) -o $@ $(LDFLAGS)

# Reglas de compilación optimizadas
src/controllers/%.o: src/controllers/%.c
	$(CC) $(CFLAGS) $(MYSQL_INCLUDE) -c $< -o $@

src/utils/%.o: src/utils/%.c
	$(CC) $(CFLAGS) $(MYSQL_INCLUDE) -c $< -o $@

src/%.o: src/%.c
	$(CC) $(CFLAGS) $(MYSQL_INCLUDE) -c $< -o $@

clean:
	@if exist $(EXEC) del /F $(EXEC)
	@for %%f in ($(subst /,\,$(OBJ))) do @if exist %%f del /F %%f

.PHONY: all clean