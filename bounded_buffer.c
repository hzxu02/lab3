#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 5      // 버퍼의 크기 (제한된 버퍼)
#define NUM_PRODUCERS 2    // 생산자 수
#define NUM_CONSUMERS 2    // 소비자 수
#define MAX_ITEMS 20       // 생산할 총 아이템 수

// 공유 자원 (버퍼)
int buffer[BUFFER_SIZE];
int count = 0;  // 현재 버퍼에 있는 아이템 개수
int in = 0;     // 생산자가 넣을 인덱스
int out = 0;    // 소비자가 꺼낼 인덱스
int produced_count = 0; // 총 생산된 아이템 수

// 동기화를 위한 도구
pthread_mutex_t mutex;     // 상호 배제를 위한 뮤텍스
pthread_cond_t not_full;   // 버퍼가 꽉 차지 않음을 알리는 조건 변수 (생산자용)
pthread_cond_t not_empty;  // 버퍼가 비지 않음을 알리는 조건 변수 (소비자용)

// 생산자 쓰레드 함수
void *producer(void *arg) {
    int id = *(int *)arg;
    
    while (1) {
        // 임계 구역 진입 전 잠금 (Lock)
        pthread_mutex_lock(&mutex);

        // 총 생산량이 목표치에 도달하면 종료
        if (produced_count >= MAX_ITEMS) {
            pthread_mutex_unlock(&mutex);
            // 혹시 대기 중인 다른 쓰레드들을 깨워 종료하게 함 (Broadcast)
            pthread_cond_broadcast(&not_full);
            pthread_cond_broadcast(&not_empty);
            break;
        }

        // 버퍼가 꽉 찼으면 대기 (Wait)
        while (count == BUFFER_SIZE) {
            printf("[Producer %d] 버퍼가 꽉 찼습니다. 대기 중...\n", id);
            // not_full 조건이 만족될 때까지(누가 소비해서 공간이 생길 때까지) 대기
            pthread_cond_wait(&not_full, &mutex);
            
            // 깨어났는데 작업이 끝났으면 종료
            if (produced_count >= MAX_ITEMS) {
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
        }

        // 아이템 생산 및 버퍼에 저장 (Critical Section)
        int item = produced_count + 1;
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE; // 원형 버퍼 인덱스 증가
        count++;
        produced_count++;
        
        printf("[Producer %d] 생산: %d (Buffer: %d/%d)\n", id, item, count, BUFFER_SIZE);

        // 소비자를 위해 '비어있지 않음' 신호 발송 (Signal)
        pthread_cond_signal(&not_empty);
        
        // 잠금 해제 (Unlock)
        pthread_mutex_unlock(&mutex);
        
        usleep(100000); // 약간의 딜레이 (시뮬레이션)
    }
    return NULL;
}

// 소비자 쓰레드 함수
void *consumer(void *arg) {
    int id = *(int *)arg;

    while (1) {
        pthread_mutex_lock(&mutex);

        // 버퍼가 비어있으면 대기 (Wait)
        while (count == 0) {
            // 작업이 다 끝났고 버퍼도 비었으면 종료
            if (produced_count >= MAX_ITEMS && count == 0) {
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
            
            printf("[Consumer %d] 버퍼가 비었습니다. 대기 중...\n", id);
            // not_empty 조건이 만족될 때까지(누가 생산해서 데이터가 생길 때까지) 대기
            pthread_cond_wait(&not_empty, &mutex);
        }

        // 아이템 소비 (Critical Section)
        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE; // 원형 버퍼 인덱스 증가
        count--;

        printf("    => [Consumer %d] 소비: %d (Buffer: %d/%d)\n", id, item, count, BUFFER_SIZE);

        // 생산자를 위해 '꽉 차지 않음' 신호 발송 (Signal)
        pthread_cond_signal(&not_full);
        
        pthread_mutex_unlock(&mutex);

        usleep(150000); // 소비 시간이 생산보다 조금 더 걸린다고 가정
    }
    return NULL;
}

int main() {
    pthread_t pro[NUM_PRODUCERS], con[NUM_CONSUMERS];
    int p_ids[NUM_PRODUCERS], c_ids[NUM_CONSUMERS];

    // 뮤텍스 및 조건 변수 초기화
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&not_full, NULL);
    pthread_cond_init(&not_empty, NULL);

    // 생산자 쓰레드 생성
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        p_ids[i] = i + 1;
        pthread_create(&pro[i], NULL, producer, &p_ids[i]);
    }

    // 소비자 쓰레드 생성
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        c_ids[i] = i + 1;
        pthread_create(&con[i], NULL, consumer, &c_ids[i]);
    }

    // 모든 쓰레드 종료 대기
    for (int i = 0; i < NUM_PRODUCERS; i++) pthread_join(pro[i], NULL);
    for (int i = 0; i < NUM_CONSUMERS; i++) pthread_join(con[i], NULL);

    // 자원 해제
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);

    printf("=== 모든 생산 및 소비 작업 완료 ===\n");
    return 0;
}