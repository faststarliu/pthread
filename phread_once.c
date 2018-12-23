#include <limits.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#define MAXSTRINGSZ	4096

static pthread_key_t key;
static pthread_once_t init_done = PTHREAD_ONCE_INIT;
pthread_mutex_t env_mutex = PTHREAD_MUTEX_INITIALIZER;

extern char **environ;

void func_free()
{
	char  * envbuf;
	printf("thread%d:func_free\n",pthread_self());
	envbuf = (char *)pthread_getspecific(key);
   if(envbuf != NULL)
   {
	   printf("%s\n",envbuf);
   		free(envbuf);
   }else
   {
	   printf("free\n");
   }

}




static void
thread_init(void)
{
	pthread_key_create(&key, func_free);
}

char *
getenv(const char *name)
{
	int		i, len;
	char	*envbuf;

	pthread_once(&init_done, thread_init);
	pthread_mutex_lock(&env_mutex);
	envbuf = (char *)pthread_getspecific(key);
	if (envbuf == NULL) {
		envbuf = malloc(MAXSTRINGSZ);
		if (envbuf == NULL) {
			pthread_mutex_unlock(&env_mutex);
			return(NULL);
		}
		pthread_setspecific(key, envbuf);
	}
	len = strlen(name);
	for (i = 0; environ[i] != NULL; i++) {
		if ((strncmp(name, environ[i], len) == 0) &&
		  (environ[i][len] == '=')) {
			printf("getenv:%s\n",environ[i]);
			strncpy(envbuf, &environ[i][len+1], MAXSTRINGSZ-1);
			pthread_mutex_unlock(&env_mutex);
			return(envbuf);
		}
	}
	pthread_mutex_unlock(&env_mutex);
	return(NULL);
}
void thread(void * arg)
{
	int		i, len;
	char	*envbuf;
	printf("thread%d run...\n",(int) arg);
	getenv("HOME");	
	/*
	for (i = 0; environ[i] != NULL; i++)
	{
		printf("thread%d: %s\n",(void *)arg,environ[i]);
	}
*/


/*
	pthread_once(&init_done, thread_init);
	pthread_mutex_lock(&env_mutex);
	envbuf = (char *)pthread_getspecific(key);
	if (envbuf == NULL) {
		envbuf = malloc(MAXSTRINGSZ);
		if (envbuf == NULL) {
			pthread_mutex_unlock(&env_mutex);
  			pthread_exit((void *)arg);
		}
		pthread_setspecific(key, envbuf);
	}
	envbuf ="thread_data";
	printf("thread%d: %s\n",(void *)arg,envbuf);
	pthread_mutex_unlock(&env_mutex);
  
*/

	pthread_exit((void *)arg);
}


int main()
{
	
	int err,err1,err2;
	pthread_t tid1,tid2;
	pthread_attr_t attr;
	/*
	err = pthread_attr_init(&attr);
	if(err != 0)
		return err;
	err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if(err == 0)
	{
		err1 = pthread_create(&tid1,NULL,thread,(void *) 1);
		err2 = pthread_create(&tid2,NULL,thread,(void *) 2);
	}
	*/
	err1 = pthread_create(&tid1,NULL,thread,(void *) 1);
	err2 = pthread_create(&tid2,NULL,thread,(void *) 2);
	if(err1 != 0)
		err_exit(err1,"can't create thread1");

	if(err2 != 0)
		err_exit(err2,"can't create thread2");
	err1 = pthread_join(tid1,NULL);
	if(err1 != 0)
		err_exit(err1,"can't join with thread1");

	err2 = pthread_join(tid2,NULL);
	if(err2 != 0)
		err_exit(err2,"can't join with thread2");

    printf("parent:\n");
	
	char  * envbuf;
	envbuf = (char *)pthread_getspecific(key);
   	if(envbuf != NULL)
   	{
	   printf("parent:%s\n",envbuf);
   		free(envbuf);
   	}

	exit(0);

}
