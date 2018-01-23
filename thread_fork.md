 
 ## 概念：
 <br />     当一个线程调用函数fork的时候，整个进程地址空间会被拷贝到子进程中，在8.3节中有提到copy-on-write.子进程是一个与父进程完全不同的进程，但是如果父进程和子进程都没有对内存内容进行修改，那么该内存页就可以在父进程与子进程之间进行共享。 
 <br />     通过继承父进程的整个地址空间，子进程也会继承父进程每个互斥锁，读写锁以及条件变量的状态，如果父进程包含了多个线程，而且在fork函数返回之后并不会立即调用exec的话，子进程就需要清除锁状态。 
 <br />   在fork后的子进程内部，只会出现一个线程，它是父进程中调用fork函数的线程的拷贝。如果父进程中任何线程锁定了锁，相同的锁在子进程中也会处于锁定状态，问题是子进程并没有包含锁定锁的线程的拷贝，因此子进程没有办法知道哪一个锁需要锁定以及哪一个锁需要解除锁定。 
 <br />   上述问题可以通过如下方法避免：在fork之后调用函数exec，在这种情况下，老的地址空间将被抛弃，因此锁定状态并不重要。然而，这种方法并不总是可行的，如果子进程需要继续运行，那么我们就需要使用一个不同的策略。 
 <br />   为了避免在一个多线程进程中不一致的状态，POSIX.1指出在fork返回之后到exec函数之前的时间内只能调用异步信号安全的函数。这限制了子进程在调用exec之前可以做的事情，但是并不能解决子进程中锁状态的问题。 
为了清除锁状态，我们可以建立fork handler来进行处理。
## 小结:

也就是说，现在一个A父进程中有一个线程a1，这个线程执行需要占用锁M2，此时如果父进程执行fork创建一个子进程B，那么这个子进程B自然也会存在一个线程b1，因为这些都是从父进程中继承来的，如果父进程A中线程a1已经占有一个锁M1，那么子进程B中锁M1也是被占用了。这时候会导致什么问题呢?会导致子进程无法执行线程b1，因为这个锁被占用了，这是子进程从父进程继承而来就已经被物理内存上占了，所以子进程被死锁了，也就是说我们为什么要清理子进程中的锁问题?

## 事例分析:
前言：exce调用并不创建新进程，所以前后的进程ID并未改变，exec只是用一个全新的程序替换了当前进程的正文、数据、堆和栈段
多线程程序里不准使用fork :为什么？？？
UNIX上C++程序设计守则3
准则3：多线程程序里不准使用fork

在多线程程序里,在”自身以外的线程存在的状态”下一使用fork的话,就可能引起各种各样的问题.比较典型的例子就是,fork出来的子进程可能会死锁.请不要,在不能把握问题的原委的情况下就在多线程程序里fork子进程.

能引起什么问题呢?

那看看实例吧.一执行下面的代码,在子进程的执行开始处调用doit()时,发生死锁的机率会很高.
```
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
```

以下是说明死锁的理由：
一般的,fork做如下事情
   1. 父进程的内存数据会原封不动的拷贝到子进程中
   2. 子进程在单线程状态下被生成

在内存区域里,静态变量mutex的内存会被拷贝到子进程里.而且,父进程里即使存在多个线程,但它们也不会被继承到子进程里. fork的这两个特征就是造成死锁的原因.
译者注: 死锁原因的详细解释 ---
<br />   1. 线程里的doit()先执行.
<br />   2. doit执行的时候会给互斥体变量mutex加锁.
<br />   3. mutex变量的内容会原样拷贝到fork出来的子进程中(在此之前,mutex变量的内容已经被线程改写成锁定状态).
<br />   4.子进程再次调用doit的时候,在锁定互斥体mutex的时候会发现它已经被加锁,所以就一直等待,直到拥有该互斥体的进程释放它(实际上没有人拥有这个mutex锁).
<br />   5.线程的doit执行完成之前会把自己的mutex释放,但这是的mutex和子进程里的mutex已经是两份内存.所以即使释放了mutex锁也不会对子进程里的mutex造成什么影响.

例如,请试着考虑下面那样的执行流程,就明白为什么在上面多线程程序里不经意地使用fork就造成死锁了*3.
#### 1.    在fork前的父进程中,启动了线程1和2
#### 2.    线程1调用doit函数
#### 3.    doit函数锁定自己的mutex
#### 4.    线程1执行nanosleep函数睡10秒
#### 5.    在这儿程序处理切换到线程2
#### 6.    线程2调用fork函数
#### 7.    生成子进程
#### 8.    这时,子进程的doit函数用的mutex处于”锁定状态”,而且,解除锁定的线程在子进程里不存在
#### 9.    子进程的处理开始
#### 10.   子进程调用doit函数
#### 11.   子进程再次锁定已经是被锁定状态的mutex,然后就造成死锁

