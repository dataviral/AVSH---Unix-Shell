#include<string.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<pthread.h>
#include<signal.h>
#include <setjmp.h>


#define MAX_CLIENTS 5

char cwd[100];
// char *history[100];
static int outfd = 1;
static int errfd = 2;
static int sock;
static int numSigint;

jmp_buf ctrlc_jp;

static char *historyFile = ".history.avsh";
static char *aliasFile = ".alias.avsh";

struct history{
        int n;
        char *h[1000];
}h;


struct alias{
	int n;
	char *a[100];
}a;


int run_command(char **, char **);
void load_history();
void rewrite_history();
void show_history(int);
char **get_tokens(char *);
void load_alias();
void replace_alias(char ***);
void start_server(char *ip, int port);
void *new_connection_handler(void *);
void connect_avsh(char *ip, int port);
void sighandler(int);
void sighandler_main(int);
int parseInputServer(char *, int );
void get_fileServer(char *, int );
void connect_avsh(char *, int port);
void get_fileClient(char *, int );
int parseInputClient(char *, int );
char **get_tokens(char *);
int is_special(char);
