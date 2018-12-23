# 线程特定数据(TSD)
## 首先，为什么要使用线程特定数据呢？什么是线程特定数据。这牵涉到重入函数和不可重入函数。
### 1、线程特定数据几个函数
<br />int pthread_key_create(pthread_key_t *key， void (*destr_function) (void *))；  //该函数有两个参数，第一个参数就是上面声明的 pthread_key_t 变量，第二个参数是一个清理函数，用来在线程释放该线程存储的时候被调用。该函数指针可以设成 NULL ，这样系统将调用默认的清理函数。
<br />int pthread_key_delete(pthread_key_t key)；  
<br />int pthread_setspecific(pthread_key_t key， const void *pointer)； //该函数有两个参数，第一个为前面声明的 pthread_key_t 变量，第二个为 void* 变量，这样你可以存储任何类型的值。 
<br />void * pthread_getspecific(pthread_key_t key)； //该函数的参数为前面提到的 pthread_key_t 变量，该函数返回 void * 类型的值。 
<br />pthread_once_t   once_control = PTHREAD_ONCE_INIT；  
<br />int pthread_once(pthread_once_t *once_control， void (*init_routine) (void))；  //初始化函数， 将对key的初始化放入该函数中，可以保证inti_routine函数只运行一次。
 <br />    在单线程程序中，我们经常要用到"全局变量"以实现多个函数间共享数据， 然而在多线程环境下，由于数据空间是共享的，因此全局变量也为所有线程所共有。但有时应用程序设计中有必要提供线程私有的全局变量，仅在某个线程中有效，但却可以跨多个函数访问。POSIX线程库通过维护一定的数据结构来解决这个问题，这个些数据称为（Thread-specific-data或 TSD）， 线程特定数据如下图所示:
  
 
 <br />    从上图可知:当调用pthread_key_create 后会产生一个所有线程都可见的线程特定数据（TSD）的键值(如上图中所有的线程都会得到一个pkey[1]的值)， 但是这个键所指向的真实数据却是不同的，虽然都是pkey[1]， 但是他们并不是指向同一块内存，而是指向了只属于自己的实际数据， 因此， 如果线程0更改了pkey[1]所指向的数据， 而并不能够影像到线程n；
<br />   在线程调用pthread_setspecific后会将每个线程的特定数据与thread_key_t绑定起来，虽然只有一个pthread_key_t，但每个线程的特定数据是独立的内存空间，当线程退出时会执行destructor 函数。
### 2、	线程私有数据实现的主要思想：
<br />在分配线程私有数据之前，创建与该数据相关联的键，这个键可以被进程中的所有线程使用，但每个线程把这个键与不同的线程私有数据地址进行关联，需要说明的是每个系统支持有限数量的线程特定数据元素（如:限制为128个）。那么这个键的实现原理是什么呢？
   其实系统为每个进程维护了一个称之为Key结构的结构数组，如下图所示：
