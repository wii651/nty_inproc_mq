/***************************************************************
 * 
 * @file:    02_performance_test.c
 * @author:  wilson
 * @version: 1.0
 * @date:    2021-08-21 12:51:12
 * @license: MIT
 * @brief:   性能测试用例
 * 	编译 gcc 02_performance_test.c nty_inproc_mq.c -pthread
 * 	使用:
 * 	./a.out [发消息线程数] [收消息线程数]
 * 	之后会统计一秒内的收发总量
 * 
 ***************************************************************/

#include <stdio.h>
#include "nty_inproc_mq.h"
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int SENDER_NUM = 1;
int RECVER_NUM = 1;

int start_flag = 0;
int end_flag = 0;
uint64_t send_count = 0;
uint64_t recv_count = 0;

typedef struct ThreadParam {
	char t_name;
	MessageQueue* mq;
} ThreadParam;

typedef struct Msg {
	char sender;
	int val[50];
} Msg;

void* send_routine(void* arg) {
	//[ 发送1000条数据 ]
	ThreadParam* param = arg;
	char name = param->t_name;
	MessageQueue* mq = param->mq;
	while (end_flag == 0) {
		if (start_flag == 0)
			continue; 
		Msg* msg = (Msg*)malloc(sizeof(Msg));
		//msg->sender = name;
		//msg->val = i;
		mq_write(mq, msg);
		__sync_fetch_and_add(&send_count, 1);
		//printf("Sender[%c]: send msg[%d]\n", name, i);
	}
	
	//printf("Sender[%c] quitted...\n", name);
}


void* recv_routine(void* arg) {
	//[ 接收大家的数据 ]
	ThreadParam* param = arg;
	char name = param->t_name;
	MessageQueue* mq = param->mq;
	while (end_flag == 0) {
		if (start_flag == 0)
			continue; 
		Msg* msg = mq_tryread(mq);
		if (msg) {
			__sync_fetch_and_add(&recv_count, 1);
			free(msg);
		}
		//printf("\tRecver[%c]: recv msg[%d]from[%c]\n", name, msg->val, msg->sender);
	}
	//printf("Recver[%c] quitted...\n", name);
}

int main(int argc, char** argv) {

	if (argc != 3) {
		printf("error input param num\n");
		exit(1);
	}
	SENDER_NUM = atoi(argv[1]);
	RECVER_NUM = atoi(argv[2]);

	pthread_t t_send[SENDER_NUM];
	pthread_t t_recv[RECVER_NUM];
	MessageQueue* mq = mq_init();

	ThreadParam send_params[SENDER_NUM];
	ThreadParam recv_params[RECVER_NUM];

	for (int i = 0; i < SENDER_NUM; i++) {
		send_params[i].t_name = i + 97;
		send_params[i].mq = mq;
		pthread_create(&t_send[i], NULL, send_routine, &send_params[i]);
	}
	for (int i = 0; i < RECVER_NUM; i++) {
		recv_params[i].t_name = i + 109;
		recv_params[i].mq = mq;
		pthread_create(&t_recv[i], NULL, recv_routine, &recv_params[i]);
	}
	
	__sync_fetch_and_add(&start_flag, 1);
	usleep(1000000);
	__sync_fetch_and_add(&end_flag, 1);
	printf("send_count: %ld\nrecv_count: %ld\n", send_count, recv_count);
	for (int i = 0; i < SENDER_NUM; i++) {
	 	pthread_join(t_send[i], NULL);
	}
	for (int i = 0; i < RECVER_NUM; i++) {
	 	pthread_join(t_recv[i], NULL);
	}

	printf("main thread finished...\n");
	mq_destroy(mq);


	return 0;
}