/**
 * @file mkindex.cpp
 * @brief Makes a database index with URLs
 */

#ifndef SQLITE_ENABLE_FTS5
#define SQLITE_ENABLE_FTS5
#endif

#include <iostream>
#include <string>
#include <sqlite3.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <regex>
#include "CommandLineParser.h"

using namespace std;

static int onDatabaseEntry(void *userdata, int argc, char **argv, char **azColName)
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

string stripHTMLTags(string &html)
{
    static const regex tag_re("<[^>]*>", regex::optimize);
    return regex_replace(html, tag_re, "");
}

int main(int argc, const char *argv[])
{
    sqlite3 *db = nullptr;
    char *errorMessage = nullptr;

    // Abrimos la base de datos
    if (sqlite3_open("../index.db", &db) != SQLITE_OK)
    {
        std::cout << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    // Eliminamos la tabla existente y la recreamos
    const char *drop_table = "DROP TABLE IF EXISTS paginas_web;";
    if (sqlite3_exec(db, drop_table, nullptr, nullptr, &errorMessage) != SQLITE_OK)
    {
        std::cout << "Error dropping table: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
        sqlite3_close(db);
        return 1;
    }
    std::cout << "Previous table deleted if existed" << std::endl;

    // Creamos la tabla con FTS5 y agregamos la columna `url`
    const char *create_table = "CREATE VIRTUAL TABLE paginas_web USING fts5(titulo, url, contenido);";
    if (sqlite3_exec(db, create_table, nullptr, nullptr, &errorMessage) != SQLITE_OK)
    {
        std::cout << "Error creating table: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
        sqlite3_close(db);
        return 1;
    }
    std::cout << "Table created successfully" << std::endl;

    // Borramos entradas previas
    cout << "Deleting previous entries..." << endl;
    if (sqlite3_exec(db, "DELETE FROM paginas_web;", nullptr, nullptr, &errorMessage) != SQLITE_OK)
        cout << "Error: " << sqlite3_errmsg(db) << endl;

    // AQUI COMIENZA LA PARTE DE INSERCIÓN

    CommandLineParser parser(argc, argv);

    // Parse command line
    if (!parser.hasOption("-h"))
    {
        cout << "error: WWW_PATH must be specified." << endl;
        return 1;
    }

    string wwwPath = parser.getOption("-h");
    string wikiPath = wwwPath + "/wiki";

    filesystem::path wiki = wikiPath;

    // Chequeamos si la carpeta existe
    if (!filesystem::exists(wiki))
    {
        cerr << "Wiki folder does not exist!";
        return 1;
    }

    // Preparamos el template de inserción
    sqlite3_stmt *stmt = nullptr;
    const char *insert_sql = "INSERT INTO paginas_web (titulo, url, contenido) VALUES (?, ?, ?);";

    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cout << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return 1;
    }

    int i = 0;
    // Iteramos sobre los archivos
    for (const auto &entry : filesystem::directory_iterator(wiki))
    {
        // Chequeamos si tienen extension html
        if (entry.is_regular_file() && entry.path().extension() == ".html")
        {
            std::ifstream file(entry.path());
            // Leemos al contenido del archivo a un string
            std::string HTMLContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            // Quitamos etiquetas HTML para extraer el contenido de texto
            string strippedHTML = stripHTMLTags(HTMLContent);

            // Extraemos el título y creamos la URL
            const char *title = entry.path().stem().c_str();          // Pone como titulo el nombre del archivo
            string url = "/wiki/" + entry.path().filename().string(); // URL relativa basada en la ruta del archivo
            const char *content = strippedHTML.c_str();

            // Pre cargamos los comandos y declaraciones para insertar (se guardan en stmt)
            sqlite3_bind_text(stmt, 1, title, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, url.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, content, -1, SQLITE_TRANSIENT);

            // Ejecutamos
            if (sqlite3_step(stmt) != SQLITE_DONE)
            {
                std::cout << "Error inserting data: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_reset(stmt);
            }
            else
            {
                std::cout << "Data inserted successfully" << std::endl;
                sqlite3_reset(stmt);
            }

            cout << "Processed: " << entry.path().filename() << '\n';
            i++;
        }
    }
    cout << "Tabla completada con " << i << " entradas." << endl;

    // Cerramos la base de datos
    cout << "Closing database..." << endl;
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}