![image](https://github.com/faststarliu/pthread/blob/master/1.png)
		
<br />     在上图中Key 结构的“标志”指示这个数据元素是否正在使用。在刚开始时所有的标志初始化为“不在使用”。当一个线程调用pthread_key_create创建一个新的线程特定数据元素时，系统会搜索Key结构数组，找出第一个“不在使用”的元素。并把该元素的索引(0~127，称为“键”)返回给调用线程。
<br />      除了进程范围内的Key结构数组之外，系统还在进程内维护了关于多个线程的多条信息。这些特定于线程的信息我们称之为pthread结构。其中部分内容是我们称之为pkey数组的一个128个元素的指针数组。系统维护的关于每个线程的信息结构图如下：
![image](https://github.com/faststarliu/pthread/blob/master/2.png)
		
 <br />    在上图中，pkey数组所有元素都被初始化为空指针。这些128个指针是和进程内128个可能的键逐一关联的值。
那么当我们调用pthread_key_create函数时，系统会为我们做什么呢？
<br />      系统首先会返回给我们一个Key结构数组中第一个“未被使用”的键（即索引值），每个线程可以随后通过该键找到对应的位置，并且为这个位置存储一个值（指针）。 一般来说，这个指针通常是每个线程通过调用malloc来获得的。
<br />知道了大概的私有数据实现的原理，那么在编程中如何使用线程的特定数据呢？
 <br />    假设一个进程被启动，并且多个线程被创建。 其中一个线程调用pthread_key_create。系统在Key结构数组（图1）中找到第1个未使用的元素。并把它的索引（0~127）返回给调用者。我们假设找到的索引为1。之后线程调用pthread_getspecific获取本线程的pkey[1] 的值（图（2）中键1所值的指针）， 返回值是一个空值，线程那么调用malloc分配内存区并初始化此内存区。 之后线程调用pthread_setspecific把对应的所创建键的线程特定数据指针(pkey[1]) 设置为指向它刚刚分配的内存区。下图指出了此时的情形。
 ![image](https://github.com/faststarliu/pthread/blob/master/3.png)
  
<br />明白了怎样获取线程的特定数据值，那么如果线程终止时系统会执行什么操作呢？
<br />     我们知道，一个线程调用pthread_key_create创建某个特定的数据元素时，所指定的参数之一便是指向析构函数的指针。当一个线程终止时，系统将扫描该线程的pkey数组，为每个非空的pkey指针调用相应的析构函数。 相应的析构函数是存放在图1中的Key数组中的函数指针。这是一个线程终止时其线程特定数据的释放手段。
### 3、重入函数
<br />   重入函数就是在多个进程或者线程中，可以同时进行运行的函数，不可重入函数就是不可以同时运行的函数，这个主要是可能多个进程或线程共享了一个变量，这个变量只有一个，这样同时运行的时候，就会出问题了，因为我们不知道这个静态变量具体是存入的什么值，可能刚存入一个值，立马又因为调用这个函数编程另外一个值。更加通俗的说，在函数体内不访问那些全局变量，不使用静态局部变量，坚持只使用局部变量，写出的函数就将是可重入的。如果必须访问全局变量，记住利用互斥信号量来保护全局变量

### 4、pthread_once 函数：
 <br />     在多线程环境中，有些事仅需要执行一次。通常当初始化应用程序时，可以比较容易地将其放在main函数中。但当你写一个库时，就不能在main里面初始化了，你可以用静态初始化，但使用一次初始化（pthread_once）会比较容易些。

<br />int pthread_once(pthread_once_t *once_control， void (*init_routine) (void))；
<br />功能：本函数使用初值为PTHREAD_ONCE_INIT的once_control变量保证init_routine()函数在本进程执行序列中仅执行一次。
<br />   在多线程编程环境下，尽管pthread_once()调用会出现在多个线程中，init_routine()函数仅执行一次，究竟在哪个线程中执行是不定的，是由内核调度来决定。
<br />Linux Threads使用互斥锁和条件变量保证由pthread_once()指定的函数执行且仅执行一次，而once_control表示是否执行过。
<br />如果once_control的初值不是PTHREAD_ONCE_INIT（Linux Threads定义为0），pthread_once() 的行为就会不正常。
<br />在LinuxThreads中，实际"一次性函数"的执行状态有三种：
<br />NEVER（0）；
<br />IN_PROGRESS（1）；
<br />DONE （2）；
<br />如果once初值设为1，则由于所有pthread_once()都必须等待其中一个激发"已执行一次"信号，因此所有pthread_once ()都会陷入永久的等待中；如果设为2，则表示该函数已执行过一次，从而所有pthread_once()都会立即返回0。
```
/** 示例1: 设置/获取线程特定数据
在两个线程中分别设置/获取线程特定数据， 查看两个线程中的数据是否是一样的(肯定是不一样的O(∩_∩)O~)
**/
pthread_key_t key；
typedef struct Tsd
{
pthread_t tid；
char *str；
} tsd_t；
//用来销毁每个线程所指向的实际数据
void destructor_function(void *value)
{
free(value)；
printf("destructor ...\n" )；
}

void *thread_routine(void *args)
{
//设置线程特定数据
tsd_t *value = (tsd_t *)malloc(sizeof(tsd_t))；
value->tid = pthread_self()；
value->str = (char *)args；
pthread_setspecific(key， value)；
printf("%s setspecific， address: %p\n"， (char *)args， value)；

//获取线程特定数据
value = (tsd_t *)pthread_getspecific(key)；
printf("tid: 0x%x， str = %s\n"， (unsigned int)value->tid， value->str)；
sleep(2)；

//再次获取线程特定数据
value = (tsd_t *)pthread_getspecific(key)；
printf("tid: 0x%x， str = %s\n"， (unsigned int)value->tid， value->str)；

pthread_exit(NULL)；
}

int main()
{
//这样每个线程当中都会有一个key可用了，
//但是每个key所绑定的实际区域需要每个线程自己指定
pthread_key_create(&key， destructor_function)；

pthread_t tid1， tid2；
pthread_create(&tid1， NULL， thread_routine， (void *)"thread1")；
pthread_create(&tid2， NULL， thread_routine， (void *)"thread2")；

pthread_join(tid1， NULL)；
pthread_join(tid2， NULL)；
pthread_key_delete(key)；
return 0；
}
/** 示例2:运用pthread_once， 让key只初始化一次
注意: 将对key的初始化放入到init_routine中
**/
pthread_key_t key；
pthread_once_t once_control = PTHREAD_ONCE_INIT；
typedef struct Tsd
{
pthread_t tid；
char *str；
} tsd_t；

//线程特定数据销毁函数，
//用来销毁每个线程所指向的实际数据
void destructor_function(void *value)
{
free(value)；
printf("destructor ...\n" )；
}

//初始化函数， 将对key的初始化放入该函数中，
//可以保证inti_routine函数只运行一次
void init_routine()
{
pthread_key_create(&key， destructor_function)；
printf("destructor ...\n" )；
}

void *thread_routine(void *args)
{
pthread_once(&once_control， init_routine)；

//设置线程特定数据
tsd_t *value = (tsd_t *)malloc(sizeof(tsd_t))；
value->tid = pthread_self()；
value->str = (char *)args；
pthread_setspecific(key， value)；
printf("%s setspecific， address: %p\n"， (char *)args， value)；

//获取线程特定数据
value = (tsd_t *)pthread_getspecific(key)；
printf("tid: 0x%x， str = %s\n"， (unsigned int)value->tid， value->str)；
sleep(2)；

//再次获取线程特定数据
value = (tsd_t *)pthread_getspecific(key)；
printf("tid: 0x%x， str = %s\n"， (unsigned int)value->tid， value->str)；

pthread_exit(NULL)；
}

int main()
{
pthread_t tid1， tid2；
pthread_create(&tid1， NULL， thread_routine， (void *)"thread1")；
pthread_create(&tid2， NULL， thread_routine， (void *)"thread2")；
pthread_join(tid1， NULL)；
pthread_join(tid2， NULL)；
pthread_key_delete(key)；
return 0；
}
 
```
