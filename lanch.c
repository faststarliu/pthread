#include <stdlib.h>

#include <pthread.h>

int eat=0;

pthread_mutex_t d1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t c1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t d2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t c2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t d3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t c3 = PTHREAD_MUTEX_INITIALIZER;
void * thr_fn0(void *arg)
{
	int err;
	printf("thread 0:\n");
	if(pthread_mutex_trylock(&d1) != 0)
		pthread_exit((void *)0);
	if(pthread_mutex_trylock(&c1) != 0)
		pthread_exit((void *)0);
	printf("thread 0 eat...\n");
/*
	pthread_mutex_unlock(&c1);
	pthread_mutex_unlock(&d1);
*/
	pthread_exit((void *)0);
}

void * thr_fn1(void *arg)
{
	int err;
	printf("thread 1:\n");
	if(pthread_mutex_trylock(&d2) != 0)
		pthread_exit((void *)0);
	if(pthread_mutex_trylock(&c1) != 0)
		pthread_exit((void *)0);
	printf("thread 1 eat...\n");
	pthread_exit((void *)0);
}

void * thr_fn2(void *arg)
{
	int err;
	printf("thread 2:\n");
	if(pthread_mutex_trylock(&d2) != 0)
		pthread_exit((void *)2);
	if(pthread_mutex_trylock(&c2) != 0)
		pthread_exit((void *)2);
	printf("thread 2 eat...\n");
	pthread_exit((void *)2);
}

void * thr_fn3(void *arg)
{
	int err;
	printf("thread 3:\n");
	if(pthread_mutex_trylock(&d3) != 0)
		pthread_exit((void *)3);
	if(pthread_mutex_trylock(&c2) != 0)
		pthread_exit((void *)3);
	printf("thread 3 eat...\n");
	pthread_exit((void *)3);
}
void * thr_fn4(void *arg)
{
	int err;
	printf("thread 4:\n");
	if(pthread_mutex_trylock(&d3) != 0)
		pthread_exit((void *)4);
	if(pthread_mutex_trylock(&c3) != 0)
		pthread_exit((void *)4);
	printf("thread 4 eat...\n");
	pthread_exit((void *)2);
}
void * thr_fn5(void *arg)
{
	int err;
	printf("thread 5:\n");
	if(pthread_mutex_trylock(&d1) != 0)
		pthread_exit((void *)5);
	if(pthread_mutex_trylock(&c3) != 0)
		pthread_exit((void *)5);
	printf("thread 5 eat...\n");
	pthread_exit((void *)5);
}






#if 0
void * thr_fn1(void *arg)
{
	printf("thread 1:\n");
	pthread_mutex_trylock(&d2);
	pthread_mutex_trylock(&c1);
	printf("thread 1 eat...toatal %d\n",++eat);
/*
	pthread_mutex_unlock(&c1);
	pthread_mutex_unlock(&d1);
*/
	pthread_exit((void *)1);

}

void * thr_fn2(void *arg)
{
	printf("thread 2:\n");
	pthread_mutex_trylock(&d2);
	pthread_mutex_trylock(&c2);
	printf("thread 2 eat...total %d\n",++eat);
/*
	pthread_mutex_unlock(&c1);
	pthread_mutex_unlock(&d1);
*/
	pthread_exit((void *)2);
}


void * thr_fn3(void *arg)
{

	printf("thread 3:\n");
	pthread_mutex_trylock(&d3);
	pthread_mutex_trylock(&c2);
	printf("thread 3 eat...total %d\n",++eat);
/*
	pthread_mutex_unlock(&c1);
	pthread_mutex_unlock(&d1);
*/
	pthread_exit((void *)3);

}


void * thr_fn4(void *arg)
{

	printf("thread 4:\n");
	pthread_mutex_trylock(&d3);
	pthread_mutex_trylock(&c3);
	printf("thread 4 eat...total %d\n",++eat);
/*
	pthread_mutex_unlock(&c1);
	pthread_mutex_unlock(&d1);
*/
	pthread_exit((void *)4);

}

void * thr_fn5(void *arg)
{

	printf("thread 5:\n");
	pthread_mutex_trylock(&d1);
	pthread_mutex_trylock(&c3);
	printf("thread 5 eat...total %d\n",++eat);
/*
	pthread_mutex_unlock(&c1);
	pthread_mutex_unlock(&d1);
*/
	pthread_exit((void *)5);

}
#endif




int
main(void)
{
	int			err;
	pthread_t	tid,tid0,tid1,tid2,tid3,tid4,tid5;




	err = pthread_create(&tid0, NULL, thr_fn0, NULL);
	if (err != 0)
		err_exit(err, "can't create thread 0");


	err = pthread_create(&tid1, NULL, thr_fn1, NULL);
	if (err != 0)
		err_exit(err, "can't create thread 1");
	err = pthread_create(&tid2, NULL, thr_fn2, NULL);
	if (err != 0)
		err_exit(err, "can't create thread 2");
	err = pthread_create(&tid3, NULL, thr_fn3, NULL);
	if (err != 0)
		err_exit(err, "can't create thread 3");
	err = pthread_create(&tid4, NULL, thr_fn4, NULL);
	if (err != 0)
		err_exit(err, "can't create thread 4");


	err = pthread_create(&tid5, NULL, thr_fn5, NULL);
	if (err != 0)
		err_exit(err, "can't create thread 5");





	err = pthread_join(tid0, NULL);
	if (err != 0)
		err_exit(err, "can't join with thread 0");

	err = pthread_join(tid1, NULL);
	if (err != 0)
		err_exit(err, "can't join with thread 1");

	err = pthread_join(tid2, NULL);
	if (err != 0)
		err_exit(err, "can't join with thread 2");
	err = pthread_join(tid3, NULL);
	if (err != 0)
		err_exit(err, "can't join with thread 3");

	err = pthread_join(tid4, NULL);
	if (err != 0)
		err_exit(err, "can't join with thread 4");

	err = pthread_join(tid5, NULL);
	if (err != 0)
		err_exit(err, "can't join with thread 5");

	pthread_mutex_unlock(&d1);
	pthread_mutex_unlock(&c1);
	pthread_mutex_unlock(&d2);
	pthread_mutex_unlock(&c2);
	pthread_mutex_unlock(&d3);
	pthread_mutex_unlock(&c3);
        printf("parent:\n");
	exit(0);
}
