#include <iostream>
#include <sstream>
#include <string>
#define _WIN32_WINNT 0x501 
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

using std::cerr;
int main()
{
    WSADATA wsaData; // структура для хранение информации
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    // Ошибка библеотеки
    if (result != 0) {
        cerr << "WSAStartup failed: " << result << "\n";
        return result;
    }

    struct addrinfo* addr = NULL; // структура, хранящая информацию
    // инициализация структуры адреса
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET; 
    // использование сети для работы с сокетом
    hints.ai_socktype = SOCK_STREAM; // Задаётся потоковый тип сокета
    hints.ai_protocol = IPPROTO_TCP; // Используется протокол TCP
    hints.ai_flags = AI_PASSIVE;

    result = getaddrinfo("127.0.0.1", "8000", &hints, &addr);
    // Инициализация структуры адреса завершилась с ошибкой
    if (result != 0) {
        cerr << "getaddrinfo failed: " << result << "\n";
        WSACleanup(); // выгрузка библиотеки Ws2_32.dll
        return 1;
    }
    // Создание сокета
    int listen_socket = socket(addr->ai_family, addr->ai_socktype,
        addr->ai_protocol);
    // выгружаем dll-библиотеку и закрываем программу
    if (listen_socket == INVALID_SOCKET) {
        cerr << "Error at socket: " << WSAGetLastError() << "\n";
        freeaddrinfo(addr);
        WSACleanup();
        return 1;
    }
    // Привязываем сокет к IP-адресу
    result = bind(listen_socket, addr->ai_addr, (int)addr->ai_addrlen);
    // Выгружаем dll-библиотеку закрываем программу.
    if (result == SOCKET_ERROR) {
        cerr << "bind failed with error: " << WSAGetLastError() << "\n";
        freeaddrinfo(addr);
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }
    // Инициализируем слушающий сокет
    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "listen failed with error: " << WSAGetLastError() << "\n";
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }
   
    const int max_client_buffer_size = 1024;
    char buf[max_client_buffer_size];
    int client_socket = INVALID_SOCKET;

    for (;;) {
        // Производится принятие соединения
        client_socket = accept(listen_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            cerr << "accept failed: " << WSAGetLastError() << "\n";
            closesocket(listen_socket);
            WSACleanup();
            return 1;
        }

        result = recv(client_socket, buf, max_client_buffer_size, 0);
        std::stringstream response; // Будет производится запись ответа
        std::stringstream response_body; 

        if (result == SOCKET_ERROR) {
            cerr << "recv failed: " << result << "\n";
            closesocket(client_socket);
        } else if (result == 0) {
            cerr << "connection closed...\n";
        } else if (result > 0) {
            
            buf[result] = '\0';
            response_body << "<title>Test C++ HTTP Server</title>\n"
                << "<h1>Test page</h1>\n"
                << "<p>This is body of the test page...</p>\n"
                << "<h2>Request headers</h2>\n"
                << "<pre>" << buf << "</pre>\n"
                << "<em><small>Test C++ Http Server</small></em>\n";
            
            response << "HTTP/1.1 200 OK\r\n"
                << "Version: HTTP/1.1\r\n"
                << "Content-Type: text/html; charset=utf-8\r\n"
                << "Content-Length: " << response_body.str().length()
                << "\r\n\r\n"
                << response_body.str();

            // Отправка ответа send
            result = send(client_socket, response.str().c_str(),
                response.str().length(), 0);

            if (result == SOCKET_ERROR) {
                // Если ошибка при отправке
                cerr << "send failed: " << WSAGetLastError() << "\n";
            }
            // Закрываем конекта
            closesocket(client_socket);
        }
    }
    closesocket(listen_socket);
    freeaddrinfo(addr);
    WSACleanup();
    return 0;
}
