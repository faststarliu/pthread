# 线程取消(pthread_cancel) 
## 基本概念
pthread_cancel调用并不等待线程终止，它只提出请求。线程在取消请求(pthread_cancel)发出后会继续运行，
直到到达某个取消点(CancellationPoint)。取消点是线程检查是否被取消并按照请求进行动作的一个位置.

## 与线程取消相关的pthread函数
int pthread_cancel(pthread_t thread)
发送终止信号给thread线程，如果成功则返回0，否则为非0值。发送成功并不意味着thread会终止。

## 设置本线程对Cancel信号的反应，state有两种值：
int pthread_setcancelstate(int state,   int *oldstate)   
PTHREAD_CANCEL_ENABLE（缺省）
PTHREAD_CANCEL_DISABLE
分别表示收到信号后设为CANCLED状态和忽略CANCEL信号继续运行；old_state如果不为NULL则存入原来的Cancel状态以便恢复。   
返回：
         成功之后返回0。失败返回错误号，错误号说明如下：
         EINVAL：状态不是PTHREAD_CANCEL_ENABLE或者PTHREAD_CANCEL_DISABLE

## 设置本线程取消动作的执行时机，type由两种取值：
int pthread_setcanceltype(int type, int *oldtype)   
PTHREAD_CANCEL_DEFFERED;
PTHREAD_CANCEL_ASYCHRONOUS;
仅当Cancel状态为Enable时有效，分别表示收到信号后继续运行至下一个取消点再退出和立即执行取消动作（退出）；
oldtype如果不为NULL则存入运来的取消动作类型值。   
返回：
         成功之后返回0.失败返回错误号，错误号说明如下：
         EINVAL：状态不是PTHREAD_CANCEL_ASYNCHRONOUS或者PTHREAD_CANCEL_DEFERRED  



## 放置取消点:
void pthread_testcancel(void)；
是说pthread_testcancel在不包含取消点，但是又需要取消点的地方创建一个取消点，以便在一个没有包含取消点的执行代码线程中响应取消请求.
线程取消功能处于启用状态且取消状态设置为延迟状态时，pthread_testcancel()函数有效。如果在取消功能处处于禁用状态下调用pthread_testcancel()，则该函数不起作用。
请务必仅在线程取消线程操作安全的序列中插入pthread_testcancel()。除通过pthread_testcancel()调用以编程方式建立的取消点意外，pthread标准还指定了几个取消点。测试退出点,就是测试cancel信号.

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
    执行取消操作存在一定的危险。大多数危险都与完全恢复不变量和释放共享资源有关。取消线程时一定要格外小心，否则可能会使互斥保留为锁定状态，从而导致死锁状态。或者，已取消的线程可能保留已分配的内存区域，但是系统无法识别这一部分内存，从而无法释放它。
    标准C库指定了一个取消接口用于以编程方式允许或禁止取消功能。该库定义的取消点是一组可能会执行取消操作的点。该库还允许定义取消处理程序的范围，以确保这些处理程序在预期的时间和位置运行。取消处理程序提供的清理服务可以将资源和状态恢复到与起点一致的状态。
    必须对应用程序有一定的了解，才能放置取消点并执行取消处理程序。互斥肯定不是取消点，只应当在必要时使之保留尽可能短的时间。请将异步取消区域限制在没有外部依赖性的序列，因为外部依赖性可能会产生挂起的资源或未解决的状态条件。在从某个备用的嵌套取消状态返回时，一定要小心地恢复取消状态。该接口提供便于进行恢复的功能：pthread_setcancelstate(3C) 在所引用的变量中保留当前的取消状态，pthread_setcanceltype(3C) 以同样的方式保留当前的取消类型。
     在以下三种情况下可能会执行取消操作：
•	异步
•	执行序列中按照标准定义的点
•	调用pthread_cancel()
void pthread_testcancel(void);
当线程取消功能处于启用状态且取消状态设置为延迟状态时，pthread_testcancel()函数有效。如果在取消功能处处于禁用状态下调用pthread_testcancel()，则该函数不起作用。
请务必仅在线程取消线程操作安全的序列中插入pthread_testcancel()。除通过pthread_testcancel()调用以编程方式建立的取消点意外，pthread标准还指定了几个取消点。
测试退出点,就是测试cancel信号

