#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <graphics.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <iostream>
#include <thread>
using namespace std;


struct Player {
    int id = 0;
    int x = 0;
    int y = 0;
    int to_x = 0;
    int to_y = 0;
    int current_sprite = 0;
    int orientacao = 0;
    bool can_move = true;
    unsigned char *spriteA[16];
    unsigned char *spriteB[16];
};

struct Button{
	bool is_active = true;
	int delay_time;	
};

struct Tile{
	void *sprite[16];
};

struct Bomb {
	int time;
	void *sprite[0];
};


void openWindow() {
    initwindow(1024, 640, "Bomberman", 60,60);
    setbkcolor(0);    
    setactivepage(0);
    cleardevice();
    setvisualpage(0);
}


void applyMask(int Tam, unsigned char *Img, unsigned char *Msk){
	int i = 0;
	unsigned char B, G, R;            
    B = Img[24];
    G = Img[25];
    R = Img[26];           

    for(i=24; i < Tam; i+=4) {
        if (Img[i]==B and Img[i+1]==G and Img[i+2]==R) {
            Img[i]   = 0;
            Img[i+1] = 0;
            Img[i+2] = 0;
            Msk[i]   = 255;
            Msk[i+1] = 255;
            Msk[i+2] = 255;
        }
        else {
            Msk[i]   = 0;
            Msk[i+1] = 0;
            Msk[i+2] = 0;
        }
    }
}

Player readSprites(Player plyr, const char *path, int sprite_size){	
	setactivepage(2);
	cleardevice();	
	readimagefile(path, 0, 0, 256,256);
	
	int i=0, j=0, top=0, bot=0, left=0, right=0;
    
	for(i=0; i<4; i++){		
	    for (j=0; j<4; j++){
		    plyr.spriteA[(i*4+j)] = (unsigned char *)malloc(sprite_size);
		    plyr.spriteB[(i*4+j)] = (unsigned char *)malloc(sprite_size);
		    
		    left =j*64;
		    top  =i*64;
		    right=((1+j)*64)-1;
		    bot  =((1+i)*64)-1;		    
		    getimage(left, top, right, bot, plyr.spriteA[(i*4+j)]);
            getimage(left, top, right, bot, plyr.spriteB[(i*4+j)]);
            
            applyMask(sprite_size,plyr.spriteA[(i*4+j)], plyr.spriteB[(i*4+j)] );
	    }
	}    
	return plyr;		
}



Tile readTiles( Tile tile){	
	string path;
	int img_size = imagesize(0,0,63,63);
	for (int i=0; i<3; i++){
		tile.sprite[i] = malloc(img_size);
		
		path = ("res/img/tile"+to_string(i)+".bmp");
						
		setactivepage(2);
        cleardevice();        	
	    readimagefile(path.c_str(), 0, 0, 63,63);
	    getimage(0, 0, 63, 63, tile.sprite[i]);	    
	}
	return tile;
}

void setMaze(const char *path, int maze[9][15]){    
    /*
    **0  = RGB(0  ,  0,  0) - Preto
    **1  = RGB(0  ,  0,128) - Azul
    **2  = RGB(0  ,128,  0) - Verde
    **3  = RGB(0  ,128,128) - Azul claro
    **4  = RGB(128,  0,  0) - Vermelho
    **5  = RGB(128,  0,128) - Roxo
    **6  = RGB(128,128,  0) - Verde/Marrom
    **7  = RGB(192,192,192) - Cinza claro
    **8  = RGB(128,128,128) - Cinza
    **9  = RGB(128,128,255) - Azul médio
    **10 = RGB(128,255,128) - Verde claro
    **11 = RGB(128,255,255) - Azul/Verde Claro
    **12 = RGB(255,128,128) - Rosa
    **13 = RGB(255,128,255) - Rosa claro
    **14 = RGB(255,255,  0) - Amarelo
    **15 = RGB(255,255,255) - Branco
    */   
    
	//int maze[135];
    
    int cor;            
    readimagefile(path, 0, 0, 15, 9);        
    for (int i=0; i<9; i++){
        for (int j=0; j<15; j++){
            cor = getpixel(j,i);            
            cor = (cor < 15) ? cor : 15;            
            maze[i][j] = cor;            
        }
    }	
}

bool whoWillBeServer(){
    int want_be_server;
    
    printf("Deseja ser 0-Cliente ou 1-Servidor : ");
    scanf("%d", &want_be_server);
    
    if (want_be_server != 0){    
        printf("Vc sera o servidor\n");
        return true;       
    }
    else{
        printf("Vc sera o cliente\n");
    }
    return false;
}


