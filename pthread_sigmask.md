pthread_sigmask 控制线程的信号掩码

/* 示例一：屏蔽信号SIGINT 来源：http://www.leoox.com/?p=321 编译：gcc pthread_sigmask1.c -lpthread

运行后，你发现你按下CTRL+C(触发SIGINT信号)，这个程序根本停不下来。因为SIGINT信号已经如我们所愿被屏蔽掉了。 */

#include <pthread.h>  
#include <stdio.h>  
#include <sys/signal.h>  
#include <string.h>  

int main(int argc, char** argv)  
{  
  pthread_t tid = pthread_self();  

  sigset_t mask;  
  sigemptyset(&mask);  
  sigaddset(&mask, SIGINT);  
  pthread_sigmask(SIG_SETMASK, &mask, NULL);//SIG_BLOCK SIG_SETMASK 会屏蔽掉SIGINT，但SIG_UNBLOCK不会屏蔽SIGINT  

  printf("[main][%lu] working hard ...\n", tid);  

  sleep(60);  

  printf("[main][%lu] good bye and good luck!\n", tid);  
  return 0;  
}

示例二：主进程创建出来的线程将继承主进程的掩码 示例二：主进程创建出来的线程将继承主进程的掩码 来源：http://www.leoox.com/?p=321 编译：gcc pthread_sigmask2.c -lpthread

运行后，你可以发现，子线程果然接收不到pthread_kill发送给自己的SIGINT信号， 
但可以收到SIGUSR1信号！本来要休眠300s，但是收到了SIGUSR1信号， 
才休眠5秒就（提前294秒）醒来say goodbye了。 

运行结果：

[lgh@lghvm001 thread]$ ./a.out

[main][139999919077120] working hard ...

Thread[139999919068928] Running ......

[main][139999919077120] send signal SIGINT ...

Thread[139999919068928] catch signo = 10

Thread[139999919068928] waitup(294), and say good bye!

[main][139999919077120] good bye and good luck!

[lgh@lghvm001 thread]$

#include <pthread.h>  
#include <stdio.h>  
#include <sys/signal.h>  
#include <string.h>  

void handler(int signo)  
{  
  pthread_t tid = pthread_self();  
  printf("Thread[%lu] catch signo = %d\n", tid, signo);  
  return;  
}  

void* run(void *param)  
{  
  pthread_t tid = pthread_self();  
  printf(">>> Thread[%lu] Running ......\n", tid);  

  int rc = sleep(300);  
  printf("Thread[%lu] waitup(%d), and say good bye!\n", tid, rc); 
  return NULL;  
}  

int main(int argc, char** argv)  
{  
  int ret = 0, i = 0;  
  pthread_t tid = pthread_self();  

  /* 注册SIGUSR1信号处理函数 */  
  struct sigaction sa;  
  memset(&sa, 0, sizeof(sa));  
  sigemptyset(&sa.sa_mask);  
  sa.sa_flags = 0;  
  sa.sa_handler = handler;  
  sigaction(SIGUSR1, &sa, NULL);  

  /* 屏蔽信号SIGINT */  
  sigset_t mask;  
  sigemptyset(&mask);  
  sigaddset(&mask, SIGINT);  
  pthread_sigmask(SIG_BLOCK, &mask, NULL);  

  pthread_t threads[2];  
  pthread_create(&threads[0], NULL, run, NULL);  
  printf("[main][%lu] working hard ...\n", tid);  
  sleep(1);  

  /* 主进程创建出来的线程将继承主进程的掩码。所以子线程收不到SIGINT信号。 */  
  pthread_kill(threads[0], SIGINT);  
  printf("[main][%lu] send signal SIGINT ...\n", tid);  
  sleep(5);  

  /* 子线程可以收到SIGUSR1信号。 */  
  pthread_kill(threads[0], SIGUSR1);  

  pthread_join(threads[0], NULL);  

  sleep(1);  
  printf("[main][%lu] good bye and good luck!\n", tid);  
  return 0;  
}

示例三：子线程可以后天培养自己对信号的喜好

示例三：子线程可以后天培养自己对信号的喜好

