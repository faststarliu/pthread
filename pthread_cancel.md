# 线程取消(pthread_cancel) 
## 基本概念
pthread_cancel调用并不等待线程终止，它只提出请求。线程在取消请求(pthread_cancel)发出后会继续运行，
直到到达某个取消点(CancellationPoint)。取消点是线程检查是否被取消并按照请求进行动作的一个位置.

## 与线程取消相关的pthread函数
int pthread_cancel(pthread_t thread)
发送终止信号给thread线程，如果成功则返回0，否则为非0值。发送成功并不意味着thread会终止。
int pthread_setcancelstate(int state,   int *oldstate)   
## 设置本线程对Cancel信号的反应，state有两种值：
PTHREAD_CANCEL_ENABLE（缺省）
PTHREAD_CANCEL_DISABLE
分别表示收到信号后设为CANCLED状态和忽略CANCEL信号继续运行；old_state如果不为NULL则存入原来的Cancel状态以便恢复。   
int pthread_setcanceltype(int type, int *oldtype)   
## 设置本线程取消动作的执行时机，type由两种取值：
PTHREAD_CANCEL_DEFFERED;
PTHREAD_CANCEL_ASYCHRONOUS;
仅当Cancel状态为Enable时有效，分别表示收到信号后继续运行至下一个取消点再退出和立即执行取消动作（退出）；
oldtype如果不为NULL则存入运来的取消动作类型值。   
void pthread_testcancel(void)
是说pthread_testcancel在不包含取消点，但是又需要取消点的地方创建一个取消点，以便在一个没有包含取消点的执行代码线程中响应取消请求.
线程取消功能处于启用状态且取消状态设置为延迟状态时，pthread_testcancel()函数有效。
如果在取消功能处处于禁用状态下调用pthread_testcancel()，则该函数不起作用。
请务必仅在线程取消线程操作安全的序列中插入pthread_testcancel()。除通过pthread_testcancel()调用以编程方式建立的取消点意外，pthread标准还指定了几个取消点。测试退出点,就是测试cancel信号.

## 取消点:
线程取消的方法是向目标线程发Cancel信号，但如何处理Cancel信号则由目标线程自己决定，或者忽略、或者立即终止、或者继续运行至Cancelation-point（取消点），由不同的Cancelation状态决定。
线程接收到CANCEL信号的缺省处理（即pthread_create()创建线程的缺省状态）是继续运行至取消点，也就是说设置一个CANCELED状态，线程继续运行，只有运行至Cancelation-point的时候才会退出。
pthreads标准指定了几个取消点，其中包括：
(1)通过pthread_testcancel调用以编程方式建立线程取消点。 
(2)线程等待pthread_cond_wait或pthread_cond_timewait()中的特定条件。 
(3)被sigwait(2)阻塞的函数 
(4)一些标准的库调用。通常，这些调用包括线程可基于阻塞的函数。 
  
缺省情况下，将启用取消功能。有时，您可能希望应用程序禁用取消功能。如果禁用取消功能，则会导致延迟所有的取消请求，
直到再次启用取消请求。  
根据POSIX标准，pthread_join()、pthread_testcancel()、pthread_cond_wait()、pthread_cond_timedwait()、sem_wait()、sigwait()等函数以及
read()、write()等会引起阻塞的系统调用都是Cancelation-point，而其他pthread函数都不会引起Cancelation动作。
但是pthread_cancel的手册页声称，由于LinuxThread库与C库结合得不好，因而目前C库函数都不是Cancelation-point；但CANCEL信号会使线程从阻塞的系统调用中退出，并置EINTR错误码，因此可以在需要作为Cancelation-point的系统调用前后调用pthread_testcancel()，从而达到POSIX标准所要求的目标.
即如下代码段：
pthread_testcancel();
retcode = read(fd, buffer, length);
pthread_testcancel();
注意：
程序设计方面的考虑,如果线程处于无限循环中，且循环体内没有执行至取消点的必然路径，则线程无法由外部其他线程的取消请求而终止。因此在这样的循环体的必经路径上应该加入pthread_testcancel()调用.

## 取消类型(Cancellation Type)
我们会发现，通常的说法：某某函数是 Cancellation Points，这种方法是容易令人混淆的。
因为函数的执行是一个时间过程，而不是一个时间点。其实真正的 Cancellation Points 只是在这些函数中 Cancellation Type 被修改为 PHREAD_CANCEL_ASYNCHRONOUS 和修改回 PTHREAD_CANCEL_DEFERRED 中间的一段时间。
POSIX的取消类型有两种，一种是延迟取消(PTHREAD_CANCEL_DEFERRED)，这是系统默认的取消类型，即在线程到达取消点之前，不会出现真正的取消；另外一种是异步取消(PHREAD_CANCEL_ASYNCHRONOUS)，使用异步取消时，线程可以在任意时间取消。
