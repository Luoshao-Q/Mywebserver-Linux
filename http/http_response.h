#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string>
#include <climits>
#include <cstdlib>
#include <sys/stat.h>   // for stat, S_ISDIR
#include "utils/file_utils.h"

struct HttpResponse {
    int statusCode;  // 状态码：200、404、500
    std::string contentType;  // 内容类型：text/html、image/png
    std::string body;  // 响应体：文件内容

    //拼装 HTTP 响应报文
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
        // 1. URL 解码
        std::string decodedPath = urlDecode(filepath);
        
        // 2. 构建完整路径
        std::string fullPath = rootDir + decodedPath;
        
        // 3. 用 realpath 解析真实路径
        char realBuf[PATH_MAX];
        if (realpath(fullPath.c_str(), realBuf) == nullptr) {
            return make404Response(rootDir);
        }
        std::string resolvedPath(realBuf);
        
        // 4. 规范化 rootDir
        char rootBuf[PATH_MAX];
        if (realpath(rootDir.c_str(), rootBuf) == nullptr) {
            return make404Response(rootDir);
        }
        std::string rootReal(rootBuf);
        
        // 5. ? 修复问题1：目录边界校验（防前缀碰撞）
        //检查“请求的文件是否真的在根目录内”，防止攻击者用 root-bak 这种前缀相同的目录绕过检查
        bool withinRoot = false;
        if (resolvedPath == rootReal) {
            withinRoot = true;
        } else if (resolvedPath.size() > rootReal.size() &&
                   resolvedPath.compare(0, rootReal.size(), rootReal) == 0 &&
                   resolvedPath[rootReal.size()] == '/') {
            withinRoot = true;
        }
        
        if (!withinRoot) {
            return make404Response(rootDir);
        }
        
        // 6. ? 修复问题2：如果是目录，自动补上 index.html
        struct stat st;
        if (stat(resolvedPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            resolvedPath += "/index.html";
        }
        
        // 7. 读取文件
        std::string content = readFile(resolvedPath);
        HttpResponse resp;
        resp.contentType = getContentType(resolvedPath);
        if (!content.empty()) {
            resp.statusCode = 200;
            resp.body = content;
        } else {
            return make404Response(rootDir);
        }
        return resp;
    }

    static HttpResponse make404Response(const std::string& rootDir) {
        HttpResponse resp;
        resp.statusCode = 404;
        resp.contentType = "text/html";
        std::string notFoundContent = readFile(rootDir + "/404.html");
        resp.body = notFoundContent.empty() ? "<h1>404 Not Found</h1>" : notFoundContent;
        return resp;
    }

    static std::string urlDecode(const std::string& str) {
        std::string result;
        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == '%' && i + 2 < str.size()) {
                std::string hex = str.substr(i + 1, 2);
                char* endptr;
                long val = strtol(hex.c_str(), &endptr, 16);
                if (endptr == hex.c_str() + 2) {
                    result += static_cast<char>(val);
                    i += 2;
                    continue;
                }
            }
            result += str[i];
        }
        return result;
    }
};

#endif