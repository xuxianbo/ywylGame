#if ! defined(TCP_H)  /* { */
#define       TCP_H        1
#if defined(__cplusplus)
extern "C"{
#endif

#define BUFSIZE 5120
#define MAXTIMEWAIT	3 
#define MAXTIMEWAIT_WRITE 2
#define TIMEOUT 0
#define CONTROLLEN sizeof (struct cmsghdr) + sizeof (int)
/*
typedef union overlay {
    char letter[4];
    int    number;
} overlay;

overlay FNumber;*/

void send_long64_2buf(char *buf,long long int n);
void recv_long64_from(char *buf,long long int *n);
void send_int32_2buf(char *buf, int n);
void send_string_2buf(char *buf, char*str);
void recv_int32_from(char *buf, int *n);
void send_int16_2buf(char *buf, short int n);
void recv_int16_from(char *buf, short int *n);

void XORStr(char *buff,int b_size);

int App_Send(int socket_fd,char *buff,int b_size);

int App_Recv(int socket,char *buff,int size);

int	init_sock(char *host,int port_id);

int checkSocket( int sock, int timeout );

int openSocket(char *host, int port,int forkid);

int checkWriteSocket( int sock, int timeout );

int read_fd(int fd);

int write_fd(int fd, int sendfd);

#if defined(__cplusplus)
}
#endif

#endif

