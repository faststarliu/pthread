# linux线程同步-屏障 
## 一.概述                                                   
barrier(屏障)与互斥量，读写锁，自旋锁不同，它不是用来保护临界区的。相反，它跟条件变量一样，是用来协同多线程一起工作！！！
<br/>条件变量是多线程间传递状态的改变来达到协同工作的效果。屏障是多线程各自做自己的工作，如果某一线程完成了工作，就等待在屏障那里，直到其他线程的工作都完成了，再一起做别的事。举个通俗的例子：
<br/>1.对于条件变量。在接力赛跑里，1号队员开始跑的时候，2，3，4号队员都站着不动，直到1号队员跑完一圈，把接力棒给2号队员，2号队员收到接力棒后就可以跑了，跑完再给3号队员。这里这个接力棒就相当于条件变量，条件满足后就可以由下一个队员(线程)跑。
<br/>2.对于屏障。在百米赛跑里，比赛没开始之前，每个运动员都在赛场上自由活动，有的热身，有的喝水，有的跟教练谈论。比赛快开始时，准备完毕的运动员就预备在起跑线上，如果有个运动员还没准备完(除去特殊情况)，他们就一直等，直到运动员都在起跑线上，裁判喊口号后再开始跑。这里的起跑线就是屏障，做完准备工作的运动员都等在起跑线，直到其他运动员也把准备工作做完！
## 二.函数接口                                           
### 1.创建屏障

<br/>#include <pthread.h>
<br/>int pthread_barrier_init(pthread_barrier_t *restrict barrier, const pthread_barrierattr_t *restrict attr, unsigned count);
<br/>barrier：pthread_barrier_t结构体指针
<br/>attr：屏障属性结构体指针
<br/>count：屏障等待的线程数目，即要count个线程都到达屏障时，屏障才解除，线程就可以继续执行

### 2.等待
#include <pthread.h>
int pthread_barrier_wait(pthread_barrier_t *barrier);
函数的成功返回值有2个，第一个成功返回的线程会返回PTHREAD_BARRIER_SERIAL_THREAD，其他线程都返回0。可以用第一个成功返回的线程来做一些善后处理工作。

### 3.销毁屏障
#include <pthread.h>
int pthread_barrier_destroy(pthread_barrier_t *barrier);

## 三.简单例子                                           
写个简单的例子，主线程等待其他线程都完成工作后自己再向下执行，类似pthread_join()函数！
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
/* 屏障总数 */
#define PTHREAD_BARRIER_SIZE 4
/* 定义屏障 */
pthread_barrier_t barrier;
void err_exit(const char *err_msg){
    printf("error:%s\n", err_msg);
    exit(1);
}

void *thread_fun(void *arg){
    int result;
    char *thr_name = (char *)arg;
     /* something work */

    printf("线程%s工作完成...\n", thr_name);
 
    /* 等待屏障 */
    result = pthread_barrier_wait(&barrier);
    if (result == PTHREAD_BARRIER_SERIAL_THREAD)
        printf("线程%s，wait后第一个返回\n", thr_name);
    else if (result == 0)
        printf("线程%s，wait后返回为0\n", thr_name);
 
   return NULL;
}
 
int main(void){
     pthread_t tid_1, tid_2, tid_3;
 
     /* 初始化屏障 */
     pthread_barrier_init(&barrier, NULL, PTHREAD_BARRIER_SIZE);
 
     if (pthread_create(&tid_1, NULL, thread_fun, "1") != 0)
         err_exit("create thread 1");
 
     if (pthread_create(&tid_2, NULL, thread_fun, "2") != 0)
         err_exit("create thread 2");
 
     if (pthread_create(&tid_3, NULL, thread_fun, "3") != 0)
         err_exit("create thread 3"); 
     /* 主线程等待工作完成 */
     pthread_barrier_wait(&barrier);
     printf("所有线程工作已完成...\n");

     sleep(1);
     return 0;
 }
 
 
<br/>28行是线程自己要做的工作，62行的sleep(1)让所有线程有足够的时间把自己的返回值打印出来。编译运行：
<br/>可以看到，3个线程工作完成后才可以越过屏障打印返回值，第一个返回的是PTHREAD_BARRIER_SERIAL_THREAD，其他都是0。
<br/>这里有一点要注意：我们从运行结果看出，主线程打印"所有线程工作已完成"之后，线程1，线程2还在运行打印返回值。这个结果难免会误解"主线程等待所有线程完成工作之后再向下执行"。区分一点即可：等待只针对屏障之前的动作，越过屏障后，无论是主线程，还是子线程都会并发执行，如果非要让子线程完完全全执行完，可以再加个屏障到线程函数末尾，相应主线程也要加！

