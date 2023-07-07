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

    // �����׽���
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cout << "�����׽���ʧ��" << std::endl;
        return -1;
    }

    std::cout << "�����׽��ֳɹ�" << std::endl;

    // ���׽���
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);

    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cout << "���׽���ʧ��" << std::endl;
        return -1;
    }

    // �����׽���
    if (listen(serverSocket, 5) == -1) {
        std::cout << "�����׽���ʧ��" << std::endl;
        return -1;
    }

    while (true) {
        // ��������
        sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket == -1) {
            std::cout << "��������ʧ��" << std::endl;
            return -1;
        }

        // ����Http����
        char buffer[BUFFER_SIZE];
        int length = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (length == -1) {
            std::cout << "����Http����ʧ��" << std::endl;
            // �ر��׽���
            close(clientSocket);
            continue;
        }

        // ��������·��
        std::string request{ buffer };
        size_t pathStart = request.find(' ') + 1;
        size_t pathEnd = request.find(' ', pathStart);
        std::string requestPath = request.substr(pathStart, pathEnd - pathStart);

        // �򿪲���ȡ��̬ҳ��
        std::string filePath = "." + requestPath;  // ���農̬ҳ��λ�ڵ�ǰ����Ŀ¼��
        std::string pageContent = readStaticPage(filePath);

        // ����HTTP��Ӧ
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

        // ����HTTP��Ӧ
        sendResponse(clientSocket, response);

        // �رտͻ����׽���
        close(clientSocket);
    }

    // �رշ������׽���
    close(serverSocket);

    return 0;
}