来源：http://www.leoox.com/?p=321

编译：gcc pthread_sigmask3.c -lpthread

子线程天生继承了主线程对信号的喜好，但是自己可以通过后天的努力改变。 比如主线程喜欢SIGUSR1信号，但是子线程可以不喜欢它，屏蔽掉SIGUSR1信号。

由此可见，linux里的每个线程有自己的信号掩码，所以使用pthread_kill给指定线程发送信号时， 一定谨慎设置好线程的信号掩码。

当然，用kill发送信号，在多线程环境下，kill所产生的信号时传递到整个进程的， 并且所有线程都有机会收到这个信号，但具体是哪个线程处理这个信号，就不一定。 一般情况下，都是主线程处理这个信号。

运行结果：

[lgh@lghvm001 thread]$ gcc pthread_sigmask3.c -lpthread

[lgh@lghvm001 thread]$ ./a.out

[main][140613382825728] working hard ...

[1481543657]Thread[140613382817536] Running ......

[main][140613382825728] send signal SIGUSR1 ...

[1481543841]Thread[140613382825728] catch signo = 10 ... //子线程sleep期间,kill -SIGUSR1 2839

[1481543861]Thread[140613382825728] catch signo = 10 ... done

[1481543957]Thread[140613382817536] waitup(0), and say good bye!

[main][140613382825728] good bye and good luck!

[lgh@lghvm001 thread]$

#include <pthread.h>  
#include <stdio.h>  
#include <sys/signal.h>  
#include <string.h>  

void handler(int signo)  
{  
  pthread_t tid = pthread_self();  
  printf("[%u]Thread[%lu] catch signo = %d ...\n", time(NULL), tid, signo);  
  sleep(20);  
  printf("[%u]Thread[%lu] catch signo = %d ... done\n", time(NULL), tid, signo);  
  return;  
}  

void* run(void *param)  
{  
  pthread_t tid = pthread_self();  
  sigset_t mask;  
#if 1  
  /* 这种情况下，本线程屏蔽所有的信号 */  
  sigfillset(&mask);  
#endif  

#if 0  
  /* 这种情况下，本线程不屏蔽任何信号 */  
  sigemptyset(&mask);  
#endif  

#if 0   
  /* 这种情况，本线程屏蔽以下的指定信号 */  
  sigemptyset(&mask);  
  sigaddset(&mask, SIGINT);  
  sigaddset(&mask, SIGQUIT);  
  sigaddset(&mask, SIGHUP);  
  sigaddset(&mask, SIGTERM);  
#endif   

  pthread_sigmask(SIG_SETMASK, &mask, NULL);  

  printf(">>> [%u]Thread[%lu] Running ......\n", time(NULL), tid);  

  int rc = sleep(300);  

  printf("[%u]Thread[%lu] waitup(%d), and say good bye!\n", time(NULL), tid, rc);  
  return NULL;  
}  

int main(int argc, char** argv)  
{  
  pthread_t tid = pthread_self();  

  /* 注册SIGUSR1信号处理函数 */  
  struct sigaction sa;  
  memset(&sa, 0, sizeof(sa));  
  sigemptyset(&sa.sa_mask);  
  sa.sa_flags = 0;  
  sa.sa_handler = handler;  
  sigaction(SIGUSR1, &sa, NULL);  

  pthread_t threads[1];  
  pthread_create(&threads[0], NULL, run, NULL);  
  printf("[main][%lu] working hard ...\n", tid);  
  sleep(5);  

  /* 子线程屏蔽了SIGUSR1信号，所以子线程收不到SIGUSR1信号。 */  
  pthread_kill(threads[0], SIGUSR1);  
  printf("[main][%lu] send signal SIGUSR1 ...\n", tid);  
  sleep(5);  

  pthread_join(threads[0], NULL);  
  sleep(1);  

  printf("[main][%lu] good bye and good luck!\n", tid);  
  return 0;  
}

示例四：主线程收到信号，没处理完，又来一个信号，子线程会处理吗? 会！

示例四：主线程收到信号，没处理完，又来一个信号，子线程会处理吗? 会！

来源：http://www.leoox.com/?p=321

