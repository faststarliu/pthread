线程和信号

Pthread库也对线程和信号的处理提供了一些函数，这些函数包括：
int pthread_sigmask(int how,const sigset_t * newmask, sigset_t * oldmask);
int pthread_kill(pthread_t thread,int signo);
int sigwait(const sigset_t * set, int * sig);
以上这些函数包含在signal.h中。
这三个函数如果正确执行，返回值都为0。如果发生错误，则pthread_sigmask()和pthread_kill()函数返回值不为0,并且相应的错误代码被设置；sigwait()是线程取消点，总是返回0。
pthread_sigmask()函数用来改变或者设置线程的信号屏蔽(signal mask)；
1、	newmask用来保存将要执行的新的信号屏蔽集，以前的信号屏蔽集被存放到oldmask指向的位置。
2、	参数how。如果how是：
A、	SIG_SETMASK把信号屏蔽值集设置为newmask；
B、	SIG_BLOCK，那么在newmask中指定的信号就添加到了当前信号的屏蔽中；
C、	SIG_UNBLOCK，那么newmask中指定的信号从当前信号屏蔽中被删除；


sigwait函数；
sigwait()挂起调用sigwait()的线程，直到收到第一个参数set指向的信号集中指定的信号，且等待到信号被存放到第二个参数sig指向的位置。这里需要注意的是，在多线程情况下，执行sigwait()的时侯，sigwait()的第一个参数指向的信号集中的信号必须被阻塞。如果sigwait()等待的信号有相应的信号处理函数将不被调用。

在linux中，使用sigset_t数据类型存放信号集合。对信号集合的操作的GNU C library提供了一些函数：

int sigemptyset(sigset_t * set);
int sigfillset(sigset_t * set);
int sigaddset(sigset_t * set);
int sigdelset(sigset_t * set);
int sigismember(const sigset_t * set,int signo);
以上这些函数包含在signal.h中

其中；
sigemptyset把set指向的信号集清空；
sigfillset初始化信号集让其包括所有的信号；
sigaddset把信号signo添加到信号集中；
sigdelset是从信号集删除信号signo；
sigismember用来判断某个信号signo是否在信号集中。





int pthread_kill(pthread_t thread, int sig) 函数可以向其它线程发送信号； 
返回值：
成功:0
线程不存在：ESRCH
信号不合法：EINVAL

向指定ID的线程发送sig信号，如果线程代码内不做处理，则按照信号默认的行为影响整个进程，也就是说，如果你给一个线程发送了SIGQUIT，但线程却没有实现signal处理函数，则整个进程退出。
如果要获得正确的行为，就需要在线程内实现signal(SIGKILL,sig_handler)了。

所以，如果int sig的参数不是0，那一定要清楚到底要干什么，而且一定要实现线程的信号处理函数，否则，就会影响整个进程。


OK，如果int sig是0呢，这是一个保留信号，一个作用是用来判断线程是不是还活着。

所以，pthread_kill(threadid,0)就很有用啦。

int kill_rc = pthread_kill(thread_id,0);

if(kill_rc == ESRCH)
    printf("the specified thread did not exists or already quit/n");
else if(kill_rc == EINVAL)
    printf("signal is invalid/n");
else
    printf("the specified thread is alive/n");

上述的代码就可以判断线程是不是还活着了。


