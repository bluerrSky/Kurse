#include <ncurses.h>
#include <ctype.h>
#include <signal.h> 
#include <stdlib.h>
int vimMode = 0;
int vimMotionCh;
int vimmedScrX=0, vimmedScrY=0;


struct prevScrSize{
	int scrHeight;
	int scrWidth;
} prevScr; 
int isChBackSpace  = 0;
void handlePress(char *text, int* currTextSize, char ch){ 
	if(ch == KEY_BACKSPACE || ch == 127 || ch == 8){
		if(*currTextSize > 0){
			(*currTextSize)--;
			isChBackSpace = 1;
		}
		text[*currTextSize] = '\0';
	}else if(ch == KEY_RESIZE){
	}	
	else{
		text[*currTextSize] = ch;
		(*currTextSize)++;
	}	
}
void scrollUp(char* text, int* scrollIndex, int* i){
	for(int k = *scrollIndex - 2; k >= 0; k--){
		if(text[k] == '\n'){
			*i = *scrollIndex = k+1;
			break;
		}
		*i = *scrollIndex = 0;
	}
	clear();
	move(0,0);
}
void scrollDown(char* text, int* scrollIndex, int* currTextSize, int* i, int* scrY,  int* scrX){
	clear();
	//find first \n
	int l;
	for( l =*scrollIndex;l < *currTextSize; l++){
		if(text[l] == '\n'){
			break;
		}
	}
	*i = *scrollIndex = l+1;	
	*scrX = *scrY = 0;
}
					
void moveUpAtEnd(char* text, int currTextSize, int scrWidth, int scrollIndex){
	int newY = 0, newX=0, iter =scrollIndex;
	getyx(stdscr, vimmedScrY, vimmedScrX);
	for(; iter < currTextSize && newY < vimmedScrY; iter++){ 
		if(text[iter] == '\n' || newX+1 == scrWidth ){
			if((newY+1) < vimmedScrY){
				newY++;
				newX=0;
			}else{
				break;
			}
		}
		else{
	 		newX++;
		}
	}
						
	move(newY, newX);
}
void moveDownAtEnd(char* text, int currTextSize, int scrWidth, int scrollIndex){
	int newY = 0, newX=0, iter =scrollIndex;
	getyx(stdscr, vimmedScrY, vimmedScrX);
	for(; iter < currTextSize && newY <= vimmedScrY+1; iter++){ 
		if(text[iter] == '\n' || newX+1 == scrWidth ){
			if((newY+1) <= vimmedScrY+1){
				newY++;
				newX=0;
			}else{
				break;
			}
		}
		else{
	 		newX++;
		}
	}
	move(newY, newX);
}
						
	
void displayOnScr(char* text, int currTextSize){
	static int scrollIndex; 
	int scrY, scrX,scrHeight, scrWidth;
	getyx(stdscr, scrY, scrX);
	getmaxyx(stdscr, scrHeight, scrWidth);
	int i; 
	int goUp = 0;
	if(prevScr.scrHeight == scrHeight && prevScr.scrWidth == scrWidth){
		if(!vimMode){
				if(isChBackSpace){
					if(scrX == 0 && scrY == 0 && scrollIndex > 0){
						scrollUp(text, &scrollIndex, &i);
					}else if(scrX == 0){
						moveUpAtEnd(text, currTextSize, scrWidth, scrollIndex);
					}else{
						move(scrY, scrX-1);
					}
					delch();
					isChBackSpace = 0;
					refresh();
				}else{
					//code for scrolling
					if((scrY == scrHeight-1  && scrX == scrWidth-1) || (scrY == scrHeight-1 && text[currTextSize-1] == '\n')){
						scrollDown(text,&scrollIndex, &currTextSize, &i, &scrY,  &scrX); 
					}else{
						i = currTextSize-1;
					}
				}
		}
		else{
			switch(vimMotionCh){
				case 'h':
					move(scrY, scrX-1);
					break;
				case 'l':
					move(scrY, scrX+1);
					break;
				case 'j':
					if(scrY == scrHeight-1){	
						scrollDown(text,&scrollIndex, &currTextSize, &i, &scrY,  &scrX); 
					}else{
						moveDownAtEnd(text, currTextSize, scrWidth, scrollIndex);

					}
					break;	
				case 'k':	
					if(scrY == 0){
						if(scrollIndex != 0){
							scrX = 0;
							scrollUp(text, &scrollIndex, &i);
							goUp= 1;
						}
					}else{
						i = currTextSize;
						moveUpAtEnd(text, currTextSize, scrWidth, scrollIndex);
					}
					break;
			}
		}
		refresh();
			
	}else{
		i  = 0;


		prevScr.scrHeight = scrHeight;
		prevScr.scrWidth = scrWidth;
		clear();
		scrY = scrX = 0;
	}

	for(; i < currTextSize; i++){
		//isprint to handle  the resizing character(unprintable)
		if(i >= currTextSize) break;
		if(isprint(text[i]) || text[i]  == '\n'){
			if(text[i] == '\n'){
				scrY++;
				scrX = 0;
			}else{
				scrX++;
				if(scrX == scrWidth){
					scrY++;	
					scrX=0;
				}
				addch(text[i]);		
			}
			move(scrY, scrX);
		}
		refresh();
	}
	if(goUp == 1){
		move(1,0);
		moveUpAtEnd(text, currTextSize, scrWidth, scrollIndex);
		goUp = 0;
		refresh();
	}
}
	
int main(){
	initscr();
	//setting scr struct size which works to redraw the screen only if it resized
	prevScr.scrHeight = LINES;
	prevScr.scrWidth = COLS;
	noecho();
	//to handle resizing defining signal
	int textSize = LINES*COLS;
	char* text = malloc(sizeof(char) * textSize);
	char ch;	
	int currTextSize = 0;
	while(1){
		ch = getch();
		if(ch == 27){
			if(vimMode == 0){
				vimMode = 1;
			}else{
				break;
			}
		}
			
		if(currTextSize == textSize -1){
			textSize *= 2;
			text = realloc(text, sizeof(char) * textSize);	
			if(text == NULL){
				free(text);
				endwin();
				printf("Could not create input text buffer\n");
				return 1;
			}
		}
		if(!vimMode){
			handlePress(text,&currTextSize, ch); 
			displayOnScr(text, currTextSize);
		}else{
			vimMotionCh = ch;
			displayOnScr(text, currTextSize);
		}
			
	}
	endwin();	
	return 0;	
}


