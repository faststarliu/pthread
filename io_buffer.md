

## 概述
 
标准I/O库提供缓冲的目的是尽可能地减少使用read和write调用的次数。他也对每个I/O流自动地进行缓冲管理，从而避免了应用程序需要考虑这一点所带来的麻烦。
不幸的是，标准I/O库最令人迷惑的也是他的缓冲。
## 标准I/O提供了三种类型的缓冲：
### 1、全缓冲。这种情况下，在填满标准I/O缓冲区后才进行实际I/O操作。对于驻留在磁盘上的文件通常是由标准I/O库实施全缓冲。一个流上执行第一次I/O操作时，相关标准I/O函数通常调用malloc获得需使用的缓冲区。

术语冲洗说明I/O缓冲区的写操作。缓冲区可由标准I/O例程自动冲洗，或者可以调用函数fflush冲洗一个流。值得引起注意的是在UNIX环境 中，flush有两种意思。
在标准I/O库方面，flush意味着将缓冲区中的内容写到磁盘上。在终端驱动程序方面flush表示丢弃已存储在缓冲区中的数据。

### 2、行缓冲。在这种情况下，当在输入和输出中遇到换行符时，标准I/O库执行I/O操作。这允许我们一次输出一个字符，但只有在写了一行之后才进行实际I/O操作。当流涉及一个终端时，通常使用行缓冲。

#### 对于行缓冲有两个限制:

第一，因为标准I/O库用来收集每一行的缓冲区的长度是固定的，所以只要填满了缓冲区，那么即使没有写一个换行符，也进行I/O操作。

第二，任何时候只要通过标准I/O库要求从a一个布袋缓冲的流，或者b一个行缓冲的流得到输入数据，那么就会造成冲洗所有行缓冲输出流。在b中带了一个在括号中的说明，其理由是，所需的数据可能已在缓冲区中，他并不需求在需要数据时才从内核读数据。很明显，从不带缓冲的一个流中进行输入要求当时从内核得到数据。

### 3、不带缓冲。标准I/O库不对字符进行缓冲存储。例如，如果用I/O函数fputs写15个字符到不带缓冲的流中，则该函数很可能用write系统调用函数将这些字符立即写至相关联的打开文件中。
标准出错流stderr通常是不带缓冲的，这就使得出错信息可以尽快显示出来，而不管它们是否含有一个换行符。
## ISO C要求下列缓冲特征：
### 1.当且仅当标准输入和标准输出并不涉及交互式设备(比如终端显示器)，他们才是全缓冲的。
### 2.标准出错绝不会是全缓冲的。
但是，这并没有告诉我们如果标准输入和标准输出涉及交互式设备时，他们是不带缓冲的还是行缓冲的；以及标准出错是不带缓冲的还是行缓冲的。

### 很多系统默认使用下列类型的缓冲：

1.标准出错是不带缓缓冲的。

2.如若是涉及终端设备(在终端显示器上显示)的其他流，则他们是行缓冲的；否则是全缓冲的。

3.对任何一个给定的流，如果我们并不喜欢这些系统默认的情况，则可调用下列函数中的一个更改缓冲类型：
void setbuf(FILE *restrict fp, char *restrict buf)
int setvbuf(FILE *restrict fp, char *restrict buf,int mode,size_t size)

看个小例子
源程序：
```

int globa = 4;

int main (void )
{
pid_t pid;
int vari = 5;

printf ("before fork\n" );

if ((pid = fork()) < 0){
  printf ("fork error\n");
  exit (0);
}else if (pid == 0){
  globa++ ;
  vari--;
  printf("Child changed\n");
}else
  printf("Parent did not changde\n");

  printf("globa = %d vari = %d\n",globa,vari);
  exit(0);
}

执行结果：
输出到标准输出
[root@happy bin]# ./simplefork
before fork
Child changed
globa = 5 vari = 4
Parent did not changde
globa = 4 vari = 5

重定向到文件时before fork输出两边
[root@happy bin]# ./simplefork>temp
[root@happy bin]# cat temp
before fork
Child changed
globa = 5 vari = 4
before fork
Parent did not changde
globa = 4 vari = 5
```
## 总结：

1、输出到终端，分析直接运行程序时标准输出是行缓冲的，很快被新的一行冲掉。

2、输出到文件，重定向后，标准输出是全缓冲的。当调用fork时before fork这行仍保存在缓冲中，并随着数据段复制到子进程缓冲中。这样，这一行就分别进入父子进程的输出缓冲中，余下的输出就接在了这一行的后面。
