#ifndef DATABASE_H__
#define DATABASE_H__

#include <stdio.h>

int database_initialize(const char *db_name);

int insert_value(int id, char *data);
void close_database(void);



#endif