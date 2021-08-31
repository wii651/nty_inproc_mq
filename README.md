# nty_inproc_mq
简易的线程间消息队列组件

对于4核心线程的cpu, 2个写线程2个读线程的情况下, 每秒信息收发量在3,000,000左右

## API
```c
// 创建消息队列
MessageQueue* mq_init();

// 向消息队列中写消息
int mq_write(MessageQueue* mq, void* msg_cnt);

// 非阻塞方式, 从消息队列中读消息
void* mq_tryread(MessageQueue* mq);

// 阻塞方式, 从消息队列中读消息
void* mq_read(MessageQueue* mq);

// 销毁消息队列
void mq_destroy(MessageQueue* mq);
```

## 编译
以编译quick_start.c为例
```c
gcc quick_start.c nty_inproc_mq.c -pthread
```