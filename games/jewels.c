#include <curses.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#define LEN 17
#define WID 19
#define DELAY 2
#define SAVE_TO_NUM 10
#define DEF_ADDRESS "JewelScores"

/*
Jewels


copyright Hossein Bakhtiarifar 2018 (c)
No rights are reserved and this software comes with no warranties of any kind to the extent permitted by law.

compile with -lnucrses

A pair of jewels appear on top of the window, And you can move and rotate them while they are falling down.
If you make a vertical or horizontal row of 4 jewels they will explode and add up to your score.
Like Tetris,You will lose the game when the center of the uppermost row is filled.
*/

typedef signed char byte;
chtype board[LEN][WID];
byte jx,jy; //first jewel's position
byte kx,ky;//second jewel's position in relation to that of j
long score=0;
char* controls = "j,l-Move k-Rotate p-Pause q-Quit";
FILE* scorefile;
byte scorewrite(long score){// only saves the top 10
	bool deforno;
        if( !getenv("JW_SCORES") && (scorefile= fopen(DEF_ADDRESS,"r")) ){
		deforno=1;
	}
	else{
		deforno=0;
		if( !(scorefile = fopen(getenv("JW_SCORES"),"r")) ){
			printf("\nNo accessible score files found.\n");
			exit(EXIT_SUCCESS);
		}
	}

        char namebuff[SAVE_TO_NUM][60];
        long scorebuff[SAVE_TO_NUM];

        memset(namebuff,0,SAVE_TO_NUM*60*sizeof(char) );
        memset(scorebuff,0,SAVE_TO_NUM*sizeof(long) );

        long fuckingscore =0;
        char fuckingname[60]={0};
        byte location=0;

        while( fscanf(scorefile,"%59s : %ld\n",fuckingname,&fuckingscore) == 2 && location<SAVE_TO_NUM ){
                strcpy(namebuff[location],fuckingname);
                scorebuff[location] = fuckingscore;
                location++;

                memset(fuckingname,0,60);
                fuckingscore=0;
        }
	if(deforno)
        	scorefile = fopen(DEF_ADDRESS,"w+");//get rid of the text
	else
		scorefile = fopen(getenv("JW_SCORES"), "w+") ;
	if(!scorefile){
		printf("\nThe file cannot be opened in w+.\n");
		exit(EXIT_SUCCESS);
	}

        byte itreached=location;
        byte ret = -1;
        bool wroteit=0;

        for(location=0;location<=itreached && location<SAVE_TO_NUM-wroteit;location++){
                if(!wroteit && (location>=itreached || score>=scorebuff[location]) ){
                        fprintf(scorefile,"%s : %ld\n",getenv("USER"),score);
                        ret=location;
                        wroteit=1;
                }
                if(location<SAVE_TO_NUM-wroteit && location<itreached)
                        fprintf(scorefile,"%s : %ld\n",namebuff[location],scorebuff[location]);
        }
        fflush(scorefile);
        return ret;
}
void showscores(byte playerrank){
	if(playerrank == 0){
		char formername[60]={0};
		long formerscore=0;
		rewind(scorefile);
		fscanf(scorefile,"%*s : %*d\n");
		if ( fscanf(scorefile,"%s : %ld\n",formername,&formerscore)==2){
			printf("\n****CONRAGULATIONS!***\n");
			printf("     _____ You bet the\n");
			printf("   .'     |   previous\n");
			printf(" .'       |     record\n");
			printf(" |  .|    |         of\n");
			printf(" |.' |    |%11ld\n",formerscore);
			printf("     |    |    held by\n");
			printf("  ___|    |___%8s\n",formername);
			printf(" |            |\n");
			printf(" |____________|\n");
			printf("**********************\n");
		}
		
	}
	//scorefile is still open with w+
	char pname[60] = {0};
	long pscore=0;
	byte rank=0;
	rewind(scorefile);
	printf("\n>*>*>Top %d<*<*<\n",SAVE_TO_NUM);
	while( rank<SAVE_TO_NUM && fscanf(scorefile,"%s : %ld\n",pname,&pscore) == 2){
		if(rank == playerrank)
			printf(">>>");
		printf("%d) %s : %ld\n",rank+1,pname,pscore);
		rank++;
	}
	putchar('\n');
	
}
//apply gravity
bool fall(void){
	bool jfall,kfall,ret;
	jfall=kfall=ret=0;
	for(int y=LEN-1;y>0;y--){
		chtype c,d;
		for(int x=WID-1;x>=0;x--){
			c=board[y][x];
			d=board[y-1][x];
			if(!c && d){
				board[y-1][x]=0;
				board[y][x]=d;
				if(y-1==jy && x==jx)
					jfall=1;
				if((y-1==jy+ky) && (x==jx+kx))
					kfall=1;
				ret=1;
			}
		}
	}
	if(jfall&&kfall)
		jy++;
	else
		jy = LEN+1;
	return ret;
}
// rotate 90d clockwise in ky/x format
void clockwise(byte* y,byte* x){
		/*
		 o		x
		 x	xo	o    ox*/
		chtype fx,fy;
		if(*y)
			fy=0;
			fx=-*y;
		
		if(*x)
			fx=0;
			fy=*x;
		*y=fy;
		*x=fx;
}
			
