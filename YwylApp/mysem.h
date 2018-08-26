#if !defined MYSEMH
#include <stdio.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
//#include <linux/sem.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define MYSEMH  1
#define  SEM_KEY 9981

#if defined(__cplusplus)
extern "C"{
#endif


/**
 *   initial the sempore
 */
void init_mysem(int ini_flag);

void init_mysemvalue(int s_id,int ini_value);

int  getmysemvalue(int s_id);

void mywait(int s_id);

void mysignal(int s_id);

void myuwait(int s_id);

void myusignal(int s_id);

#if defined(__cplusplus)
}
#endif

#endif
