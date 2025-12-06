#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// 공유 자원 및 동기화 도구
pthread_mutex_t mutex;
pthread_cond_t cond;
int is_child_turn = 0; // 이진 플래그 (0: 부모 차례, 1: 자식 차례)

// 자식 쓰레드 함수
void *child_func(void *arg) {
    for (int i = 0; i < 5; i++) { // 5번 반복
        pthread_mutex_lock(&mutex);

        // 내 차례(1)가 아니면 대기
        // (Spurious wakeup 방지를 위해 if 대신 while 사용 권장)
        while (is_child_turn != 1) {
            pthread_cond_wait(&cond, &mutex);
        }

        // 작업 수행
        printf("hello child\n");
        sleep(1); // 1초 대기

        // 차례 넘기기 (자식(1) -> 부모(0))
        is_child_turn = 0;
        
        // 잠자는 부모 쓰레드 깨우기
        pthread_cond_signal(&cond);

        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t tid;

    // 초기화
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    // 자식 쓰레드 생성
    if (pthread_create(&tid, NULL, child_func, NULL) != 0) {
        perror("쓰레드 생성 실패");
        exit(1);
    }

    // 부모 쓰레드 작업 (메인 함수)
    for (int i = 0; i < 5; i++) { // 5번 반복
        pthread_mutex_lock(&mutex);

        // 내 차례(0)가 아니면 대기
        while (is_child_turn != 0) {
            pthread_cond_wait(&cond, &mutex);
        }

        // 작업 수행
        printf("hello parent\n");
        sleep(1); // 1초 대기

        // 차례 넘기기 (부모(0) -> 자식(1))
        is_child_turn = 1;
        
        // 잠자는 자식 쓰레드 깨우기
        pthread_cond_signal(&cond);

        pthread_mutex_unlock(&mutex);
    }

    // 자식 쓰레드 종료 대기
    pthread_join(tid, NULL);

    // 자원 해제
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}