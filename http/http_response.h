#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string>
#include "utils/file_utils.h"

struct HttpResponse {
    int statusCode;
    std::string contentType;
    std::string body;

    std::string build() const {
        std::string statusLine;
        if (statusCode == 200) {
            statusLine = "HTTP/1.1 200 OK\r\n";
        } else if (statusCode == 404) {
            statusLine = "HTTP/1.1 404 Not Found\r\n";
        } else {
            statusLine = "HTTP/1.1 500 Internal Server Error\r\n";
        }

        return statusLine +
               "Content-Type: " + contentType + "\r\n" +
               "Content-Length: " + std::to_string(body.size()) + "\r\n" +
               "Connection: close\r\n" +
               "\r\n" +
               body;
    }

    static HttpResponse fromFile(const std::string& filepath, const std::string& rootDir) {
        std::string fullPath = filepath == "/" ? rootDir + "/index.html" : rootDir + filepath;
        std::string content = readFile(fullPath);
        HttpResponse resp;
        resp.contentType = getContentType(fullPath);

        if (!content.empty()) {
            resp.statusCode = 200;
            resp.body = content;
        } else {
            std::string notFoundPath = rootDir + "/404.html";
            std::string notFoundContent = readFile(notFoundPath);
            resp.statusCode = 404;
            resp.contentType = "text/html";
            resp.body = notFoundContent.empty() ? "<h1>404 Not Found</h1>" : notFoundContent;
        }
        return resp;
    }
};

#endif