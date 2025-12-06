#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100
#define MAX_CLNT 256

void *handle_clnt(void *arg);
void send_msg(char *msg, int len);
void error_handling(char *msg);

int clnt_cnt = 0;            // 접속한 클라이언트 수
int clnt_socks[MAX_CLNT];    // 클라이언트 소켓 관리 배열
pthread_mutex_t mutx;        // 뮤텍스 (임계 영역 보호)

int main(int argc, char *argv[]) {
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;
    pthread_t t_id;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    pthread_mutex_init(&mutx, NULL); // 뮤텍스 초기화
    serv_sock = socket(PF_INET, SOCK_STREAM, 0);

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");
    if (listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    printf("=== 채팅 서버 시작 (Port: %s) ===\n", argv[1]);

    while (1) {
        clnt_adr_sz = sizeof(clnt_adr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);

        // [임계 구역] 새로운 클라이언트 소켓을 배열에 등록
        pthread_mutex_lock(&mutx);
        clnt_socks[clnt_cnt++] = clnt_sock;
        pthread_mutex_unlock(&mutx);

        // 해당 클라이언트를 담당할 쓰레드 생성
        pthread_create(&t_id, NULL, handle_clnt, (void *)&clnt_sock);
        pthread_detach(t_id); // 쓰레드 종료 시 자원 자동 소멸
        printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
    }
    close(serv_sock);
    return 0;
}

// 클라이언트 담당 쓰레드 함수
void *handle_clnt(void *arg) {
    int clnt_sock = *((int *)arg);
    int str_len = 0;
    char msg[BUF_SIZE];

    // 메시지 수신 루프
    while ((str_len = read(clnt_sock, msg, sizeof(msg))) != 0)
        send_msg(msg, str_len); // 받은 메시지를 모든 클라이언트에게 전송

    // 연결 종료 시 배열에서 제거
    pthread_mutex_lock(&mutx);
    for (int i = 0; i < clnt_cnt; i++) {
        if (clnt_sock == clnt_socks[i]) {
            // 현재 소켓을 제거하고 뒤에 있는 소켓들을 한 칸씩 앞으로 당김
            while (i++ < clnt_cnt - 1)
                clnt_socks[i] = clnt_socks[i + 1];
            break;
        }
    }
    clnt_cnt--;
    pthread_mutex_unlock(&mutx);
    close(clnt_sock);
    return NULL;
}

// 메시지 방송 (Broadcast) 함수
void send_msg(char *msg, int len) {
    pthread_mutex_lock(&mutx);
    for (int i = 0; i < clnt_cnt; i++)
        write(clnt_socks[i], msg, len);
    pthread_mutex_unlock(&mutx);
}

void error_handling(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}