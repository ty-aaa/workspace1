#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

constexpr int PORT = 8081;

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

    if (path == "/" || path == "/index.html") {
        body = "<html><body><h1>Hello from easyhttp!</h1>"
               "<p>It works.</p></body></html>";
    } else {
        status = "404 Not Found";
        body = "<html><body><h1>404 Not Found</h1>"
               "<p>The requested path was not found.</p></body></html>";
    }

    std::ostringstream resp;
    resp << "HTTP/1.1 " << status << "\r\n"
         << "Content-Type: text/html; charset=utf-8\r\n"
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
