#include <chrono>
#include <sqlite3.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "HttpRequestHandler.h"

using namespace std;

HttpRequestHandler::HttpRequestHandler(string homePath)
{
    this->homePath = homePath;
}

/**
 * @brief Serves a webpage from file
 *
 * @param url The URL
 * @param response The HTTP response
 * @return true URL valid
 * @return false URL invalid
 */
bool HttpRequestHandler::serve(string url, vector<char> &response)
{
    auto homeAbsolutePath = filesystem::absolute(homePath);
    auto relativePath = homeAbsolutePath / url.substr(1);
    string path = filesystem::absolute(relativePath.make_preferred()).string();

    if (path.substr(0, homeAbsolutePath.string().size()) != homeAbsolutePath)
        return false;

    ifstream file(path);
    if (file.fail())
        return false;

    file.seekg(0, ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, ios::beg);

    response.resize(fileSize);
    file.read(response.data(), fileSize);

    return true;
}

bool HttpRequestHandler::handleRequest(string url, HttpArguments arguments, vector<char> &response)
{
    string searchPage = "/search";
    if (url.substr(0, searchPage.size()) == searchPage)
    {
        string searchString = arguments.find("q") != arguments.end() ? arguments["q"] : "";

        string responseString = "<!DOCTYPE html><html><head><meta charset=\"utf-8\" />"
                                "<title>EDAoogle</title>"
                                "<link rel=\"stylesheet\" href=\"../css/style.css\" />"
                                "</head><body><article class=\"edaoogle\">"
                                "<div class=\"title\"><a href=\"/\">EDAoogle</a></div>"
                                "<div class=\"search\"><form action=\"/search\" method=\"get\">"
                                "<input type=\"text\" name=\"q\" value=\"" +
                                searchString + "\" autofocus>"
                                               "</form></div>";

        // Conectar con la base de datos SQLite
        sqlite3 *db;
        const char *dbPath = "/home/francob/Desktop/eda/EDAOOGLE/index.db";
        if (sqlite3_open(dbPath, &db) != SQLITE_OK)
        {
            cerr << "No se puede abrir la base de datos: " << sqlite3_errmsg(db) << endl;
            return false;
        }

        // Medición del tiempo de búsqueda
        auto start = chrono::system_clock::now();

        // Escapar caracteres especiales en `searchString` para evitar inyecciones
        for (auto &ch : searchString)
        {
            if (ch == '\'' || ch == '"' || ch == '%')
            {
                ch = ' ';
            }
        }

        // Consulta SQL para obtener `titulo`, `url`, y `contenido` con FTS5 y ordenamiento con bm25
        string searchQuery = "SELECT titulo, url, contenido, bm25(paginas_web) AS rank FROM paginas_web WHERE paginas_web MATCH ? ORDER BY rank LIMIT 10;";
        sqlite3_stmt *stmt;
        vector<tuple<string, string, string>> results; // Almacena titulo, url y contenido (vista previa)

        // Preparar la declaración con parámetros de seguridad
        if (sqlite3_prepare_v2(db, searchQuery.c_str(), -1, &stmt, 0) == SQLITE_OK)
        {
            sqlite3_bind_text(stmt, 1, searchString.c_str(), -1, SQLITE_TRANSIENT);

            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                const unsigned char *titulo = sqlite3_column_text(stmt, 0);
                const unsigned char *url = sqlite3_column_text(stmt, 1);
                const unsigned char *contenido = sqlite3_column_text(stmt, 2);

                string tituloStr = titulo ? reinterpret_cast<const char *>(titulo) : "Sin título";
                string urlStr = url ? reinterpret_cast<const char *>(url) : "#";
                string contenidoStr = contenido ? reinterpret_cast<const char *>(contenido) : "Sin contenido";

                // Limitar el contenido a una vista previa de 200 caracteres
                if (contenidoStr.size() > 200)
                {
                    contenidoStr = contenidoStr.substr(0, 200) + "...";
                }

                results.emplace_back(tituloStr, urlStr, contenidoStr);
            }
            sqlite3_finalize(stmt);
        }
        else
        {
            cerr << "Error en la ejecución de la consulta: " << sqlite3_errmsg(db) << endl;
            sqlite3_close(db);
            return false; // Detener ejecución si la consulta falla
        }

        sqlite3_close(db);

        auto end = chrono::system_clock::now();
        chrono::duration<double> searchTime = end - start;

        // Generación de los resultados en HTML
        responseString += "<div class=\"results\">" + to_string(results.size()) + " resultados (" + to_string(searchTime.count()) + " segundos):</div>";
        for (const auto &result : results)
        {
            responseString += "<div class=\"result\">"
                              "<a href=\"" +
                              get<1>(result) + "\"><strong>" + get<0>(result) + "</strong></a><br>"
                                                                                "<a href=\"" +
                              get<1>(result) + "\" class=\"url\">" + get<1>(result) + "</a><br>"
                                                                                      "<p>" +
                              get<2>(result) + "</p><br></div>";
        }

        responseString += "</article></body></html>";
        response.assign(responseString.begin(), responseString.end());

        return true;
    }
    else
    {
        return serve(url, response);
    }

    return false;
}
