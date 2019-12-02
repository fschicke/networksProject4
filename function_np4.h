// Networks Project 4
// Thomas Clare & Francis Schickel
// 12/2/2019

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

extern int ballX,   ballY,   dx,   dy,   padLY,   padRY,   scoreL,   scoreR;
extern int ballX_c, ballY_c, dx_c, dy_c, padLY_c, padRY_c, scoreL_c, scoreR_c;


int recv_func(int s, struct sockaddr_in *sin, pthread_t pth);
void send_func(int s, struct sockaddr_in *sin, pthread_t pth);
void logic_check();
void kill_switch(int s, struct sockpthread_t pth);
