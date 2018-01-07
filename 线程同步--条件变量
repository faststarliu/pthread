# 线程同步——条件变量
## 1.互斥量的存在问题：
    互斥量是线程程序必需的工具，但它们并非万能的。例如，如果线程正在等待共享数据内某个条件出现，那会发生什么呢？它可以重复对互斥对象锁定和解锁，每次都会检查共享数据结构，以查找某个值。但这是在浪费时间和资源，而且这种繁忙查询的效率非常低。
    在每次检查之间，可以让调用线程短暂地进入睡眠，比如睡眠三秒钟，但是因此线程代码就无法最快作出响应。真正需要的是这样一种方法：当线程在等待满足某些条件时使线程进入睡眠状态。一旦条件满足，就唤醒因等待满足特定条件而睡眠的线程。如果能够做到这一点，线程代码将是非常高效的，并且不会占用宝贵的互斥对象锁。这正是 POSIX 条件变量能做的事！
 
## 2.条件变量:
    条件变量通过允许线程阻塞和等待另一个线程发送信号的方法弥补了互斥锁的不足，它常和互斥锁一起使用。使用时，条件变量被用来阻塞一个线程，当条件不满足时，线程往往解开相应的互斥锁并等待条件发生变化。一旦其它的某个线程改变了条件变量，它将通知相应的条件变量唤醒一个或多个正被此条件变量阻塞的线程。这些线程将重新锁定互斥锁并重新测试条件是否满足。
 
## 3.条件变量的相关函数
### （1）创建和注销
    条件变量和互斥锁一样，都有静态动态两种创建方式
#### a.静态方式
    静态方式使用PTHREAD_COND_INITIALIZER常量，如下：
    pthread_cond_t cond=PTHREAD_COND_INITIALIZER
 
#### b.动态方式
    int pthread_cond_init(pthread_cond_t *cond, pthread_condattr_t *cond_attr)
    使用 cond_attr 指定的属性初始化条件变量 cond，当 cond_attr 为 NULL 时，使用缺省的属性。LinuxThreads 实现条件变量不支持属性，因此 cond_attr 参数实际被忽略。

#### c.注销
    int pthread_cond_destroy(pthread_cond_t *cond)
    注销一个条件变量需要调用pthread_cond_destroy()，只有在没有线程在该条件变量上等待的时候才能注销这个条件变量，否则返回EBUSY。因为Linux实现的条件变量没有分配什么资源，所以注销动作只包括检查是否有等待线程。
 
### （2）等待和激发
#### a.等待
    int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
    了解 pthread_cond_wait() 的作用非常重要——它是 POSIX 线程信号发送系统的核心，也是最难以理解的部分。
    首先，让我们考虑以下情况：线程为查看已链接列表而锁定了互斥对象，然而该列表恰巧是空的。这一特定线程什么也干不了——其设计意图是从列表中除去节点，但是现在却没有节点。因此，它只能：
    锁定互斥对象时，线程将调用 pthread_cond_wait(&mycond,&mymutex)。pthread_cond_wait() 调用相当复杂，因此我们每次只执行它的一个操作。
    pthread_cond_wait() 所做的第一件事就是同时对互斥对象解锁（于是其它线程可以修改已链接列表），并等待条件 mycond 发生（这样当 pthread_cond_wait() 接收到另一个线程的“信号”时，它将苏醒）。现在互斥对象已被解锁，其它线程可以访问和修改已链接列表，可能还会添加项。
    此时，pthread_cond_wait() 调用还未返回。对互斥对象解锁会立即发生，但等待条件 mycond 通常是一个阻塞操作，这意味着线程将睡眠，在它苏醒之前不会消耗 CPU 周期。这正是我们期待发生的情况。线程将一直睡眠，直到特定条件发生，在这期间不会发生任何浪费 CPU 时间的繁忙查询。从线程的角度来看，它只是在等待 pthread_cond_wait() 调用返回。
    现在继续说明，假设另一个线程（称作“2 号线程”）锁定了 mymutex 并对已链接列表添加了一项。在对互斥对象解锁之后，2 号线程会立即调用函数 pthread_cond_broadcast(&mycond)。此操作之后，2 号线程将使所有等待 mycond 条件变量的线程立即苏醒。这意味着第一个线程（仍处于 pthread_cond_wait() 调用中）现在将苏醒。
    现在，看一下第一个线程发生了什么。您可能会认为在2号线程调用 pthread_cond_broadcast(&mymutex) 之后，1 号线程的 pthread_cond_wait() 会立即返回。不是那样！实际上，pthread_cond_wait() 将执行最后一个操作：重新锁定 mymutex。一旦 pthread_cond_wait() 锁定了互斥对象，那么它将返回并允许 1 号线程继续执行。那时，它可以马上检查列表，查看它所感兴趣的更改。
 
回顾！
那个过程非常复杂，因此让我们先来回顾一下。第一个线程首先调用：
    pthread_mutex_lock(&mymutex);
 
然后，它检查了列表。没有找到感兴趣的东西，于是它调用：
    pthread_cond_wait(&mycond, &mymutex);

然后，pthread_cond_wait() 调用在返回前执行许多操作：
    pthread_mutex_unlock(&mymutex);
 
它对 mymutex 解锁，然后进入睡眠状态，等待 mycond 以接收 POSIX 线程“信号”。一旦接收到“信号”（加引号是因为我们并不是在讨论传统的 UNIX 信号，而是来自 pthread_cond_signal() 或 pthread_cond_broadcast() 调用的信号），它就会苏醒。但 pthread_cond_wait() 没有立即返回——它还要做一件事：重新锁定 mutex：
    pthread_mutex_lock(&mymutex);
 
