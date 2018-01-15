## 线程同步——互斥量
    我们可以通过pthread提供的互斥量接口来保护我们的数据，确保每次只有一个线程访问。从本质上说就是一把锁，我们在访问共享数据的时候设置（上锁），在访问完成后释放（解锁）。当我们解锁互斥量的时候，如果有多余一个的线程被阻塞，则所有阻塞在这个锁的进程都被唤醒，变成可以运行的状态。接下来，只有一个线程开始运行并设置锁，其他的看到互斥量仍然是被锁定，继续等待。
 
### 1. 初始化:
    线程的互斥量数据类型是pthread_mutex_t. 在使用前, 要对它进行初始化:
    对于静态分配的互斥量, 可以初始化为PTHREAD_MUTEX_INITIALIZER（等价于 pthread_mutex_init(…, NULL)）或调用pthread_mutex_init。
    对于动态分配的互斥量, 在申请内存(malloc)之后, 通过pthread_mutex_init进行初始化, 并且在释放内存(free)前需要调用pthread_mutex_destroy.
 
原型:
int pthread_mutex_init(pthread_mutex_t *restrict mutex, const pthread_mutexattr_t *restric attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
返回值：成功则返回0, 出错则返回错误编号.
说明：1、如果使用默认的属性初始化互斥量, 只需把attr设为NULL。
      2、销毁一个互斥锁即意味着释放它所占用的资源，且要求锁当前处于开放状态。由于在Linux中，互斥锁并不占用任何资源，因此LinuxThreads中的pthread_mutex_destroy()除了检查锁状态以外（锁定状态则返回EBUSY）没有其他动作。
 
### 2. 互斥操作:
    对共享资源的访问, 要对互斥量进行加锁, 如果互斥量已经上了锁, 调用线程会阻塞, 直到互斥量被解锁。在完成了对共享资源的访问后, 要对互斥量进行解锁。
首先说一下加锁函数：
 
原型：
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
返回值：成功则返回0, 出错则返回错误编号.
说明：1、想给一个互斥量上锁，我们调用pthread_mutex_lock。如果mutex已经上锁，调用的线程将会被阻塞，直至信号量解锁。
      2、具体说一下trylock函数, 这个函数是非阻塞调用模式, 也就是说, 如果互斥量没被锁住, trylock函数将把互斥量加锁, 并获得对共享资源的访问权限；如果互斥量被锁住了, trylock函数将不会阻塞等待而直接返回EBUSY, 表示共享资源处于忙状态。
      3、要解锁一个信号量，我们调用phtread_mutex_unlock。
 
例子：
    我们使用mutex来保护数据结构：当多个进程需要访问动态申请的结构，我们嵌入了引用计数，来保证知道所有线程都使用完它时，我们才释放它。
#include <pthread.h>  
#include <stdlib.h>  
 
struct foo{  
    int f_count;  
    pthread_mutex_t f_lock;  
     
};  
 
struct foo * foo_alloc(void){  
    struct foo *fp;  
    if((fp = malloc(sizeof(struct foo))) != NULL){  
        fp->f_count = 1;  
        if(pthread_mutex_init(&fp->f_lock,NULL) != 0){  
            free(fp);  
            return NULL;  
        }  
    }   
    return fp;  
}  
 
void foo_hold(struct foo *fp){  
    pthread_mutex_lock(&fp->f_lock);  
    fp->f_count++;  
    pthread_mutex_unlock(&fp->f_lock);  
}  
 
void foo_rele(struct foo *fp){  
    pthread_mutex_lock(&fp->f_lock);  
    if(--fp->f_count == 0){  
        pthread_mutex_unlock(&fp->f_lock);  
        pthread_mutex_destroy(&fp->f_lock);  
        free(fp);  
    }else{  
        pthread_mutex_unlock(&fp->f_lock);  
    }  
} 
 
死锁:
    有时，可能需要同时访问两个资源。您可能正在使用其中的一个资源，随后发现还需要另一个资源。如果两个线程尝试声明这两个资源，但是以不同的顺序锁定与这些资源相关联的互斥锁，则会出现问题。例如，如果两个线程分别锁定互斥锁 1 和互斥锁 2，则每个线程尝试锁定另一个互斥锁时，将会出现死锁。下面的例子说明了可能的死锁情况。
线程 1	线程 2	
pthread_mutex_lock(&m1);
pthread_mutex_lock(&m2);
pthread_mutex_unlock(&m2);
pthread_mutex_unlock(&m1);	pthread_mutex_lock(&m2);
pthread_mutex_lock(&m1);
pthread_mutex_unlock(&m1);
pthread_mutex_unlock(&m2);	
    避免此问题的最佳方法是，确保线程在锁定多个互斥锁时，以同样的顺序进行锁定。如果始终按照规定的顺序锁定，就不会出现死锁。此方法称为锁分层结构，它通过为互斥锁指定逻辑编号来对这些锁进行排序。
    另外，请注意以下限制：如果您持有的任何互斥锁其指定编号大于 n，则不能提取指定编号为 n 的互斥锁。
    但是，不能始终使用此方法。有时，必须按照与规定不同的顺序提取互斥锁。要防止在这种情况下出现死锁，请使用 pthread_mutex_trylock()。如果线程发现无法避免死锁时，该线程必须释放其互斥锁。
  
    总体来讲, 有几个不成文的基本原则:
    对共享资源操作前一定要获得锁。
    完成操作以后一定要释放锁。
    尽量短时间地占用锁。
    如果有多锁, 如获得顺序是ABC连环扣, 释放顺序也应该是ABC。
    线程错误返回时应该释放它所获得的锁。