<br />像这里的doit函数那样的,在多线程里因为fork而引起问题的函数,我们把它叫做”fork-unsafe函数”.反之,不能引起问题的函数叫做”fork-safe函数”.虽然在一些商用的UNIX里,源于OS提供的函数(系统调用),在文档里有fork-safety的记载,但是在 Linux(glibc)里当然!不会被记载.即使在POSIX里也没有特别的规定,所以那些函数是fork-safe的,几乎不能判别.不明白的话,作为unsafe考虑的话会比较好一点吧.(2004/9/12追记)Wolfram Gloger说过,调用异步信号安全函数是规格标准,所以试着调查了一下,在pthread_atforkの这个地方里有” In the meantime*5, only a short list of async-signal-safe library routines are promised to be available.”这样的话.好像就是这样.

<br />随便说一下,malloc函数就是一个维持自身固有mutex的典型例子,通常情况下它是fork-unsafe的.依赖于malloc函数的函数有很多,例如printf函数等,也是变成fork-unsafe的.

<br />直到目前为止,已经写上了thread+fork是危险的,但是有一个特例需要告诉大家.
### ”fork后马上调用exec的场合,是作为一个特列不会产生问题的”. 什么原因呢..?
<br />exec函数*6一被调用,进程的”内存数据”就被临时重置成非常漂亮的状态.因此,即使在多线程状态的进程里,fork后不马上调用一切危险的函数,只是调用exec函数的话,子进程将不会产生任何的误动作.但是,请注意这里使用的”马上”这个词.即使exec前仅仅只是调用一回printf(“I’m child process”),也会有死锁的危险.
<br />注:exec函数里指明的命令一被执行,该命令的内存映像就会覆盖父进程的内存空间.所以,父进程里的任何数据将不复存在.

<br />文中理解：查看前面进程创建中，子进程在创建后，是写时复制的，也就是子进程刚创建时，与父进程一样的副本，当exce后，那么老的地址空间被丢弃，而被新的exec的命令的内存的印像覆盖了进程的内存空间，所以锁的状态无关紧要了。

### 如何规避灾难呢? 为了在多线程的程序中安全的使用fork,而规避死锁问题的方法有吗?试着考虑几个.

#### 规避方法1:做fork的时候,在它之前让其他的线程完全终止.
在fork之前,让其他的线程完全终止的话,则不会引起问题.但这仅仅是可能的情况.还有,因为一些原因而其他线程不能结束就执行了fork的时候,就会是产生出一些解析困难的不具合的问题.

#### 规避方法2:fork后在子进程中马上调用exec函数

<br />不用使用规避方法1的时候,在fork后不调用任何函数(printf等)就马上调用execl等,exec系列的函数.如果在程序里不使用”没有exec就fork”的话,这应该就是实际的规避方法吧.
<br />注:笔者的意思可能是把原本子进程应该做的事情写成一个单独的程序,编译成可执行程序后由exec函数来调用.

#### 规避方法3:”其他线程”中,不做fork-unsafe的处理
除了调用fork的线程,其他的所有线程不要做fork-unsafe的处理.为了提高数值计算的速度而使用线程的场合*7,这可能是fork- safe的处理,但是在一般的应用程序里则不是这样的.即使仅仅是把握了那些函数是fork-safe的,做起来还不是很容易的.fork-safe函数,必须是异步信号安全函数,而他们都是能数的过来的.因此,malloc/new,printf这些函数是不能使用的.
#### 规避方法4:使用pthread_atfork函数,在即将fork之前调用事先准备的回调函数.apue中详细介绍了它.

使用pthread_atfork函数,在即将fork之前调用事先准备的回调函数,在这个回调函数内,协商清除进程的内存数据.但是关于OS提供的函数 (例:malloc),在回调函数里没有清除它的方法.因为malloc里使用的数据结构在外部是看不见的.因此,pthread_atfork函数几乎是没有什么实用价值的.
```
int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void));
Returns: 0 if OK, error number on failure.
```
#### 使用函数pthread_atfork,我们可以建立起三个函数来帮组清除锁的锁定状态。
##### prepare函数在父进程调用函数fork创建子进程之前被父进程调用，该fork handler的作用是获取父进程定义的所有锁；
##### parent fork handler是在父进程fork创建子进程后，fork函数还没有返回之前在父进程中调用执行的，该fork handler的作用是解除父进程中所有prepare fork handler获取到的锁的锁定状态；
##### child fork handler在子进程中fork函数返回之前在子进程中调用，就像parent fork handler一样，child fork handler必须释放子进程中所有prepare fork handler获取到的锁。 
<br />注意，这些锁并没有被锁定一次，解锁两次，因为在子进程被创建的时候，它获取到了父进程定义的锁的所有状态(比如锁定了所有锁)，因为prepare锁定了所有锁，父进程和子进程会在相同的内存内容下开始运行，当父进程以及子进程分别解除它们锁的拷贝的锁定状态的时候，新的内存空间将被分配给子进程，并且父进程的内存内容将被拷贝到子进程(copy-on-write)，所以看起来就是父进程锁定了父进程以及子进程的所有的锁，然后父进程和子进程分别解除两份处于不同地址空间的锁的锁定状态，就像执行了如下的一个序列:

