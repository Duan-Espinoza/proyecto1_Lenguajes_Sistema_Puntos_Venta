CC = gcc
CFLAGS = -I"src/models" 
LDFLAGS = -L"C:/Program Files/MySQL/MySQL Server 8.0/lib" -lmysql
MYSQL_INCLUDE = -I"C:/Program Files/MySQL/MySQL Server 8.0/include"

SRC = src/main.c \
      src/controllers/admin.c \
      src/controllers/database.c \
      src/controllers/productos.c \
      src/utils/file_manager.c

OBJ = $(SRC:.c=.o)
EXEC = sistema_ventas.exe

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) $(MYSQL_INCLUDE) $(OBJ) -o $@ $(LDFLAGS)

clean:
	@if exist $(EXEC) del /F $(EXEC)
	@for %%f in ($(subst /,\,$(OBJ))) do @if exist %%f del /F %%f

.PHONY: all clean