pthread_cond_wait() 知道我们在查找 mymutex “背后”的变化，因此它继续操作，为我们锁定互斥对象，然后才返回。
 
    int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
    pthread_cond_timedwait 和 pthread_cond_wait 一样，自动解锁互斥量及等待条件变量，但它还限定了等待时间。如果在 abstime 指定的时间内 cond 未触发，互斥量 mutex 被重新加锁，并返回错误 ETIMEDOUT。abstime 参数指定一个绝对时间，时间原点与 time 和 gettimeofday 相同：abstime = 0 表示 1970 年 1 月 1 日 00:00:00 GMT。
 
例如，要等待 5 秒，这样处理：
   struct timeval now;
   struct timespec timeout;

   gettimeofday(&now);
   timeout.tv_sec = now.tv_sec + 5;
   timeout.tv_nsec = now.tv_usec * 1000;
其中：
struct timeval {
　　time_t tv_sec;
　　suseconds_t tv_usec;
　　};
struct timespec {
    time_t   tv_sec;
    long     tv_nsec;
    };
 
b.激发
    int pthread_cond_signal(pthread_cond_t *cond);
    int pthread_cond_broadcast(pthread_cond_t *cond);
    激发条件有两种形式，pthread_cond_signal()激活一个等待该条件的线程，多个线程阻塞在此条件变量上时，哪一个线程被唤醒是由线程的调度策略所决定的；而pthread_cond_broadcast()则激活所有等待线程，这些线程被唤醒后将再次竞争相应的互斥锁。
    要注意的是，必须用保护条件变量的互斥锁来保护激活函数，否则条件满足信号有可能在测试条件和调用pthread_cond_wait()函数之间被发出，从而造成无限制的等待。
 
## 4、例子
    a、建立两个线程1、2，两个线程分别访问共享资源，并进行加1操作，当共享资源<=3时，线程1挂起不操作，这时线程2工作，共享资源>3后，两者都工作。
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
 
int gnum = 0;
pthread_mutex_t mutex;
pthread_cond_t  cond;
 
static void pthread_func_1 (void);
static void pthread_func_2 (void);
 
int main (void)
{
    pthread_t pt_1 = 0;
    pthread_t pt_2 = 0;
    int ret = 0;
 
    pthread_mutex_init (&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    ret = pthread_create (&pt_1, NULL, (void *)pthread_func_1, NULL);
    if (ret != 0)
    {
        perror ("pthread_1_create");
    }
      
    ret = pthread_create (&pt_2, NULL, (void *)pthread_func_2, NULL);
    if (ret != 0)     
    {
        perror ("pthread_2_create");
    }
    pthread_join (pt_1, NULL);
    pthread_join (pt_2, NULL);
      
    printf ("main programme exit!\n");
    return 0;
}
  
static void pthread_func_1 (void)     
{     
    int i = 0;
      
    for (;;)
    {
        printf ("This is pthread1!\n");
        pthread_mutex_lock(&mutex);
       
        //注意，这里以防线程的抢占，以造成一个线程在另一个线程sleep时多次访问互斥资源，所以sleep要在得到互斥锁后调用 
        sleep (1);
 
        //条件变量，当gnum<=3时，线程1自己挂起并且解锁，让线程2进去
        while (gnum <= 3) {
            pthread_cond_wait(&cond, &mutex);
        }  
 
        //当线程2调用pthread_cond_signal(&cond)时，线程1在这里重启
        //临界资源
        gnum++;
        printf ("Thread1 add one to num:%d\n",gnum);
        pthread_mutex_unlock(&mutex);
    }     
}
  
static void pthread_func_2 (void)     
{     
    int i = 0;     
      
    for (;;)     
    {     
        printf ("This is pthread2!\n");
        pthread_mutex_lock(&mutex);
        //注意，这里以防线程的抢占，以造成一个线程在另一个线程sleep时多次访问互斥资源，所以sleep要在得到互斥锁后调用 
        sleep (1);
 
        //临界资源
        gnum++;
        printf ("Thread2 add one to num:%d\n",gnum);
        //当gnum == 4时，触发
        if (gnum == 4)   
            pthread_cond_signal(&cond);
          
        pthread_mutex_unlock(&mutex);             
    }      
    pthread_exit (0);     
}
 
b、
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
 
void *thread1(void *);
void *thread2(void *);
 
int i=1;

int main(void)
{
    pthread_t t_a;
    pthread_t t_b;
 
    pthread_create(&t_a,NULL,thread1,(void *)NULL);
    pthread_create(&t_b,NULL,thread2,(void *)NULL);
    pthread_join(t_b, NULL);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    exit(0);
}
 
void *thread1(void *junk)
{
    for(i=1;i<=9;i++)
    {
        pthread_mutex_lock(&mutex);
        if(i%3==0)
           pthread_cond_signal(&cond);
        else       
           printf("thead1:%d\n",i);
        pthread_mutex_unlock(&mutex);
  
  sleep(1);
 } 
}
 
void *thread2(void *junk)
{
    while(i<=9)
    {
    pthread_mutex_lock(&mutex);
  
    if(i%3!=0)
        pthread_cond_wait(&cond,&mutex);
    printf("thread2:%d\n",i);
    pthread_mutex_unlock(&mutex);
  
  sleep(1);
 } 
}
    程序创建了2个新线程使他们同步运行，实现进程t_b打印10以内3的倍数，t_a打印其他的数，程序开始线程t_b不满足条件等待，线程t_a运行使a循环加1并打印。直到i为3的倍数时，线程t_a发送信号通知进程t_b，这时t_b满足条件，打印i值。
下面是运行结果：
#cc –lpthread –o cond cond.c
#./cond
thread1:1
thread1:2
thread2:3
thread1:4
thread1:5
thread2:6
thread1:7
thread1:8
thread2:9

