#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <unordered_map>
#include <algorithm>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    bool parse(const std::string& rawData) {
        size_t pos = rawData.find("\r\n");
        if (pos == std::string::npos) return false;

        std::string requestLine = rawData.substr(0, pos);
        size_t first = requestLine.find(' ');
        if (first == std::string::npos) return false;
        size_t second = requestLine.find(' ', first + 1);
        if (second == std::string::npos) return false;

        method = requestLine.substr(0, first);
        path = requestLine.substr(first + 1, second - first - 1);
        version = requestLine.substr(second + 1);

        // 去掉 path 里的查询参数（?后面的）
        size_t qpos = path.find('?');
        if (qpos != std::string::npos) {
            path = path.substr(0, qpos);
        }

        // 简单解析 headers（略，目前用不到）
        return true;
    }

    bool isGet() const { return method == "GET"; }
    bool isPost() const { return method == "POST"; }
};

#endif