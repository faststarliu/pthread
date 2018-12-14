#include <stdlib.h>

#include <pthread.h>

#define NHASH 29
#define HASH(id) ((unsigned long)id)

struct foo *fh[NHASH];

pthread_mutex_t hashlock = PTHREAD_MUTEX_INITIALIZER;

struct foo {
	int             f_count;
	pthread_mutex_t f_lock;
	int             f_id;
	struct foo     *f_next; /* protected by hashlock */
	/* ... more stuff here ... */
};

struct foo *
foo_alloc(int id) /* allocate the object */
{
	struct foo	*fp;
	int			idx;

	if ((fp = malloc(sizeof(struct foo))) != NULL) {
		fp->f_count = 1;
		fp->f_id = id;
		if (pthread_mutex_init(&fp->f_lock, NULL) != 0) {
			free(fp);
			return(NULL);
		}
		idx = HASH(id);
		pthread_mutex_lock(&hashlock);
		fp->f_next = fh[idx];
		fh[idx] = fp;
		pthread_mutex_lock(&fp->f_lock);
		pthread_mutex_unlock(&hashlock);
		/* ... continue initialization ... */
		pthread_mutex_unlock(&fp->f_lock);
	}
	return(fp);
}

void
foo_hold(struct foo *fp) /* add a reference to the object */
{
	pthread_mutex_lock(&fp->f_lock);
	fp->f_count++;
	pthread_mutex_unlock(&fp->f_lock);
}

struct foo *
foo_find(int id) /* find an existing object */
{
	struct foo	*fp;

	pthread_mutex_lock(&hashlock);
	for (fp = fh[HASH(id)]; fp != NULL; fp = fp->f_next) {
		if (fp->f_id == id) {
			foo_hold(fp);
			break;
		}
	}
	pthread_mutex_unlock(&hashlock);
	return(fp);
}

void
foo_rele(struct foo *fp) /* release a reference to the object */
{
	struct foo	*tfp;
	int			idx;

	pthread_mutex_lock(&fp->f_lock);
	if (fp->f_count == 1) { /* last reference */
		pthread_mutex_unlock(&fp->f_lock);
		pthread_mutex_lock(&hashlock);
		pthread_mutex_lock(&fp->f_lock);
		/* need to recheck the condition */
		if (fp->f_count != 1) {
			fp->f_count--;
			pthread_mutex_unlock(&fp->f_lock);
			pthread_mutex_unlock(&hashlock);
			return;
		}
		/* remove from list */
		idx = HASH(fp->f_id);
		tfp = fh[idx];
		if (tfp == fp) {
			fh[idx] = fp->f_next;
		} else {
			while (tfp->f_next != fp)
				tfp = tfp->f_next;
			tfp->f_next = fp->f_next;
		}
		pthread_mutex_unlock(&hashlock);
		pthread_mutex_unlock(&fp->f_lock);
		pthread_mutex_destroy(&fp->f_lock);
		free(fp);
	} else {
		fp->f_count--;
		pthread_mutex_unlock(&fp->f_lock);
	}
}
void * thr_fn0(void *arg)
{
	struct foo * fp;
	printf("thread 0:\n");
	foo_find(0);
/*
	fp = fh[HASH(0)];
	printf("thr_fn0 f_id: %d ",fp->f_id);
	printf("thr_fn0 f_count: %d\n",fp->f_count);
*/

	
	pthread_exit((void *)0);
}
void * thr_fn1(void *arg)
{
	struct foo * fp;
	printf("thread 1:\n");
	foo_find(0);	
/*
	fp = fh[HASH(1)];
	printf("thr_fn1 f_id: %d ",fp->f_id);
	printf("thr_fn1 f_count: %d\n",fp->f_count);
*/
	pthread_exit((void *)1);

}

void * thr_fn2(void *arg)
{
	struct foo * fp;
	printf("thread 2:\n");
	foo_find(0);	
/*
	fp = fh[HASH(2)];
	printf("thr_fn2 f_id: %d ",fp->f_id);
	printf("thr_fn2 f_count: %d\n",fp->f_count);
*/
	pthread_exit((void *)2);
}


void * thr_fn3(void *arg)
{

	struct foo * fp;
	printf("thread 3:\n");
	foo_find(0);	
/*
	fp = fh[HASH(3)];
	printf("thr_fn3 f_id: %d ",fp->f_id);
	printf("thr_fn3 f_count: %d\n",fp->f_count);
*/
	pthread_exit((void *)3);

}


int
main(void)
{
	int			err;
	pthread_t	tid,tid0,tid1,tid2,tid3;
	pid_t pid;
        int i;
	struct foo * fp;
	for(i=0;i<5;i++)
	{
		fp = foo_alloc(i);
		printf("creat new foo ;");
		printf("f_id: %d ",fp->f_id);
		printf("f_count: %d\n",fp->f_count);

	}




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


	for(i=0;i<5;i++)
	{
		fp = fh[HASH(i)];

		printf("parent f_id: %d f_count: %d\n",fp->f_id,fp->f_count);
		foo_rele(fp);
	}

        printf("parent:\n");
	exit(0);
}
