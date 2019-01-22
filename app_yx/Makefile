CFLAGS=-Wall -g -I/usr/local/mysql/include
LIB =-lcrypto -lhiredis -ldl -lm -lmysqlclient -lpthread -L/usr/local/mysql/lib
CC = gcc -O ${CFLAGS}

all:yx

obj = zjh.o bjl.o mytcp.o mybase.o mymysql.o myshm.o myhiredis.o cards.o tcppack.o ox.o wzq.o mj.o hb.o

cleanobj:
	@rm -f *.o
cleanall:
	@rm -f *.o core *.bak yx

yx: app.c appproc.o  $(obj)
	$(CC) -o $@ $^ $(LIB) 

appproc: appproc.c $(obj)
	$(CC) -o $@ $^ $(LIB)

mytcp:mytcp.c
	$(CC) -o $@ $^ $(LIB)

mybase:mybase.c
	$(CC) -o $@ $^ $(LIB)

mymysql:mymysql.c
	$(CC) -o $@ $^ $(LIB)

myshm:myshm.c
	$(CC) -o $@ $^ $(LIB)

myhiredis:myhiredis.c
	$(CC) -o $@ $^ $(LIB)

cards:cards.c
	$(CC) -o $@ $^ $(LIB)

zjh:zjh.c
	$(CC) -o $@ $^ $(LIB)

bjl:bjl.c
	$(CC) -o $@ $^ $(LIB)

ox:ox.c
	$(CC) -o $@ $^ $(LIB)

tcppack:tcppack.c
	$(CC) -o $@ $^ $(LIB)

wzq:wzq.c
	$(CC) -o $@ $^ $(LIB)

mj:mj.c
	$(CC) -o $@ $^ $(LIB)

hb:hb.c
	$(CC) -o $@ $^ $(LIB)