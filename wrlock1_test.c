#include <stdlib.h>
#include <pthread.h>


struct queue *qp;
struct job {
	struct job *j_next;
	struct job *j_prev;
	pthread_t   j_id;   /* tells which thread handles this job */
	/* ... more stuff here ... */
	char * j_name;
};

struct queue {
	struct job      *q_head;
	struct job      *q_tail;
	pthread_rwlock_t q_lock;
};

/*
 * Initialize a queue.
 */
int
queue_init(struct queue *qp)
{
	int err;

	qp->q_head = NULL;
	qp->q_tail = NULL;
	err = pthread_rwlock_init(&qp->q_lock, NULL);
	if (err != 0)
		return(err);
	/* ... continue initialization ... */
	printf("queque init... \n");
	
	return(0);
}

/*
 * Insert a job at the head of the queue.
 */
void
job_insert(struct queue *qp, struct job *jp)
{
	pthread_rwlock_wrlock(&qp->q_lock);
	jp->j_next = qp->q_head;
	jp->j_prev = NULL;
	if (qp->q_head != NULL)
		qp->q_head->j_prev = jp;
	else
		qp->q_tail = jp;	/* list was empty */
	qp->q_head = jp;
	pthread_rwlock_unlock(&qp->q_lock);
}

/*
 * Append a job on the tail of the queue.
 */
void
job_append(struct queue *qp, struct job *jp)
{
	pthread_rwlock_wrlock(&qp->q_lock);
	jp->j_next = NULL;
	jp->j_prev = qp->q_tail;
	if (qp->q_tail != NULL)
		qp->q_tail->j_next = jp;
	else
		qp->q_head = jp;	/* list was empty */
	qp->q_tail = jp;
	pthread_rwlock_unlock(&qp->q_lock);
}

/*
 * Remove the given job from a queue.
 */
void
job_remove(struct queue *qp, struct job *jp)
{
	pthread_rwlock_wrlock(&qp->q_lock);
	if (jp == qp->q_head) {
		qp->q_head = jp->j_next;
		if (qp->q_tail == jp)
			qp->q_tail = NULL;
		else
			jp->j_next->j_prev = jp->j_prev;
	} else if (jp == qp->q_tail) {
		qp->q_tail = jp->j_prev;
		jp->j_prev->j_next = jp->j_next;
	} else {
		jp->j_prev->j_next = jp->j_next;
		jp->j_next->j_prev = jp->j_prev;
	}
	pthread_rwlock_unlock(&qp->q_lock);
}

/*
 * Find a job for the given thread ID.
 */
struct job *
job_find(struct queue *qp, pthread_t id)
{
	struct job *jp;

	if (pthread_rwlock_rdlock(&qp->q_lock) != 0)
		return(NULL);

	for (jp = qp->q_head; jp != NULL; jp = jp->j_next)
		if (pthread_equal(jp->j_id, id))
			break;

	pthread_rwlock_unlock(&qp->q_lock);
	return(jp);
}






void * thr_fn0(void *arg)
{
	struct job *jp;
	printf("thread 0:\n");
	if((jp=malloc(sizeof(struct job))) != NULL)
	{
		jp->j_id = pthread_self();
		jp->j_name = "thr_fn0";		

	}
	job_insert(qp,jp);
	
	pthread_exit((void *)0);
}
void * thr_fn1(void *arg)
{
	struct job *jp;
	printf("thread 1:\n");
	if((jp=malloc(sizeof(struct job))) != NULL)
	{
		jp->j_id = pthread_self();
		jp->j_name = "thr_fn1";		

	}
	job_insert(qp,jp);
	
	pthread_exit((void *)1);

}

void * thr_fn2(void *arg)
{
	struct job *jp;
	printf("thread 2:\n");
	if((jp=malloc(sizeof(struct job))) != NULL)
	{
		jp->j_id = pthread_self();
		jp->j_name = "thr_fn2";		

	}
	job_insert(qp,jp);
	
	pthread_exit((void *)2);
}


void * thr_fn3(void *arg)
{
	struct job *jp;
	printf("thread 3:\n");
	if((jp=malloc(sizeof(struct job))) != NULL)
	{
		jp->j_id = pthread_self();
		jp->j_name = "thr_fn3";		

	}

	sleep(1);
	job_insert(qp,jp);
	
	pthread_exit((void *)3);

}

void * thr_fn4(void *arg)
{
	struct job *jp;
	printf("thread 4:\n");
        jp = job_find(qp,arg);
	if(jp != NULL)
		printf("thread 4 find pthread id %d; name %s\n",jp->j_id,jp->j_name);		else
		printf("thread4  can't find pthread\n");
	
	pthread_exit((void *)4);

}


void * thr_fn5(void *arg)
{
	struct job *jp;
	printf("thread 5:\n");
	
        jp = job_find(qp,arg);
	job_remove(qp,jp);
	printf("thread 5 remove pthread id %d; name %s\n",jp->j_id,jp->j_name);	
	pthread_exit((void *)5);

}



void * thr_fn6(void *arg)
{
	struct job *jp;
	printf("thread 6:\n");
	
        jp = job_find(qp,arg);
	if(jp == NULL)
		printf("thread 6 can't find pthread\n");	
	else
		printf("thread 6 find pthread id %d; name %s\n",jp->j_id,jp->j_name);	
	pthread_exit((void *)6);

}

















int
main(void)
{
	int			err;
	pthread_t	tid,tid0,tid1,tid2,tid3,tid4,tid5,tid6;
	pid_t pid;
        int i;
	if((qp = malloc(sizeof(struct queue))) != NULL){
		queue_init(qp);
	}


	err = pthread_create(&tid0, NULL, thr_fn0, NULL);
	if (err != 0)
		err_exit(err, "can't create thread 0");


	err = pthread_create(&tid4, NULL, thr_fn4, (void *)tid0);
	if (err != 0)
		err_exit(err, "can't create thread 4");

	err = pthread_create(&tid5, NULL, thr_fn5, (void *)tid0);
	if (err != 0)
		err_exit(err, "can't create thread 5");


	err = pthread_create(&tid6, NULL, thr_fn6, (void *)tid0);
	if (err != 0)
		err_exit(err, "can't create thread 6");



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

	err = pthread_join(tid4, NULL);
	if (err != 0)
		err_exit(err, "can't join with thread 4");
	err = pthread_join(tid5, NULL);
	if (err != 0)
		err_exit(err, "can't join with thread 5");
	err = pthread_join(tid6, NULL);
	if (err != 0)
		err_exit(err, "can't join with thread 6");
	while(qp->q_head)
	{
		printf("parent thread name: %s\n",qp->q_head->j_name);	
		qp->q_head = qp->q_head->j_next;

}

        printf("parent:\n");
	exit(0);
}