编译：gcc pthread_sigmask4.c -lpthread

在一个命令行终端启动a.out，在另一个命令行终端发送4次SIGUSR1信号给a.out进程

运行结果：

[lgh@lghvm001 thread]$ ./a.out

[main][139796483913472] working hard ...

[1481545031]Thread[139796483905280] Running ...... //此时发送四次kill -SIGUSR1 2886，2886为a.out的进程号

[1481545054]Thread[139796483913472] catch signo = 10 ...

[1481545055]Thread[139796483905280] catch signo = 10 ...

[1481545056]Thread[139796483913472] catch signo = 10 ... done

[1481545056]Thread[139796483913472] catch signo = 10 ...

[1481545057]Thread[139796483905280] catch signo = 10 ... done

[1481545057]Thread[139796483905280] catch signo = 10 ...

[1481545058]Thread[139796483913472] catch signo = 10 ... done

[1481545059]Thread[139796483905280] catch signo = 10 ... done

[1481545059]Thread[139796483905280] waitup(35), and say good bye!

[main][139796483913472] good bye and good luck!

[lgh@lghvm001 thread]$

#include <pthread.h>  
#include <stdio.h>  
#include <sys/signal.h>  
#include <string.h>  

void handler(int signo)  
{  
  pthread_t tid = pthread_self();  
  printf("[%u]Thread[%lu] catch signo = %d ...\n", time(NULL), tid, signo);  
  /* 
   * 信号处理函数休眠2秒，这期间再发送同一个信号，观察子线程的表现。 
   */  
  sleep(2);  
  printf("[%u]Thread[%lu] catch signo = %d ... done\n", time(NULL), tid, signo);  
  return;  
}  

void* run(void *param)  
{  
  pthread_t tid = pthread_self();  
  printf(">>> [%u]Thread[%lu] Running ......\n", time(NULL), tid);  

  int rc = sleep(60);  

  printf(">>> [%u]Thread[%lu] waitup(%d), and say good bye!\n", time(NULL), tid, rc);  
  return NULL;  
}  

int main(int argc, char** argv)  
{  
  pthread_t tid = pthread_self();  

  /* 注册SIGUSR1信号处理函数 */  
  struct sigaction sa;  
  memset(&sa, 0, sizeof(sa));  
  sigemptyset(&sa.sa_mask);  
  sa.sa_flags = 0;  
  sa.sa_handler = handler;  
  sigaction(SIGUSR1, &sa, NULL);  

  pthread_t threads[1];  
  pthread_create(&threads[0], NULL, run, NULL);  
  printf("[main][%lu] working hard ...\n", tid);  

  pthread_join(threads[0], NULL);  
  sleep(1);  

  printf("[main][%lu] good bye and good luck!\n", tid);  
  return 0;  
}

如果把void handler(int signo)函数中的sleep(2)注释掉，运行结果如下：

[lgh@lghvm001 thread]$ ./a.out

[main][140353429055232] working hard ...

[1481596389]Thread[140353429047040] Running ...... //此处发送6次“kill -SIGUSR1 3123”

[1481596401]Thread[140353429055232] catch signo = 10 ...

[1481596401]Thread[140353429055232] catch signo = 10 ... done

[1481596401]Thread[140353429055232] catch signo = 10 ...

[1481596401]Thread[140353429055232] catch signo = 10 ... done

[1481596402]Thread[140353429055232] catch signo = 10 ...

[1481596402]Thread[140353429055232] catch signo = 10 ... done

[1481596402]Thread[140353429055232] catch signo = 10 ...

[1481596402]Thread[140353429055232] catch signo = 10 ... done

[1481596402]Thread[140353429055232] catch signo = 10 ...

[1481596402]Thread[140353429055232] catch signo = 10 ... done

[1481596403]Thread[140353429055232] catch signo = 10 ...

[1481596403]Thread[140353429055232] catch signo = 10 ... done

[1481596449]Thread[140353429047040] waitup(0), and say good bye!

[main][140353429055232] good bye and good luck!

[lgh@lghvm001 thread]$

可见，若子线程“没空”(在sleep)，主线程有空，信号都给主线程处理了。 
