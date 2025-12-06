#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    const char *welcome_msg = "서버에 연결되었습니다. (종료: 'exit' 입력)\n";

    printf("[Server] 소켓 생성 중...\n");

    // 1. 소켓 생성 (IPv4, TCP)
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("소켓 생성 실패");
        exit(EXIT_FAILURE);
    }

    // 주소 구조체 설정
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // 모든 IP에서의 연결 허용
    address.sin_port = htons(PORT);       // 포트 번호 설정 (Host to Network Short)

    // 2. 바인딩 (소켓에 주소 할당)
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("바인딩 실패 (포트가 이미 사용 중일 수 있습니다)");
        exit(EXIT_FAILURE);
    }

    // 3. 리스닝 (연결 대기)
    if (listen(server_fd, 3) < 0) {
        perror("리스닝 실패");
        exit(EXIT_FAILURE);
    }

    printf("[Server] 포트 %d에서 클라이언트 연결 대기 중...\n", PORT);

    // 4. 연결 수락 (Accept) - 블로킹(대기) 상태
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("연결 수락 실패");
        exit(EXIT_FAILURE);
    }

    printf("[Server] 클라이언트가 연결되었습니다!\n");
    send(new_socket, welcome_msg, strlen(welcome_msg), 0);

    // 5. 데이터 송수신 (Echo)
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        
        if (valread <= 0) {
            printf("[Server] 클라이언트 연결 종료\n");
            break;
        }

        // 'exit' 메시지 체크 (줄바꿈 문자 제거 후 비교)
        buffer[strcspn(buffer, "\n")] = 0;
        if (strcmp(buffer, "exit") == 0) {
            printf("[Server] 종료 요청 수신. 연결을 끊습니다.\n");
            break;
        }

        printf("[Server] 받은 메시지: %s\n", buffer);

        // 에코 (받은 그대로 다시 전송)
        char response[BUFFER_SIZE + 20];
        sprintf(response, "[Echo]: %s", buffer);
        send(new_socket, response, strlen(response), 0);
    }

    // 6. 소켓 종료
    close(new_socket);
    close(server_fd);
    return 0;
}