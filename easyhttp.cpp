#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

constexpr int PORT = 8081;

// 根据扩展名推断 Content-Type
std::string contentTypeOf(const std::string& path) {
    auto endsWith = [&](const char* suffix) {
        size_t n = std::strlen(suffix);
        return path.size() >= n && path.compare(path.size() - n, n, suffix) == 0;
    };
    if (endsWith(".html")) return "text/html; charset=utf-8";
    if (endsWith(".css"))  return "text/css; charset=utf-8";
    if (endsWith(".js"))   return "application/javascript; charset=utf-8";
    return "application/octet-stream";
}

// 从 www/ 目录读取文件；成功返回 true 并填充 body
bool readWebFile(const std::string& urlPath, std::string& body) {
    // "/" 映射到 index.html
    std::string relative = (urlPath == "/") ? "/index.html" : urlPath;

    // 防止路径穿越：拒绝任何含 ".." 的请求
    if (relative.find("..") != std::string::npos) {
        return false;
    }

    std::ifstream file("www" + relative, std::ios::binary);
    if (!file) {
        return false;
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    body = contents.str();
    return true;
}

// 处理一次客户端请求，返回生成的 HTTP 响应字符串
std::string handleRequest(const std::string& request) {
    std::istringstream in(request);
    std::string method, path, version;
    in >> method >> path >> version;

    if (method != "GET") {
        std::string body = "Only GET is supported";
        std::ostringstream resp;
        resp << "HTTP/1.1 405 Method Not Allowed\r\n"
             << "Content-Type: text/plain; charset=utf-8\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Connection: close\r\n\r\n"
             << body;
        return resp.str();
    }

    std::string body;
    std::string status = "200 OK";
    std::string contentType = "text/html; charset=utf-8";

    if (readWebFile(path, body)) {
        contentType = contentTypeOf(path);
    } else {
        status = "404 Not Found";
        body = "<html><body><h1>404 Not Found</h1>"
               "<p>The requested path was not found.</p></body></html>";
    }

    std::ostringstream resp;
    resp << "HTTP/1.1 " << status << "\r\n"
         << "Content-Type: " << contentType << "\r\n"
         << "Content-Length: " << body.size() << "\r\n"
         << "Connection: close\r\n\r\n"
         << body;
    return resp.str();
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "socket() failed\n";
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "bind() failed\n";
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "listen() failed\n";
        close(server_fd);
        return 1;
    }

    std::cout << "easyhttp listening on http://localhost:" << PORT << "\n";

    while (true) {
        sockaddr_in client{};
        socklen_t client_len = sizeof(client);
        int client_fd = accept(server_fd, reinterpret_cast<sockaddr*>(&client), &client_len);
        if (client_fd < 0) {
            std::cerr << "accept() failed\n";
            continue;
        }

        char buffer[4096];
        ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (n > 0) {
            buffer[n] = '\0';
            std::string request(buffer);

            std::string response = handleRequest(request);
            send(client_fd, response.c_str(), response.size(), 0);
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}
