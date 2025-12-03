#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// 쓰레드가 실행할 함수
// void* 를 인자로 받고 void* 를 반환하는 형태여야 합니다.
void *thread_function(void *arg) {
    int id = *(int *)arg; // 인자로 받은 쓰레드 ID
    
    printf("[Thread %d] 쓰레드가 생성되었습니다.\n", id);
    
    for (int i = 0; i < 3; i++) {
        printf("[Thread %d] 작동 중... (%d초)\n", id, i + 1);
        sleep(1); // 1초 대기
    }
    
    printf("[Thread %d] 작업 완료 및 종료.\n", id);
    return NULL;
}

int main() {
    pthread_t t_id1, t_id2; // 쓰레드 식별자
    int id1 = 1;
    int id2 = 2;
    int status;

    printf("[Main] 메인 함수 시작. 쓰레드를 생성합니다.\n");

    // 1. 첫 번째 쓰레드 생성
    // pthread_create(&식별자, 속성, 실행할함수, 인자)
    if (pthread_create(&t_id1, NULL, thread_function, (void *)&id1) != 0) {
        perror("쓰레드 1 생성 실패");
        return 1;
    }

    // 2. 두 번째 쓰레드 생성
    if (pthread_create(&t_id2, NULL, thread_function, (void *)&id2) != 0) {
        perror("쓰레드 2 생성 실패");
        return 1;
    }

    // 3. 쓰레드 종료 대기 (Join)
    // 메인 함수가 먼저 종료되면 자식 쓰레드들도 강제로 종료되므로,
    // 작업이 끝날 때까지 기다려줍니다.
    pthread_join(t_id1, (void **)&status);
    pthread_join(t_id2, (void **)&status);

    printf("[Main] 모든 쓰레드가 종료되었습니다. 메인 함수를 종료합니다.\n");

    return 0;
}