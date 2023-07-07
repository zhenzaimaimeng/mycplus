#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

std::string readStaticPage(const std::string& filePath) {
    std::ifstream file(filePath);
    if (file) {
        std::string content((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());
        return content;
    }
    return "";
}


void sendResponse(int clientSocket, const std::string& response) {
    send(clientSocket, response.c_str(), response.size(), 0);
}


int main() {

    // 创建套接字
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cout << "创建套接字失败" << std::endl;
        return -1;
    }

    std::cout << "创建套接字成功" << std::endl;

    // 绑定套接字
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cout << "绑定套接字失败" << std::endl;
        return -1;
    }

    // 监听套接字
    if (listen(serverSocket, 5) == -1) {
        std::cout << "监听套接字失败" << std::endl;
        return -1;
    }

    while (true) {
        // 接受链接
        sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket == -1) {
            std::cout << "接受链接失败" << std::endl;
            return -1;
        }

        // 接受Http请求
        char buffer[BUFFER_SIZE];
        int length = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (length == -1) {
            std::cout << "接受Http请求失败" << std::endl;
            // 关闭套接字
            close(clientSocket);
            continue;
        }

        // 解析请求路径
        std::string request{ buffer };
        size_t pathStart = request.find(' ') + 1;
        size_t pathEnd = request.find(' ', pathStart);
        std::string requestPath = request.substr(pathStart, pathEnd - pathStart);

        // 打开并读取静态页面
        std::string filePath = "." + requestPath;  // 假设静态页面位于当前工作目录下
        std::string pageContent = readStaticPage(filePath);

        // 构造HTTP响应
        std::string response;
        if (!pageContent.empty()) {
            response = "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: " + std::to_string(pageContent.size()) + "\r\n"
                "\r\n" + pageContent;
        }
        else {
            response = "HTTP/1.1 404 Not Found\r\n\r\n";
        }

        // 发送HTTP响应
        sendResponse(clientSocket, response);

        // 关闭客户端套接字
        close(clientSocket);
    }

    // 关闭服务器套接字
    close(serverSocket);

    return 0;
}