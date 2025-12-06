#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char input[BUFFER_SIZE] = {0};

    printf("[Client] 소켓 생성 중...\n");

    // 1. 소켓 생성
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n 소켓 생성 오류 \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // 127.0.0.1 (로컬호스트) 주소 변환
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\n주소 변환 오류 / 지원하지 않는 주소 \n");
        return -1;
    }

    // 2. 서버 연결 요청 (Connect)
    printf("[Client] 서버(127.0.0.1:%d)에 연결 시도...\n", PORT);
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\n연결 실패 (서버가 실행 중인지 확인하세요)\n");
        return -1;
    }

    // 환영 메시지 수신
    read(sock, buffer, BUFFER_SIZE);
    printf("%s\n", buffer);

    // 3. 데이터 송수신 루프
    while (1) {
        printf("보낼 메시지 (종료: exit) > ");
        if (fgets(input, BUFFER_SIZE, stdin) == NULL) break;

        // 메시지 전송
        send(sock, input, strlen(input), 0);

        // 'exit' 입력 시 클라이언트도 종료
        input[strcspn(input, "\n")] = 0; // 줄바꿈 제거
        if (strcmp(input, "exit") == 0) {
            break;
        }

        // 서버 응답 수신
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(sock, buffer, BUFFER_SIZE);
        if (valread > 0) {
            printf("%s\n", buffer);
        } else {
            printf("서버와 연결이 끊어졌습니다.\n");
            break;
        }
    }

    // 4. 소켓 종료
    close(sock);
    return 0;
}