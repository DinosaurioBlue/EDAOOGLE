/**
 * @file mkindex.cpp
 * @author Marc S. Ressl
 * @brief Makes a database index
 * @version 0.3
 *
 * @copyright Copyright (c) 2022-2024 Marc S. Ressl
 */

#include <iostream>
#include <string>

#include <sqlite3.h>

using namespace std;

static int onDatabaseEntry(void *userdata,
                           int argc,
                           char **argv,
                           char **azColName)
{
    cout << "--- Entry" << endl;
    for (int i = 0; i < argc; i++)
    {
        if (argv[i])
            cout << azColName[i] << ": " << argv[i] << endl;
        else
            cout << azColName[i] << ": " << "NULL" << endl;
    }

    return 0;
}
/*****************************************************MAIN************************** */
int main(int argc,
         const char *argv[])
{
    char *databaseFile = "index.db";
    sqlite3 *database;
    char *databaseErrorMessage;
    string name;
    name = "myTable";
    string auxstr = "CREATE VIRTUAL TABLE " + name + " USING fts5(title,url, body);";



    
    // Open database file
    cout << "Opening database..." << endl;
    if (sqlite3_open(databaseFile, &database) != SQLITE_OK)
    {
        cout << "Can't open database: " << sqlite3_errmsg(database) << endl;

        return 1;
    }

    // Create a sample table
    cout << "Creating table..." << endl;
    if (sqlite3_exec(database,
                     auxstr.c_str(),
                     NULL,
                     0,
                     &databaseErrorMessage) != SQLITE_OK){
                         cout << "Error: " << sqlite3_errmsg(database) << endl;
                     }
       

    //Delete previous entries if table already existed
    cout << "Deleting previous entries..." << endl;
    auxstr = "DELETE FROM " + name;
    if (sqlite3_exec(database,
                     auxstr.c_str(),
                     NULL,
                     0,
                     &databaseErrorMessage) != SQLITE_OK){
                        cout << "Error: " << sqlite3_errmsg(database) << endl;
                     }
        

    // // Create sample entries


    auxstr = "INSERT INTO " + name + " (title,url,body) VALUES\n";
    cout << "Creating sample entries..." << endl;
    if (sqlite3_exec(database,
                     "INSERT INTO mytable (title, url, body) VALUES "
                     "('goku','hhhtppf.','esto es un tecto');",
                     NULL,
                     0,
                     &databaseErrorMessage) != SQLITE_OK){
                         cout << "Error: " << sqlite3_errmsg(database) << endl;
                     }
    // cout << "Creating sample entries..." << endl;
    // if (sqlite3_exec(database,
                    //  "INSERT INTO room_occupation (room, reserved_from, reserved_until) VALUES "
                    //  "('401F','2022-03-03 15:00:00','2022-03-03 16:00:00');",
                     NULL,
    //                  0,
    //                  &databaseErrorMessage) != SQLITE_OK)
    //     cout << "Error: " << sqlite3_errmsg(database) << endl;
    // if (sqlite3_exec(database,
    //                  "INSERT INTO room_occupation (room, reserved_from, reserved_until) VALUES "
    //                  "('501F','2022-03-03 15:00:00','2022-03-03 17:00:00');",
    //                  NULL,
    //                  0,
    //                  &databaseErrorMessage) != SQLITE_OK)
    //     cout << "Error: " << sqlite3_errmsg(database) << endl;
    // if (sqlite3_exec(database,
    //                  "INSERT INTO room_occupation (room, reserved_from, reserved_until) VALUES "
    //                  "('001R','2022-03-03 15:00:00','2022-03-03 16:30:00');",
    //                  NULL,
    //                  0,
    //                  &databaseErrorMessage) != SQLITE_OK)
    //     cout << "Error: " << sqlite3_errmsg(database) << endl;
    // if (sqlite3_exec(database,
    //                  "INSERT INTO room_occupation (room, reserved_from, reserved_until) VALUES "
    //                  "('1001F','2022-03-03 15:30:00','2022-03-03 16:30:00');",
    //                  NULL,
    //                  0,
    //                  &databaseErrorMessage) != SQLITE_OK)
    //     cout << "Error: " << sqlite3_errmsg(database) << endl;

    // Fetch entries
    cout << "Fetching entries..." << endl;
    if (sqlite3_exec(database,
                     "SELECT * from myTable;",
                     onDatabaseEntry,
                     0,
                     &databaseErrorMessage) != SQLITE_OK)
        cout << "Error: " << sqlite3_errmsg(database) << endl;

    // Close database
    cout << "Closing database..." << endl;
    sqlite3_close(database);
}
