#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <string>
#include <unordered_map>
#include <algorithm>

struct HttpRequest {
    std::string method;  // 请求方法，如 "GET"
    std::string path;  // 请求路径，如 "/index.html"
    std::string version;  // HTTP 版本，如 "HTTP/1.1"
    std::unordered_map<std::string, std::string> headers;  // 请求头（目前没用）
    std::string body;  // 请求体（目前没用）

    bool parse(const std::string& rawData) {
        size_t pos = rawData.find("\r\n");
        if (pos == std::string::npos) return false;
        //HTTP 报文的第一行（请求行）以 \r\n 结尾。
        //如果找不到 \r\n，说明报文格式不对，直接返回 false

        std::string requestLine = rawData.substr(0, pos);
        //从开头截取到 \r\n 之前，得到请求行，比如：GET /index.html HTTP/1.1
        size_t first = requestLine.find(' ');
        //按空格拆分请求行
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

        // 简单解析 headers（略）
        return true;
    }

    bool isGet() const { return method == "GET"; }
    bool isPost() const { return method == "POST"; }
};

#endif