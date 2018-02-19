# thread-rwlock

# 线程同步——读写锁
读写锁也叫共享——排他锁，因为有3种状态, 所以可以有更高的并行性。
对比mutex，它的状态要么处于锁住和未锁状态，只有一个线程可以上锁。而读写锁有更多的状态：在读状态锁住，在写状态锁住，未锁住。只有一个线程可以获得写锁，多个线程可以同时获得读锁。
### 特性：
</br></br>•	当读写锁是写加锁状态时, 在这个锁被解锁之前, 所有试图对这个锁加锁的线程都会被阻塞。
</br>•	当读写锁在读加锁状态时, 所有试图以读模式对它进行加锁的线程都可以得到访问权, 但是如果线程希望以写模式对此锁进行加锁, 它必须阻塞知道所有的线程释放锁。
</br>•	通常, 当读写锁处于读模式锁住状态时, 如果有另外线程试图以写模式加锁, 读写锁通常会阻塞随后的读模式锁请求, 这样可以避免读模式锁长期占用, 而等待的写模式锁请求长期阻塞。
 
### 适用性：
    读写锁适合读比写频繁情形，最好读的数据比较大，也就是说如果不满足“读的次数远多于写”那么就不能发挥的写锁的特性，而且效率会比一般的锁低。如果锁持有的时间很快，读的时候只读取几十个字节的内存，还用读写锁，那么很可能会造成系统性能的下降，既然读取已经很快还有必要并发的读么？如果只是读取十几个字节还要排队竞争去读，读一次还需要个1秒钟，那么cpu一直在忙于读取（内存操作很快）和锁竞争，那么使用读锁前面的系统开销一点没有减少，反而可能因为使用读写锁会增加系统开销。读写锁和互斥量一样也需要在使用前初始化，在释放他们内存的时候销毁。
 
#### 初始化和销毁：
    int pthread_rwlock_init(pthread_rwlock_t *restrict rwlock, const pthread_rwlockattr_t *restrict attr); 
    int pthread_rwlock_destroy(pthread_rwlock_t *restrict rwlock);
    一个读写锁可以调用pthread_rwlock_init来初始化，我们可以传递NULL作为attr的参数，这样会使用读写锁的默认属性。我们可以调用pthread_rwlock_destroy来清理，销毁它所占的内存空间。
 
#### 读和写：
    int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);  
    int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);  
    实现上可能会对读写锁中读模式的锁锁住次数有一定的限制，所以我们需要检查返回值，以确定是否成功。而其他的两个函数会返回错误，但是只要我们的锁设计的恰当，我们可以不必做检查。
 
#### 非阻塞的函数为：
    int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock);  
    int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock);
    当锁成功获取时，返回0，否则返回EBUSY。这两个函数可以避免死锁。
如果针对未初始化的读写锁调用进行读写操作，则结果是不确定的。
	int pthread_rwlock_timedrdlock(pthread_rwlock_t *restrict rwlock, const struct timespec *restrict abs_timeout);
	int pthread_rwlock_timedwrlock(pthread_rwlock_t *restrict rwlock, const struct timespec *restrict abs_timeout);
 try类函数加锁：如果获取不到锁，会立即返回错误EBUSY！
timed类函数加锁：如果规定的时间内获取不到锁，会返回ETIMEDOUT错误！

#### 释放：
    int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);
    用来释放在 rwlock 引用的读写锁对象中持有的锁。
    如果调用线程未持有读写锁 rwlock，或者针对未初始化的读写锁调用该函数，则结果是不确定的。
### 案例                                        
创建4个线程，2个线程读锁，2个线程写锁，观察4个线程进入临界区的顺序：

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/* 初始化读写锁 */
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
/* 全局资源 */
int global_num = 10;

void err_exit(const char *err_msg)
{
    printf("error:%s\n", err_msg);
    exit(1);
}

/* 读锁线程函数 */
void *thread_read_lock(void *arg)
{
    char *pthr_name = (char *)arg;

    while (1)
    {
        /* 读加锁 */
        pthread_rwlock_rdlock(&rwlock);

        printf("thread_read_lock %s in, global_num = %d\n", pthr_name, global_num);
        sleep(1);
        printf("thread_read_lock %s out.\n", pthr_name);

        /* 读解锁 */
        pthread_rwlock_unlock(&rwlock);

        sleep(1);
    }

    return NULL;
}

/* 写锁线程函数 */
void *thread_write_lock(void *arg)
{
    char *pthr_name = (char *)arg;

    while (1)
    {
        /* 写加锁 */
        pthread_rwlock_wrlock(&rwlock);

        /* 写操作 */
        global_num++;
        printf("thread_write_lock %s in, global_num = %d\n", pthr_name, global_num);
        sleep(1);
        printf("thread_write_lock %s out...\n", pthr_name);

        /* 写解锁 */
        pthread_rwlock_unlock(&rwlock);

        sleep(2);
    }

    return NULL;
}

int main(void)
{
    pthread_t tid_read_1, tid_read_2, tid_write_1, tid_write_2;

    /* 创建4个线程，2个读，2个写 */
    if (pthread_create(&tid_read_1, NULL, thread_read_lock, "read_1") != 0)
        err_exit("create tid_read_1");

    if (pthread_create(&tid_read_2, NULL, thread_read_lock, "read_2") != 0)
        err_exit("create tid_read_2");

    if (pthread_create(&tid_write_1, NULL, thread_write_lock, "write_1") != 0)
        err_exit("create tid_write_1");

    if (pthread_create(&tid_write_2, NULL, thread_write_lock, "write_2") != 0)
        err_exit("create tid_write_2");

    /* 随便等待一个线程，防止main结束 */
    if (pthread_join(tid_read_1, NULL) != 0)
        err_exit("pthread_join()");
	pthread_rwlock_destroy(&rwlock);

    return 0;
}
2个线程函数的临界区里面都sleep(1)，测试给足够的时间看其他线程能不能进来。64行，写锁函数里面，sleep(2)，因为写锁请求会阻塞后面的读锁，2个写锁一起请求会让读锁饥饿，所以比39行的sleep(1)多一秒！

可以看到，读锁可以一起进入临界区，而写锁在临界区里面等1秒都不会有其他线程能进来！！！ 



