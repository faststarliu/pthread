 
 ## 概念：
 <br />     当一个线程调用函数fork的时候，整个进程地址空间会被拷贝到子进程中，在8.3节中有提到copy-on-write.子进程是一个与父进程完全不同的进程，但是如果父进程和子进程都没有对内存内容进行修改，那么该内存页就可以在父进程与子进程之间进行共享。 
 <br />     通过继承父进程的整个地址空间，子进程也会继承父进程每个互斥锁，读写锁以及条件变量的状态，如果父进程包含了多个线程，而且在fork函数返回之后并不会立即调用exec的话，子进程就需要清除锁状态。 
 <br />   在fork后的子进程内部，只会出现一个线程，它是父进程中调用fork函数的线程的拷贝。如果父进程中任何线程锁定了锁，相同的锁在子进程中也会处于锁定状态，问题是子进程并没有包含锁定锁的线程的拷贝，因此子进程没有办法知道哪一个锁需要锁定以及哪一个锁需要解除锁定。 
 <br />   上述问题可以通过如下方法避免：在fork之后调用函数exec，在这种情况下，老的地址空间将被抛弃，因此锁定状态并不重要。然而，这种方法并不总是可行的，如果子进程需要继续运行，那么我们就需要使用一个不同的策略。 
 <br />   为了避免在一个多线程进程中不一致的状态，POSIX.1指出在fork返回之后到exec函数之前的时间内只能调用异步信号安全的函数。这限制了子进程在调用exec之前可以做的事情，但是并不能解决子进程中锁状态的问题。 
为了清除锁状态，我们可以建立fork handler来进行处理。
## 小结:
也就是说，现在一个A父进程中有一个线程a1，这个线程执行需要占用锁M2，此时如果父进程执行fork创建一个子进程B，那么这个子进程B自然也会存在一个线程b1，因为这些都是从父进程中继承来的，如果父进程A中线程a1已经占有一个锁M1，那么子进程B中锁M1也是被占用了。这时候会导致什么问题呢?会导致子进程无法执行线程b1，因为这个锁被占用了，这是子进程从父进程继承而来就已经被物理内存上占了，所以子进程被死锁了，也就是说我们为什么要清理子进程中的锁问题?

事例分析:
前言：exce调用并不创建新进程，所以前后的进程ID并未改变，exec只是用一个全新的程序替换了当前进程的正文、数据、堆和栈段
多线程程序里不准使用fork :为什么？？？
UNIX上C++程序设计守则3
准则3：多线程程序里不准使用fork

在多线程程序里,在”自身以外的线程存在的状态”下一使用fork的话,就可能引起各种各样的问题.比较典型的例子就是,fork出来的子进程可能会死锁.请不要,在不能把握问题的原委的情况下就在多线程程序里fork子进程.

能引起什么问题呢?

那看看实例吧.一执行下面的代码,在子进程的执行开始处调用doit()时,发生死锁的机率会很高.
···
void* doit(void*) {
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&mutex);
    struct timespec ts = {10, 0}; nanosleep(&ts, 0); // 10秒寝る
                                                     // 睡10秒
    pthread_mutex_unlock(&mutex);
    return 0;
}

int main(void) {
        pthread_t t;

        pthread_create(&t, 0, doit, 0);                                 // 做成并启动子线程
        if (fork() == 0) {
              //子进程
             //在子进程被创建的瞬间,父的子进程在执行nanosleep的场合比较多
              doit(0);

              return 0;
        }
        pthread_join(t, 0); //
         // 等待子线程结束
}
···

以下是说明死锁的理由：
一般的,fork做如下事情
   1. 父进程的内存数据会原封不动的拷贝到子进程中
   2. 子进程在单线程状态下被生成

在内存区域里,静态变量mutex的内存会被拷贝到子进程里.而且,父进程里即使存在多个线程,但它们也不会被继承到子进程里. fork的这两个特征就是造成死锁的原因.
译者注: 死锁原因的详细解释 ---
   1. 线程里的doit()先执行.
   2. doit执行的时候会给互斥体变量mutex加锁.
   3. mutex变量的内容会原样拷贝到fork出来的子进程中(在此之前,mutex变量的内容已经被线程改写成锁定状态).
   4.子进程再次调用doit的时候,在锁定互斥体mutex的时候会发现它已经被加锁,所以就一直等待,直到拥有该互斥体的进程释放它(实际上没有人拥有这个mutex锁).
   5.线程的doit执行完成之前会把自己的mutex释放,但这是的mutex和子进程里的mutex已经是两份内存.所以即使释放了mutex锁也不会对子进程里的mutex造成什么影响.


```
#include "apue.h"
#include <pthread.h>

pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;

void
prepare(void)
{
	int err;

	printf("preparing locks...\n");
	if ((err = pthread_mutex_lock(&lock1)) != 0)
		err_cont(err, "can't lock lock1 in prepare handler");
	if ((err = pthread_mutex_lock(&lock2)) != 0)
		err_cont(err, "can't lock lock2 in prepare handler");
}

void
parent(void)
{
	int err;

	printf("parent unlocking locks...\n");
	if ((err = pthread_mutex_unlock(&lock1)) != 0)
		err_cont(err, "can't unlock lock1 in parent handler");
	if ((err = pthread_mutex_unlock(&lock2)) != 0)
		err_cont(err, "can't unlock lock2 in parent handler");
}

void
child(void)
{
	int err;

	printf("child unlocking locks...\n");
	if ((err = pthread_mutex_unlock(&lock1)) != 0)
		err_cont(err, "can't unlock lock1 in child handler");
	if ((err = pthread_mutex_unlock(&lock2)) != 0)
		err_cont(err, "can't unlock lock2 in child handler");
}

void *
thr_fn(void *arg)
{
	printf("thread started...\n");
	pause();
	return(0);
}

int
main(void)
{
	int			err;
	pid_t		pid;
	pthread_t	tid;

	if ((err = pthread_atfork(prepare, parent, child)) != 0)
		err_exit(err, "can't install fork handlers");
	if ((err = pthread_create(&tid, NULL, thr_fn, 0)) != 0)
		err_exit(err, "can't create thread");

	sleep(2);
	printf("parent about to fork...\n");

	if ((pid = fork()) < 0)
		err_quit("fork failed");
	else if (pid == 0)	/* child */
		printf("child returned from fork\n");
	else		/* parent */
		printf("parent returned from fork\n");
	exit(0);
}


```
