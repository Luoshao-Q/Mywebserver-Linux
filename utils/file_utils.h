#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string>
#include <fstream>
#include <sstream>

// 读取静态文件内容
inline std::string readFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// 根据文件后缀返回 Content-Type
inline std::string getContentType(const std::string& filepath) {
    size_t dot = filepath.find_last_of(".");
    if (dot == std::string::npos) return "text/plain";
    std::string ext = filepath.substr(dot);
    if (ext == ".html" || ext == ".htm") return "text/html";
    if (ext == ".css") return "text/css";
    if (ext == ".js") return "application/javascript";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".png") return "image/png";
    if (ext == ".gif") return "image/gif";
    if (ext == ".ico") return "image/x-icon";
    return "text/plain";
}

#endif