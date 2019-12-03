#include <ncurses.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#define WIDTH 43
#define HEIGHT 21
#define PADLX 1
#define PADRX WIDTH - 2
#define MAX_LINE 256

// Global variables recording the state of the game
// Position of ball
int ballX, ballY;
// Movement of ball
int dx, dy;
// Position of paddles
int padLY, padRY;
// Player scores
int scoreL, scoreR;

// ncurses window
WINDOW *win;

/* Draw the current game state to the screen
 * ballX: X position of the ball
 * ballY: Y position of the ball
 * padLY: Y position of the left paddle
 * padRY: Y position of the right paddle
 * scoreL: Score of the left player
 * scoreR: Score of the right player
 */

void errorAndExit(char * s) { 
	fprintf(stderr,s);
	exit(1);
}

void draw(int ballX, int ballY, int padLY, int padRY, int scoreL, int scoreR) {
    // Center line
    int y;
    for(y = 1; y < HEIGHT-1; y++) {
        mvwaddch(win, y, WIDTH / 2, ACS_VLINE);
    }
    // Score
    mvwprintw(win, 1, WIDTH / 2 - 3, "%2d", scoreL);
    mvwprintw(win, 1, WIDTH / 2 + 2, "%d", scoreR);
    // Ball
    mvwaddch(win, ballY, ballX, ACS_BLOCK);
    // Left paddle
    for(y = 1; y < HEIGHT - 1; y++) {
        int ch = (y >= padLY - 2 && y <= padLY + 2)? ACS_BLOCK : ' ';
        mvwaddch(win, y, PADLX, ch);
    }
    // Right paddle
    for(y = 1; y < HEIGHT - 1; y++) {
        int ch = (y >= padRY - 2 && y <= padRY + 2)? ACS_BLOCK : ' ';
        mvwaddch(win, y, PADRX, ch);
    }
    // Print the virtual window (win) to the screen
    wrefresh(win);
    // Finally erase ball for next time (allows ball to move before next refresh)
    mvwaddch(win, ballY, ballX, ' ');
}

/* Return ball and paddles to starting positions
 * Horizontal direction of the ball is randomized
 */
void reset() {
    ballX = WIDTH / 2;
    padLY = padRY = ballY = HEIGHT / 2;
    // dx is randomly either -1 or 1
    dx = (rand() % 2) * 2 - 1;
    dy = 0;
    // Draw to reset everything visually
    draw(ballX, ballY, padLY, padRY, scoreL, scoreR);
}

/* Display a message with a 3 second countdown
 * This method blocks for the duration of the countdown
 * message: The text to display during the countdown
 */
void countdown(const char *message) {
    int h = 4;
    int w = strlen(message) + 4;
    WINDOW *popup = newwin(h, w, (LINES - h) / 2, (COLS - w) / 2);
    box(popup, 0, 0);
    mvwprintw(popup, 1, 2, message);
    int countdown;
    for(countdown = 3; countdown > 0; countdown--) {
        mvwprintw(popup, 2, w / 2, "%d", countdown);
        wrefresh(popup);
        sleep(1);
    }
    wclear(popup);
    wrefresh(popup);
    delwin(popup);
    padLY = padRY = HEIGHT / 2; // Wipe out any input that accumulated during the delay
}

/* Perform periodic game functions:
 * 1. Move the ball
 * 2. Detect collisions
 * 3. Detect scored points and react accordingly
 * 4. Draw updated game state to the screen
 */
void tock() {
    // Move the ball
    ballX += dx;
    ballY += dy;
    
    // Check for paddle collisions
    // padY is y value of closest paddle to ball
    int padY = (ballX < WIDTH / 2) ? padLY : padRY;
    // colX is x value of ball for a paddle collision
    int colX = (ballX < WIDTH / 2) ? PADLX + 1 : PADRX - 1;
    if(ballX == colX && abs(ballY - padY) <= 2) {
        // Collision detected!
        dx *= -1;
        // Determine bounce angle
        if(ballY < padY) dy = -1;
        else if(ballY > padY) dy = 1;
        else dy = 0;
    }

    // Check for top/bottom boundary collisions
    if(ballY == 1) dy = 1;
    else if(ballY == HEIGHT - 2) dy = -1;
    
    // Score points
    if(ballX == 0) {
        scoreR = (scoreR + 1) % 100;
        reset();
        countdown("SCORE -->");
    } else if(ballX == WIDTH - 1) {
        scoreL = (scoreL + 1) % 100;
        reset();
        countdown("<-- SCORE");
    }
    // Finally, redraw the current state
    draw(ballX, ballY, padLY, padRY, scoreL, scoreR);
}

/* Listen to keyboard input
 * Updates global pad positions
 */
void *listenInput(void *args) {
    while(1) {
        switch(getch()) {
            case KEY_UP: padRY--;
             break;
            case KEY_DOWN: padRY++;
             break;
            case 'w': padLY--;
             break;
            case 's': padLY++;
             break;
            default: break;
       }
    }       
    return NULL;
}

void initNcurses() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    refresh();
    win = newwin(HEIGHT, WIDTH, (LINES - HEIGHT) / 2, (COLS - WIDTH) / 2);
    box(win, 0, 0);
    mvwaddch(win, 0, WIDTH / 2, ACS_TTEE);
    mvwaddch(win, HEIGHT-1, WIDTH / 2, ACS_BTEE);
}

