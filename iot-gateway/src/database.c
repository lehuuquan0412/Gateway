#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "database.h"

extern int fifo_fd[2];

sqlite3 *db;

static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    int i;
    for(i = 0; i<argc; i++) 
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}


int database_initialize(const char *db_name)
{
    int num_write;
    char *zErrMsg = 0;
    int rc;
    char *sql;
    //const char* data = "Callback function called";
    char log_buffer[] = "Connect to SQL server established\n";
    char log_buffer_table[] = "New table DATA SENSOR created\n";

    /* Open database */
    rc = sqlite3_open(db_name, &db);
    
    if( rc ) {
        //fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return -1;
    }else {
        num_write = write(fifo_fd[1], log_buffer, strlen(log_buffer));
    }

    /* Create SQL statement */
    sql = "CREATE TABLE SENSOR("  \
        "ID             INT     NOT NULL," \
        "DATA           CHAR(5)    NOT NULL);";

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        printf("Error\n");
        return -1;
    } else {
        //fprintf(stdout, "Sensor data table created successfully\n");
        num_write = write(fifo_fd[1], log_buffer_table, strlen(log_buffer_table));
        return SQLITE_OK;
    }
}

int insert_value(int id, char *data)
{
    char *zErrMsg = 0;
    int rc;
    char sql[50];

    sprintf(sql, "INSERT INTO SENSOR(ID,DATA) VALUES (%d,'%s')", id, data);
    rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
    } else {
        return SQLITE_OK;
    } 
}

void close_database(void)
{
    sqlite3_close(db);
}