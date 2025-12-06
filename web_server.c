#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h> // isxdigit 함수 등을 위해 필요

#define PORT 8080
#define BUFFER_SIZE 4096

// 16진수 문자를 정수로 변환하는 헬퍼 함수
int hex_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

// URL 디코딩 함수 (%EC%84%A4 -> 설)
void url_decode(char *src, char *dest) {
    while (*src) {
        if (*src == '+') { 
            *dest = ' '; // 공백 처리
            src++; 
        } 
        else if (*src == '%' && isxdigit(*(src + 1)) && isxdigit(*(src + 2))) {
            // %XX 형태인 경우 16진수를 문자로 변환
            *dest = (char)((hex_to_int(*(src + 1)) << 4) | hex_to_int(*(src + 2)));
            src += 3; 
        } 
        else {
            *dest = *src; // 그 외에는 그대로 복사
            src++; 
        }
        dest++;
    }
    *dest = '\0'; // 문자열 끝마침
}

void handle_client(int client_sock) {
    char buffer[BUFFER_SIZE*2] = {0};
    read(client_sock, buffer, BUFFER_SIZE);
    
    // HTTP 메소드 파싱
    if (strncmp(buffer, "GET", 3) == 0) {
        char *response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n\r\n"
            "<html><head><meta charset='UTF-8'></head><body>"
            "<h1>안녕하세요! GET 요청입니다.</h1>"
            "<form method='POST' action='/'>"
            "이름: <input type='text' name='username'>"
            "<input type='submit' value='POST 전송'>"
            "</form>"
            "</body></html>";
        write(client_sock, response, strlen(response));
    } 
    else if (strncmp(buffer, "POST", 4) == 0) {
        // 본문(Body) 찾기
        char *body = strstr(buffer, "\r\n\r\n");
        char decoded_body[BUFFER_SIZE] = {0}; // 디코딩된 문자열 저장 공간

        if (body) {
            body += 4; // 빈 줄 건너뛰기
            // username=... 부분만 파싱하기 위해 간단히 처리
            if (strncmp(body, "username=", 9) == 0) {
                // "username=" 뒤의 값만 디코딩
                url_decode(body + 9, decoded_body);
            } else {
                url_decode(body, decoded_body);
            }
        } else {
            strcpy(decoded_body, "데이터 없음");
        }

        char response[BUFFER_SIZE*2];
        sprintf(response, 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n\r\n"
            "<html><head><meta charset='UTF-8'></head><body>"
            "<h1>POST 요청을 받았습니다!</h1>"
            "<h2>전송된 이름: <span style='color:blue'>%s</span></h2>"
            "<a href='/'>돌아가기</a>"
            "</body></html>", decoded_body);
        
        write(client_sock, response, strlen(response));
    } 
    else {
        char *response = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
        write(client_sock, response, strlen(response));
    }

    close(client_sock);
}

int main() {
    int server_fd, client_sock;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed"); exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed"); exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed"); exit(EXIT_FAILURE);
    }

    printf("=== 향상된 웹 서버 시작 (http://localhost:%d) ===\n", PORT);

    while (1) {
        if ((client_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed"); continue;
        }
        handle_client(client_sock);
    }
    return 0;
}