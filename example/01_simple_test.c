/***************************************************************
 * 
 * @file:    01_simple_test.c
 * @author:  wilson
 * @version: 1.0
 * @date:    2021-08-21 13:06:23
 * @license: MIT
 * @brief:   简单测试用例, 对消息队列执行, 多写单独
 * 编译:
 * 	gcc -g ./01_simple_test.c ./nty_inproc_mq.c -pthread
 ***************************************************************/

#include <stdio.h>
#include "nty_inproc_mq.h"
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define SENDER_NUM		2
#define MSG_NUM			10

typedef struct Msg {
	char sender;
	int val;
} Msg;

void* send_routine(void* arg) {
	MessageQueue* mq = (MessageQueue*)arg;
	for (int i = 0; i < MSG_NUM; i++) {
		//[ 准备要发送的消息体, 务必使用堆空间 ]
		Msg* msg = (Msg*)malloc(sizeof(Msg));
		msg->sender = 'a';
		msg->val = i;
		//[ 发送消息 ]
		mq_write(mq, msg);
		printf("Sender[%c]: send msg[%d]\n", 'a', i);
		usleep(10000);
	}
	printf("a sender quitted...\n");
}


void* recv_routine(void* arg) {
	MessageQueue* mq = (MessageQueue*)arg;
	int total_msg_num = SENDER_NUM * MSG_NUM;
	int recv_count = 0;
	while (recv_count < total_msg_num) {
		//[ 读消息 ]
		Msg* msg = (Msg*)mq_tryread(mq);	//[ mq_read(mq)亦可 ]
		if (msg) {
			printf("\tRecver: recv msg[%d]from[%c]\n", msg->val, msg->sender);
			//[ 读到的消息, 要自己负责释放资源 ]
			free(msg);
			usleep(10000);
			recv_count++;
		}
	}
	printf("Receiver quitted...\n");
}

int main() {
	pthread_t t_send[SENDER_NUM];
	pthread_t t_recv;
	//[ 初始化队列对象 ]
	MessageQueue* mq = mq_init();
	for (int i = 0; i < SENDER_NUM; i++) {
		pthread_create(&t_send[i], NULL, send_routine, (void*)mq);
	}

	pthread_create(&t_recv, NULL, recv_routine, (void*)mq);

	for (int i = 0; i < SENDER_NUM; i++) {
		pthread_join(t_send[i], NULL);
	}
	pthread_join(t_recv, NULL);

	printf("main thread finished...\n");

	//[ 释放队列对象, 对于其中未被读取的消息和消息的资源, 都会一并释放 ]
	mq_destroy(mq);

	return 0;
}