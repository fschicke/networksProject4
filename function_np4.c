// Networks Project 4
// Thomas Clare & Francis Schickel
// 12/2/2019

#include <function_np4.h>

int recv_func(int s, struct sockaddr_in * sin, pthread_t * pth){
    socklen_t addr_len = sizeof(struct sockaddr);
    char buf[BUFSIZ];
    char temp[BUFSIZ];
    struct sockaddr_in client_addr;
    if(recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)sin, &addr_len) == -1){
        exit(1);
    }
    if(strcmp(buf, "kill")==1){
        return 1;
    }
    int i;
    int j = 0;
    int arr[8];
    for(i=0; buf[i]!='\n'; i++){
        if(buf[i] == '\t'){ 
            arr[j] = atoi(temp);
            j++;
            bzero((char *)&temp, sizeof(temp));
            continue;
        }
        strcat(temp, buf[i]);
    }
    ballX_c = arr[0];
    ballY_c = arr[1];
    dx_c    = arr[2];
    dy_c    = arr[3];
    padLY_c = arr[4];
    padRY_c = arr[5];
    scoreL_c= arr[6];
    scoreR_c= arr[7];
    return 0;
}

void send_func(int s, struct sockaddr_in * sin, pthread_t pth){
    socklen_t addr_len = sizeof(struct sockaddr);
    char buf[BUFSIZ];
    char temp[BUFSIZ];
    strcat(buf, itoa(ballX, temp, 10));
    strcat(buf, "\t");
    bzero((char *)&temp, sizeof(temp));
    strcat(buf, itoa(ballY, temp, 10));
    strcat(buf, "\t");
    bzero((char *)&temp, sizeof(temp));
    strcat(buf, itoa(dx, temp, 10));
    strcat(buf, "\t");
    bzero((char *)&temp, sizeof(temp));
    strcat(buf, itoa(dy, temp, 10));
    strcat(buf, "\t");
    bzero((char *)&temp, sizeof(temp));
    strcat(buf, itoa(padLY, temp, 10));
    strcat(buf, "\t");
    bzero((char *)&temp, sizeof(temp));
    strcat(buf, itoa(padRY, temp, 10));
    strcat(buf, "\t");
    bzero((char *)&temp, sizeof(temp));
    strcat(buf, itoa(scoreL, temp, 10));
    strcat(buf, "\t");
    bzero((char *)&temp, sizeof(temp));
    strcat(buf, itoa(scoreR, temp, 10));
    strcat(buf, "\n");
    bzero((char *)&temp, sizeof(temp));
    struct sockaddr_in client_addr;
    if(sendto(s, buf, strlen(buf)+1, 0, (struct sockaddr*)sin, addr_len) == -1){
        exit(1);
    }
}

void logic_check(){
}

void *kill_switch(int s, struct sockaddr_in *sin, pthread_t pth);
