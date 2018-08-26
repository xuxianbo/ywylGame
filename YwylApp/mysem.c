#include "mysem.h"
#include "myshm.h"
#define Exit(err,s) exit(err);perror(s);
extern HALL *hall;
static int	mysem_id = -1;

union semun{
        int     val;
        struct semid_ds *buf;
        unsigned short *array;
};

void init_mysem(int ini_flag)
{
//	key_t	key;
	int i,size;
	struct sembuf	op;
	union semun		arg;
//	op.sem_num	=	s_id;
	op.sem_op	=	1;
	op.sem_flg	=	0;
//	key=ftok(PATHNAME,'O');
	size = 1;
    	printf("SEMKEY:%d,SIZE:%d\n",SEM_KEY,size);
	if((mysem_id=semget(SEM_KEY,size,IPC_CREAT|0666))==-1){
			perror("semget err!\n");
            Exit(-1, "[Error]create sem error");
	}
	if(ini_flag == 1){
		arg.array = (unsigned short *) calloc(size,sizeof(unsigned short));
		if(arg.array == NULL){
			printf("No enough memory to calloc!\n");
            Exit(-1, "[Error]create sem error.No enough memory fault.");
		}
		for(i=0;i<size;i++){
			arg.array[i]	=	1;
		}
		if(semctl(mysem_id,0,SETALL,arg) == -1){
			perror("semctl err:");
            Exit(-1, "[Error]read sem error");
		}
	}
}

void init_mysemvalue(int s_id,int ini_value)
{
//	key_t	key;
	int size;
	struct sembuf	op;
	union semun		arg;
//	op.sem_num	=	s_id;
	op.sem_op	=	1;
	op.sem_flg	=	0;
	size = 1;
//	key=ftok(PATHNAME,'O');
	if(mysem_id == -1){
		if((mysem_id=semget(SEM_KEY,size,IPC_CREAT|0666))==-1){
				perror("semget err!\n");
                Exit(-1, "[Error]create sem error");
		}
	}
	/*arg.array = (unsigned short *) calloc(SEMSIZE,sizeof(unsigned short));
	if(arg.array == NULL){
		printf("No enough memory to calloc!\n");
		exit(-1);
	}
	arg.array[s_id]	=	ini_value;*/
	arg.val = ini_value;
	if(semctl(mysem_id,s_id,SETVAL,arg) == -1){
		perror("semctl err:");
            Exit(-1, "[Error]semctl error");
	}
}

int  getmysemvalue(int s_id)
{
	int i_value,size;
	size = 1;
	if(mysem_id == -1){
		if((mysem_id=semget(SEM_KEY,size,IPC_CREAT|0666))==-1){
				perror("semget err!\n");
            Exit(-1, "[Error]create sem error");
		}
	}
	i_value = semctl(mysem_id,s_id,GETVAL);
	if(i_value == -1){
		perror("semctl err:");
Exit(-1, "[Error]semctl error");
	}
	return i_value;
}

void mywait(int s_id)
{
	struct sembuf	op;
	int    i_result;
	op.sem_num	=	s_id;
	op.sem_op	=	-1;
	op.sem_flg	=	SEM_UNDO;
	if(mysem_id == -1)
		init_mysem(0);
	while((i_result = semop(mysem_id,&op,1)) == -1 && errno == EINTR) usleep(5000);
	if(i_result==-1){
                        //printf("mywait sid = %d errno = %d   %s\n",s_id,errno,strerror(errno));
			if(errno == ERANGE){
                                printf("semval = %d\n",getmysemvalue(s_id));
                        }
			perror("semop err!\n");
	      Exit(-1, "[Error]semop error");
	}
}

void mysignal(int s_id)
{
	struct sembuf	op;
	int i_result;
	op.sem_num	=	s_id;
	op.sem_op	=	1;
	op.sem_flg	=	SEM_UNDO;
	if(mysem_id == -1)
		init_mysem(0);
	while((i_result = semop(mysem_id,&op,1)) == -1 && errno == EINTR) usleep(5000);
	if(i_result==-1){
                        //printf("mysignal s_id = %d  errno = %d   %s\n",s_id,errno,strerror(errno));
			if(errno == ERANGE){
				printf("semval = %d\n",getmysemvalue(s_id));
			}
			perror("semop err!\n");
	      Exit(-1, "[Error]semop error");
	}
}

void myuwait(int s_id)
{
	struct sembuf	op;
	int i_result;
	op.sem_num	=	s_id;
	op.sem_op	=	-1;
	op.sem_flg	=	0;
	if(mysem_id == -1)
		init_mysem(0);
	while((i_result = semop(mysem_id,&op,1)) == -1 && errno == EINTR) usleep(5000);
	if(i_result==-1){
		        //printf("myuwait s_id = %d  errno = %d   %s\n",s_id,errno,strerror(errno));
			if(errno == ERANGE){
                                printf("semval = %d\n",getmysemvalue(s_id));
                        }
			perror("semop err!\n");
	      Exit(-1, "[Error]semop error");
	}
}

void myusignal(int s_id)
{
	struct sembuf	op;
	int i_result;
	op.sem_num	=	s_id;
	op.sem_op	=	1;
	op.sem_flg	=	0;
	if(mysem_id == -1)
		init_mysem(0);
	while((i_result = semop(mysem_id,&op,1)) == -1 && errno == EINTR) usleep(5000);
	if(i_result==-1){
		        //printf("myusignal s_id = %d  errno = %d   %s\n",s_id,errno,strerror(errno));
			if(errno == ERANGE){
                                printf("semval = %d\n",getmysemvalue(s_id));
                        }
			perror("semop err!\n");
	      Exit(-1, "[Error]semop error");
	}
}

