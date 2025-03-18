#include <mysql.h>
#include <stdio.h>

int main() {
    printf("Versión de MySQL: %s\n", mysql_get_client_info());
    return 0;
}