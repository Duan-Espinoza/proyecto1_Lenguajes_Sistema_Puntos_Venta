#ifndef ADMIN_H
#define ADMIN_H

#include "../models/database.h"

int autenticar_admin(MYSQL* conn);
void menu_administrativo(MYSQL* conn);

#endif