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
#include <filesystem>
#include <fstream>
#include "CommandLineParser.h"
#include<vector>
#include <algorithm>
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
    //variables utilizadas a lo largo del programa
    char *databaseFile = "index.db";
    sqlite3 *database;
    char *databaseErrorMessage;
    
    string tableName;//nombre de nuestra tabla
    tableName = "myTable";
    string auxstr;

   //parseando linea de comando
    CommandLineParser parser(argc, argv);
    string wwwPath = "/home/francob/Desktop/eda/EDAOOGLE/www/wiki";


    // // Parse command line
    // if (!parser.hasOption("-h"))
    // {
    //     cout << "error: WWW_PATH must be specified." << endl;

    //     return 1;
    // }
   
    // wwwPath = parser.getOption("-h");
    
    // //test
    std :: cout << "path:"<<wwwPath<<std :: endl ;

    



    
    // Abriendo base de datos
    cout << "Opening database..." << endl;
    if (sqlite3_open(databaseFile, &database) != SQLITE_OK)
    {
        cout << "Can't open database: " << sqlite3_errmsg(database) << endl;

        return 1;
    }

    // creando tabla
    auxstr = "CREATE VIRTUAL TABLE " + tableName + " USING fts5(title,url,body);";
    cout << "Creating table..." << endl;
    if (sqlite3_exec(database,
                     auxstr.c_str(),
                     NULL,
                     0,
                     &databaseErrorMessage) != SQLITE_OK){
                         cout << "Error: " << sqlite3_errmsg(database) << endl;
                     }
       

    //borrando informacion previa si existia
    auxstr = "DELETE FROM " + tableName;
    cout << "Deleting previous entries..." << endl;
    if (sqlite3_exec(database,
                     auxstr.c_str(),
                     NULL,
                     0,
                     &databaseErrorMessage) != SQLITE_OK){
                        cout << "Error: " << sqlite3_errmsg(database) << endl;
                     }
        

    //Poblando nuestra tabla
    cout << "Inserting entries..." << endl;
   
   
    ifstream visitedFile;
    string nameOfVisitedFile;
    filesystem :: path path2Folder{wwwPath.c_str()};
    vector<std :: filesystem :: path> v;
    for (const auto & it : filesystem::directory_iterator(path2Folder)){
        v.push_back(it);
    }

    sort(v.begin(),v.end());
    int counter =0;
    for(auto & it : v){
        counter++;
        cout << it.string() << " " << counter;

    }
   





    
    auxstr = "INSERT INTO " + tableName + " (title,url,body) VALUES\n";
    
    if (sqlite3_exec(database,
                     "INSERT INTO mytable (title, url, body) VALUES "
                     "('goku','hhhtppf.','esto es un tecto');",
                     NULL,
                     0,
                     &databaseErrorMessage) != SQLITE_OK){
                         cout << "Error: " << sqlite3_errmsg(database) << endl;
                     }

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