int networkClientSetup(char * hostname, int portno) { 
    int s, refresh;
    unsigned int addr_len;
    struct sockaddr_in sin;
    struct hostent * hp;
    char buf[MAX_LINE];	

    hp = gethostbyname(hostname); // BUILD HOST STRUCT FROM NAME
    if (!hp) {
		fprintf(stderr,"netpong.c: unknown host: %s\n",hostname); 
		exit(1);
	}
	    
	bzero((char *)&sin, sizeof(sin)); // USE HOST STRUCT TO FILL IN SOCKADDR_IN
    sin.sin_family = AF_INET;  // IPv4 COMPATIBLE DEVICES
    bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
    sin.sin_port = htons(portno);

    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) errorAndExit("error: netpong.c: unable to build socket.\n"); 

	strcpy(buf, "SETUP");

	if(sendto(s, (char *)buf, strlen(buf) + 1, 0, (struct sockaddr *)&sin, sizeof(struct sockaddr))==-1) errorAndExit("error: netpong.c: unable to send SETUP string");

 	bzero((char *) buf, sizeof(buf));
	if( recvfrom(s, (char *)buf, sizeof(buf), 0,  (struct sockaddr *)&sin, &addr_len)==-1) errorAndExit("error: netpong.c: unable to receive level from client.\n");

	if(strcmp(buf, "easy") == 0) refresh = 80000;
    else if(strcmp(buf, "medium") == 0) refresh = 40000;
    else if(strcmp(buf, "hard") == 0) refresh = 20000;

	bzero((char *) buf, sizeof(buf));
	strcpy(buf, "ACK");

	if(sendto(s, (char *)buf, strlen(buf) + 1, 0, (struct sockaddr *)&sin, sizeof(struct sockaddr))==-1) errorAndExit("error: netpong.c: unable to send ACK\n");

	return refresh;	
}

int networkServerSetup(int portno) { 
    struct sockaddr_in sin, client_addr;
    int s;
    unsigned int addr_len = sizeof(client_addr);
	char buf[MAX_LINE];

   	bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(portno);

	if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) errorAndExit("error: netpong.c: unable to build socket.\n");
	
	if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) errorAndExit("error: netpong.c: unable to bind().\n");

    // refresh is clock rate in microseconds
    // This corresponds to the movement speed of the ball
    int refresh;
    char difficulty[MAX_LINE]; 

	do {
		printf("Please select the difficulty level (easy, medium or hard): ");
		fgets(difficulty,10,stdin);
		difficulty[strlen(difficulty)-1] = 0;	
	} while (strcmp(difficulty, "easy") && strcmp(difficulty, "medium") && strcmp(difficulty, "hard")); 

	printf("Waiting for challengers on port %d\n",portno);
		
	if( recvfrom(s, (char *)buf, sizeof(buf), 0,  (struct sockaddr *)&client_addr, &addr_len)==-1) errorAndExit("error: netpong.c: unable to receive start message from client.\n"); 
  
	if (strcmp(buf,"SETUP")){
		 fprintf(stderr,"error: netpong.c: expecting confirmation message SETUP but received %s",difficulty);
		 exit(1);
	}
		
	if(sendto(s, (char *)difficulty, strlen(difficulty) + 1, 0, (struct sockaddr *)&client_addr, sizeof(struct sockaddr))==-1) { 
		fprintf(stderr,"error: netpong.c: unable to send difficulty: %s\n",strerror(errno));
	}

	if(strcmp(difficulty, "easy") == 0) refresh = 80000;
    else if(strcmp(difficulty, "medium") == 0) refresh = 40000;
    else if(strcmp(difficulty, "hard") == 0) refresh = 20000;
	
	bzero((char *) difficulty, sizeof(difficulty));
	strcpy(difficulty, "ACK");

	if( recvfrom(s, (char *)difficulty, sizeof(difficulty), 0,  (struct sockaddr *)&client_addr, &addr_len)==-1) errorAndExit("error: netpong.c: unable to receive ACK from client.\n");
	
	return refresh;
}


int main(int argc, char *argv[]) {
    
	/* check command line args */

	if (argc != 3) errorAndExit("error: usage: ./netpong [--host|HOSTNAME] PORTNO\n");	
	int portNo = atoi(argv[2]);
	if (!portNo) errorAndExit("error: netpong.c: PORTNO must be a valid integer\n");
	
	/* determine whether this program was invoked as --host (server) or not */


	int refresh;
 	if (!strcmp(argv[1],"--host")) refresh = networkServerSetup(portNo); // this program acts as server
	else refresh = networkClientSetup(argv[1],portNo); // this program acts as a client 

    // Set up ncurses environment
    initNcurses();

    // Set starting game state and display a countdown
    reset();
    countdown("Starting Game");
    
    // Listen to keyboard input in a background thread
    pthread_t pth;
    pthread_create(&pth, NULL, listenInput, NULL);

    // Main game loop executes tock() method every REFRESH microseconds
    struct timeval tv;
    while(1) {
        gettimeofday(&tv,NULL);
        unsigned long before = 1000000 * tv.tv_sec + tv.tv_usec;
        
		/* TODO: send, recv, logic stuff (partially done by Francis) here */

		tock(); // Update game state
        gettimeofday(&tv,NULL);
        unsigned long after = 1000000 * tv.tv_sec + tv.tv_usec;
        unsigned long toSleep = refresh - (after - before);
        // toSleep can sometimes be > refresh, e.g. countdown() is called during tock()
        // In that case it's MUCH bigger because of overflow!
        if(toSleep > refresh) toSleep = refresh;
        usleep(toSleep); // Sleep exactly as much as is necessary
    }
    
    // Clean up
    pthread_join(pth, NULL);
    endwin();
    return 0;
}
