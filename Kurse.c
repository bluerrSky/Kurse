#include <ncurses.h>
#include <ctype.h>
#include <signal.h> 
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>


int vimMode = 0;
int vimMotionCh;
int vimmedScrX=0, vimmedScrY=0;


struct prevScrSize{
	int scrHeight;
	int scrWidth;
} prevScr; 
int isChBackSpace  = 0;
int globalCurrIndex = 0;
int xHelperVar = 1;

char* sliceString(const char* str, int a, int b) {
    if (a >= b || a < 0 || b > strlen(str)) {
        return NULL;    }
    
    int length = b - a;
    char* sliced_str = (char*)malloc((length + 1) * sizeof(char));      
    if (sliced_str == NULL) {
        return NULL;     }
    
    memcpy(sliced_str, str + a, length); 
    sliced_str[length] = '\0'; 

    return sliced_str;
}

				
void addChInArr(int *currTextSize, char ch, char* text){
	if(globalCurrIndex < *currTextSize){
		for(int i = *currTextSize; i > globalCurrIndex; i--){
			text[i] =  text[i-1];
		}
	}
	text[globalCurrIndex] = ch;
	globalCurrIndex++;
	(*currTextSize)++;
}	
void handlePress(char *text, int* currTextSize, char ch){ 
	if(ch == KEY_BACKSPACE || ch == 127 || ch == 8){
		if(*currTextSize > 0 && globalCurrIndex > 0){
			(*currTextSize)--;
			globalCurrIndex--;
			isChBackSpace = 1;
			if(globalCurrIndex  != *currTextSize){
				for(int i = globalCurrIndex; i < *currTextSize; i++){
					text[i] = text[i+1];
				}
			}
			text[*currTextSize] = '\0';
			
		}
	}else if(ch == KEY_RESIZE){
	}	
	else{
		/*
		text[*currTextSize] = ch;
		(*currTextSize)++;
		*/
		addChInArr(currTextSize, ch, text);
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
void moveLineUp(char *text){
	for(int i = globalCurrIndex; i >= 0; i--){
		if(text[i] == '\n'){
			if(text[i-1] != '\n' || (text[i-1] == '\n' && text[globalCurrIndex] == '\n')){
				if((text[i-1] == '\n' && text[globalCurrIndex] == '\n') && ((i-2) > 0 && text[i-2] != '\n')){
					globalCurrIndex = i-2;
				}else{
					globalCurrIndex = i-1;
				}
			}else{	
				globalCurrIndex= i;
			}
	
			break;	
		}
	}
}	
void moveLineDown(char *text, int currTextSize){
	int flag = 0;
	int charInBetweenLines = 0;
	for(int i = globalCurrIndex; i <= currTextSize; i++){
		if(text[i] == '\n' || i == currTextSize){
			if(!flag){
				flag = 1;
				continue;
			}else{
				if(charInBetweenLines  == 0){
					globalCurrIndex = i;
				}else{
					globalCurrIndex = i-1;	
				}
				break;
			}
		}
		if(flag){
			charInBetweenLines++;
		}
	}	
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
			i = currTextSize;
			switch(vimMotionCh){
				case 'h':
					if(globalCurrIndex > 0 && text[globalCurrIndex-1] != '\n'){
						globalCurrIndex--;
					}
					//move(scrY, scrX-1);
					break;
				case 'l':
					if(globalCurrIndex < currTextSize && text[globalCurrIndex+1] != '\n'){
						globalCurrIndex++;
					}
					//move(scrY, scrX+1);
					break;
				case 'j':
					xHelperVar=0;
					if(scrY == scrHeight-1){	
						scrollDown(text,&scrollIndex, &currTextSize, &i, &scrY,  &scrX); 
					}else{
						moveDownAtEnd(text, currTextSize, scrWidth, scrollIndex);

					}
					moveLineDown(text, currTextSize);
					break;	
				case 'k':	
					xHelperVar=0;
					if(scrY == 0){
						if(scrollIndex != 0){
							scrX = 0;
							scrollUp(text, &scrollIndex, &i);
							goUp= 1;
						}
					}else{
						moveLineUp(text);
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
	clear();
	i = scrollIndex;
	int tempCordX, tempCordY;
	scrX = 0;scrY = 0;
	for(; i <= currTextSize; i++){
		//isprint to handle  the resizing character(unprintable)
		if(i == globalCurrIndex){
			tempCordX = scrX;tempCordY = scrY;
		}
		if(i >= currTextSize  || i < 0) break;
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
				//addch(text[i]);		
			}
		}
	}
	char* slicedStr = sliceString(text, scrollIndex, currTextSize);
	addstr(slicedStr);
	FILE* debugFile = fopen("debug.txt","a");
	fprintf(debugFile, "GlobalCurrIndex: %d\tCurrTextSize: %d\n", globalCurrIndex, currTextSize);
	fprintf(debugFile,"Printing %s\t",text);
	fprintf(debugFile,"moving to %d %d\n", tempCordY, tempCordX);
	fclose(debugFile);
	move(tempCordY,tempCordX);
	refresh();
	if(goUp == 1){
		move(1,0);
		moveLineUp(text);
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
	char* text = calloc(textSize,sizeof(char));
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
		if(ch == 'i' && vimMode == 1){
			vimMode = 0;
			continue;
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