<br />父进程锁定所有的锁;
<br />子进程锁定所有的锁;
<br />父进程释放锁;
<br />子进程释放锁;
<br />我们可以调用函数pthread_atfork多次，从而可以创建多个fork handler的集合，如果我们不需要使用其中任何一个handlers,可以传入一个null指针即可，这并不会产生什么问题。当多个fork handlers被调用的时候，handlers被调用的顺序是不一样的，parent以及child fork handlers按照它们被注册的顺序进行调用，然而prepare函数会以它们被注册顺序的反序被调用。这一顺序允许多个模块注册它们自己的fork handlers并且保持锁定的层次结构。 
举例来讲，假设模块A调用模块B中的函数，同时两个模块都有各自的锁，如果锁定层次是A在B之前，模块B必须在模块A之前安装fork handlers.当父进程调用函数fork的时候，如下步骤将会被执行，假设子进程在父进程之前开始运行:

<br />模块A中的prepare fork handler被调用来获取模块A的锁;
<br />模块B中的prepare fork handler被调用来获取模块B的锁;
<br />子进程被创建;
<br />模块B的child fork handler被调用来释放子进程中所有模块B的锁;
<br />模块B的child fork handler被调用来释放子进程中所有模块A的锁;
<br />fork函数返回到子进程;
<br />模块B的parent fork handler被调用来释放所有模块B的锁;
<br />模块A的parent fork handler被调用来释放所有模块A的锁;
<br />fork函数返回到父进程;
#### 如果使用fork handlers来清除锁的状态的话，条件变量的状态需要如何清除呢？在一些实现上，条件变量可能并不需要任何清除工作，然而，使用锁作为条件变量的一部分的实现需要执行清除工作，但是问题是没有提供接口供我们实现清除工作，如果锁被嵌入到了条件变量的数据结构中，那么我们在调用fork函数以后不能在使用条件变量了(只是在子进程中由这一限制吧???),因为没有可移植的接口来清除其状态，另一方面，如果实现使用了一个全局锁来保护条件变量数据结构。那么实现本身可以在fork函数库中实现清理工作，然而，应用程序不应该依赖与这样的细节。

<br />就是不使用fork的方法.即用pthread_create来代替fork.这跟规避策2一样都是比较实际的方法,值得推荐.
<br />虽然pthread_atfork机制想要解决fork之后锁状态的问题，但是由于一些缺陷导致该机制只能在受限的环境下使用:

<br />就是不使用fork的方法.即用pthread_create来代替fork.这跟规避策2一样都是比较实际的方法,值得推荐.
<br />没有办法重新初始化复杂同步对象的状态，比如说条件变量以及barriers;
<br />一些实现对于互斥锁的错误检测机制在子进程尝试解锁一个其父进程锁定的互斥锁的时候会产生一个错误;
<br />递归锁不能在child fork handler中清除状态，因为没有办法知道递归锁被锁定的次数;
<br />如果子进程只允许调用异步信号安全函数，那么child fork handler都不能用于清除同步对象，因为这些函数全部都不是异步信号安全的。实际上可能出现这样的情况，同步对象在线程调用fork的时候可能处于中间状态，但是同步对象的清除工作要求其处于一致状态。
<br />如果应用程序在信号处理函数中调用fork函数(这是合法的，因为fork函数是异步信号安全的)，那么pthread_atfork注册的fork handlers只能调用异步信号安全函数，否则的话其结果是未定义的。




#### 规避方法5:在多线程程序里,不使用fork
<br />就是不使用fork的方法.即用pthread_create来代替fork.这跟规避策2一样都是比较实际的方法,值得推荐.

<br />*1：生成子进程的系统调用
<br />*2：全局变量和函数内的静态变量
<br />*3：如果使用Linux的话,查看pthread_atfork函数的man手册比较好.关于这些流程都有一些解释.
<br />*4：Solaris和HP-UX等
<br />*5：从fork后到exec执行的这段时间
<br />*6：≈execve系统调用
<br />*7：仅仅做四则演算的话就是fork-safe的




## 代码

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