//rtt jwls
bool rotate(void){//f:future
		if(jy>LEN)
			return 0;
		byte fy,fx;
		fy=ky;fx=kx;
		clockwise(&fy,&fx);
		if( jy+fy<0 || jy+fy>=LEN || jx+fx<0 || jx+fx>=WID )
			return 0;
		if(board[jy+fy][jx+fx])
			return 0;
		chtype a = board[jy+ky][jx+kx];
		board[jy+ky][jx+kx]=0;
		ky=fy;
		kx=fx;
		board[jy+ky][jx+kx]=a;
		return 1;
}
//mv jwls
bool jmove(byte dy,byte dx){
		if(jy>LEN)
			return 0;

		if(jx+dx>=WID || jx+dx<0 || jx+kx+dx>=WID ||jx+kx+dx<0 || jy+dx<0 ||jx+dx+kx<0)
			return 0;
		if( board[jy+ky+dy][jx+kx+dx] )
			if( !(jy+ky+dy == jy && jx+kx+dx==jx) )
				return 0;

		if( board[jy+dy][jx+dx])
				if(!(dx==kx && dy==ky))
					return 0;
		//still alive? 
		chtype a = board[jy][jx];
		chtype b = board[jy+ky][jx+kx];
		board[jy][jx]=0;
		board[jy+ky][jx+kx]=0;
		board[jy+dy][jx+dx]=a;
		board[jy+ky+dy][jx+kx+dx]=b;
		jy+=dy;jx+=dx;
		return 1;
}	
//scoring algorithm
bool explode(void){
	bool ret =0;
	chtype c,uc;
	byte n;
	byte y,x;
	for(y=0;y<LEN;y++){
		c=uc=n=0;
		for(byte x=0;x<WID;x++){
			uc = c; 
			c  = board[y][x];
			if(c && c == uc)
				n++;
			else
				n=0;
			if(n==3){
				ret=1;
				for(;n>=0;n--)
					board[y][x-n]=0;
				n=0;
				score+=30;
			}
		}
	}
	for(x=0;x<WID;x++){
		c=uc=n=0;
		for(byte y=0;y<LEN;y++){
			uc=c;
			c = board[y][x];
			if(c && c==uc)
				n++;
			else
				n=0;
			if(n==3){
				ret=1;
				for(;n>=0;n--)
					board[y-n][x]=0;
				n=0;
				score+=30;
			}
		}
	}
	return ret;
}

//display
void draw(void){
	erase();
	int middle = (COLS/2-1)-(WID/2);
	chtype a=A_STANDOUT|' ';
	mvhline(LEN,middle-2,a,WID+4);
	mvvline(0,middle-1,a,LEN);
	mvvline(0,middle-2,a,LEN);
	mvvline(0,middle+WID,a,LEN);
	mvvline(0,middle+WID+1,a,LEN);
	mvprintw(0,0,"Score:%d",score);
	for(byte y=0;y<LEN;y++){
		for(byte x=0;x<WID;x++){
			chtype c = board[y][x];
			if(c)
				mvaddch(y,middle+x,c);
		}
	}
	mvaddstr(LINES-2,middle-5,controls);
	refresh();
}
int main(void){
	initscr();
	cbreak();
	halfdelay(DELAY);
	noecho();
	curs_set(0);
	wnoutrefresh(stdscr);
	int input;
	bool falls;
	byte stop=0;
	char jwstr[] = {'*','^','~','"','$','V'};
	chtype colors[6]={0};
	if(has_colors()){
		start_color();
		use_default_colors();
		init_pair(1,COLOR_RED,-1);
		init_pair(2,COLOR_GREEN,-1);
		init_pair(3,COLOR_MAGENTA,-1);
		init_pair(4,COLOR_BLUE,-1);//array this thing
		init_pair(5,COLOR_YELLOW,-1);
		init_pair(6,COLOR_CYAN,-1);
		for(byte n=0;n<5;n++){
			colors[n] = COLOR_PAIR(n+1);
		}
	}

	srandom(time(NULL)%UINT_MAX);
	while(1){
		chtype a,b;
		a=board[0][WID/2];
		b=board[0][WID/2-1];
		if(a || b ){
			goto Lose;
		}
		jy=ky=0;
		jx=WID/2;
		kx=-1;
		byte ran1= random()%5;
		byte ran2= random()%5;
		board[jy][jx]=colors[ran1]|jwstr[ran1];
		board[jy+ky][jx+kx]=colors[ran2]|jwstr[ran2];
		falls = 1;
		while(falls){
			input = getch();

			if(input != ERR)
				stop+=1;

			if( stop >= 10){
				falls=fall();
				stop=0;
			}
			else if(input=='l')
				jmove(0,+1);
			else if(input=='j')
				jmove(0,-1);
			else if(input=='k')
				rotate();
			else if(input=='p'){
				mvaddstr(LINES-2,COLS/2-15,"Paused - Press a key to continue   ");
				refresh();
				nocbreak();
				cbreak();
				getch();
				halfdelay(DELAY);
			}
			else if(input=='q')
				goto Lose;
			else if(input==' ')
				while( (falls=fall()) )
					stop=0;
			else{
				falls=fall();
				stop=0;
			}
			draw();
	 	}
		while(explode()){ // explode, fall, explode, fall until nothing is left
			while(fall());
			draw();
		}
	}
	Lose:
	nocbreak();
	endwin();
	printf("%s _Jewels_ %s\n",jwstr,jwstr);
	printf("You have scored %ld points.\n",score);
	showscores(scorewrite(score));
	return EXIT_SUCCESS;
}
