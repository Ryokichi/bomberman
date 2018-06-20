/*Obs: o comando scanf tem algumas pecualiaridades que não conheço bem 
**quando se trata de de ler uma string com espaços.
**Por exemplo ele ve ser escrito exatamente assim:
**scanf(" %[^\n]s", variavel)
**[^\n] continua lendo a string até encontrar quebra linha
**já o espaço em branco antes do % pelo que li limpa o buffer do stdin
**caso esse buffer não seja limpo o proximo scanf fica com sujeira e é ignorado
**pelo que observei também atrapalha na hora de enviar msgs com socket
** fflush(stdin) ou setbuf(stdin, NULL) não funcionaram para mim
*/


#include "myfunctions.h"
using namespace std;

#define PORT_S  4242
#define PORT_C  4343
#define IP_S   "192.168.0.102"
#define IP_C   "192.168.0.104"
#define B_LEN  4096  ////tamanho do buffer para trocar mensagens
string paranaue;

///------Estrututra para conexões-----///
WSADATA wsa;
struct sockaddr_in localAddr, remoteAddr;
int sizeLocalAddr, sizeRemoteAddr;

///-----Variáveis Globais----///
int local_port, remote_port;
int local_sckt, remote_sckt, bind_sckt;
int i, j, align, collision ;
int sprite_size;
int dt = 60;
int battle_field[9][15];
char remote_ip[16];
//char s_buffer[B_LEN], r_buffer[B_LEN];
bool is_server, end_game = false;

Button btn_up, btn_down, btn_left, btn_right, btn_0, btn_1;

int plyr_no = 0;
Player player1, player2, player_aux;
Player player[3] = {player1, player2, player_aux};

Tile tile;
Bomb bomb[4];
Explosion explosion[4];


///-------------------------------------------------------------------------///
///Funcoes
///-------------------------------------------------------------------------///
void recebeDados();
void enviaDados(char *buffer_envio);
int  initSockets();
void salvaPosicoes();
void retornoPosicoes(char *enemy_pos);
void pressedButton(int btn_id);
void drawBattleField();
void drawPlayers();
void drawBombs();
void drawExplosion();
void releaseBomb(int player_num, int x, int y);
void bombExplodes (int index, int x, int y);
void checkDestruction(int x, int y);
void drawCredits();
///-------------------------------------------------------------------------///
///-------------------------------------------------------------------------///

int main(){
    clock_t t_ini;
    int delta_time, pg;
    int sprite_no = 0;
    
    pg          = 0;
    delta_time  = 0;    
    player[0].x = 64*1;
    player[0].y = 64*1;
    player[1].x = 64*13;    
    player[1].y = 64*7;

    player[0].to_x = player[0].x;
    player[0].to_y = player[0].y;
    player[1].to_x = player[1].x;    
    player[1].to_y = player[1].y;
    ///-------------------------///
    srand(time(NULL));
    is_server = whoWillBeServer();
    initSockets();
    openWindow();
    
    plyr_no = (is_server) ? 0 : 1;    
    
    player[0] = readSprites(player[0], "res/img/player1.bmp", imagesize(0,0,95,95));
    player[1] = readSprites(player[1], "res/img/player2.bmp", imagesize(0,0,95,95));
    tile = readTiles(tile);
    explosion[0] = readExplosion(explosion[0], "res/img/explosao.bmp", imagesize(0,0,63,63));
    explosion[1] = readExplosion(explosion[1], "res/img/explosao.bmp", imagesize(0,0,63,63));
    explosion[2] = readExplosion(explosion[2], "res/img/explosao.bmp", imagesize(0,0,63,63));
    explosion[3] = readExplosion(explosion[3], "res/img/explosao.bmp", imagesize(0,0,63,63));
    
    bomb[0] = readBombs (bomb[0], "res/img/bomba.bmp", imagesize(0,0,63,63));
    bomb[1] = readBombs (bomb[1], "res/img/bomba.bmp", imagesize(0,0,63,63));
    bomb[2] = readBombs (bomb[2], "res/img/bomba.bmp", imagesize(0,0,63,63));
    bomb[3] = readBombs (bomb[3], "res/img/bomba.bmp", imagesize(0,0,63,63));
    
    setMaze("res/img/maze1.bmp", battle_field);
    ///-------------------------///
  

    thread receber (recebeDados);
    t_ini = clock();
    setvisualpage(pg);
    while (!end_game){
    	if (GetKeyState(VK_ESCAPE)&0x8000) end_game = true;
        delta_time = clock() - t_ini;
        if (delta_time >= dt){
            t_ini = clock();
                            
            pg = (pg==0) ? 1 : 0;
            setactivepage(pg);
            cleardevice();
            
            if (player[0].hp > 0 && player[1].hp > 0){			            
            drawBattleField();
            drawBombs();
            drawExplosion();
            drawPlayers();            
            
            if (btn_up.is_active && (GetKeyState(VK_UP)&0x8000)) {
                pressedButton(10);
            }
            if (btn_down.is_active && (GetKeyState(VK_DOWN)&0x8000)){
                pressedButton(11);
            }
            if (btn_left.is_active && (GetKeyState(VK_LEFT)&0x8000)){
                pressedButton(12);
            }
            if (btn_right.is_active && (GetKeyState(VK_RIGHT)&0x8000)){
                pressedButton(13);
            }            
            if (btn_0.delay_time == 0 && ((GetKeyState(0x41)&0x8000) || (GetKeyState(VK_SPACE)&0x8000))){
                pressedButton(14);
            }
                        
            btn_up.delay_time    = (btn_up.delay_time    > 0) ? (btn_up.delay_time -= 1)    : 0;
            btn_down.delay_time  = (btn_down.delay_time  > 0) ? (btn_down.delay_time -= 1)  : 0;
            btn_left.delay_time  = (btn_left.delay_time  > 0) ? (btn_left.delay_time -= 1)  : 0;
            btn_right.delay_time = (btn_right.delay_time > 0) ? (btn_right.delay_time -= 1) : 0;
            btn_0.delay_time     = (btn_0.delay_time     > 0) ? (btn_0.delay_time -= 1)     : 0;
            
            btn_up.is_active    = (btn_up.delay_time    == 0) ? true : false;
            btn_down.is_active  = (btn_down.delay_time  == 0) ? true : false;
            btn_left.is_active  = (btn_left.delay_time  == 0) ? true : false;
            btn_right.is_active = (btn_right.delay_time == 0) ? true : false;
            btn_0.is_active     = (btn_0.delay_time     == 0) ? true : false;
            
            for (int i=0; i<4; i++){
                if (bomb[i].time > 0){
                    bomb[i].time -= dt;
                    if (bomb[i].time <=0 ) bomb[i].time = 0;
                    if (bomb[i].time == 0) bombExplodes(i, bomb[i].x, bomb[i].y);
                }
            }
            } else {
            	drawCredits();
			}
            
            setvisualpage(pg);
        }
    }
    closegraph();
    printf("---- FIM DE JOGO ----");
}

void drawExplosion() {
    int lin = 0;
    int col = 0;
    for (int i=0; i<4; i++){
        explosion[i].time -= dt;
        if (explosion[i].time > 0){
            lin = explosion[i].y/64;
            col = explosion[i].x/64;
            
            putimage(explosion[i].x, explosion[i].y, explosion[i].spriteB[0], AND_PUT);
            putimage(explosion[i].x, explosion[i].y, explosion[i].spriteA[0], OR_PUT);
            
            checkDestruction(explosion[i].x, explosion[i].y);
            
            if (battle_field[lin][col-1] == 0 ){
                putimage(explosion[i].x-64, explosion[i].y, explosion[i].spriteB[0], AND_PUT);
                putimage(explosion[i].x-64, explosion[i].y, explosion[i].spriteA[0], OR_PUT);
                checkDestruction(explosion[i].x-64, explosion[i].y);
                
                if (battle_field[lin][col-2] == 0 ){
                    putimage(explosion[i].x-128, explosion[i].y, explosion[i].spriteB[0], AND_PUT);
                    putimage(explosion[i].x-128, explosion[i].y, explosion[i].spriteA[0], OR_PUT);
                    checkDestruction(explosion[i].x-128, explosion[i].y);
                }
            }
            
            if (battle_field[lin][col+1] == 0 ){
                putimage(explosion[i].x+64, explosion[i].y, explosion[i].spriteB[0], AND_PUT);
                putimage(explosion[i].x+64, explosion[i].y, explosion[i].spriteA[0], OR_PUT);
                checkDestruction(explosion[i].x+64, explosion[i].y);
                if (battle_field[lin][col+2] == 0 ){
                    putimage(explosion[i].x+128, explosion[i].y, explosion[i].spriteB[0], AND_PUT);
                    putimage(explosion[i].x+128, explosion[i].y, explosion[i].spriteA[0], OR_PUT);
                    checkDestruction(explosion[i].x+128, explosion[i].y);
                }
            }
            
            if (battle_field[lin-1][col] == 0 ){
                putimage(explosion[i].x, explosion[i].y-64, explosion[i].spriteB[0], AND_PUT);
                putimage(explosion[i].x, explosion[i].y-64, explosion[i].spriteA[0], OR_PUT);
                checkDestruction(explosion[i].x, explosion[i].y-64);
                if (battle_field[lin-2][col] == 0 ){
                    putimage(explosion[i].x, explosion[i].y-128, explosion[i].spriteB[0], AND_PUT);
                    putimage(explosion[i].x, explosion[i].y-128, explosion[i].spriteA[0], OR_PUT);
                    checkDestruction(explosion[i].x, explosion[i].y-128);
                }
            }
            
            if (battle_field[lin+1][col] == 0 ){
                putimage(explosion[i].x, explosion[i].y+64, explosion[i].spriteB[0], AND_PUT);
                putimage(explosion[i].x, explosion[i].y+64, explosion[i].spriteA[0], OR_PUT);
                checkDestruction(explosion[i].x, explosion[i].y+64);
                if (battle_field[lin+2][col] == 0 ){
                    putimage(explosion[i].x, explosion[i].y+128, explosion[i].spriteB[0], AND_PUT);
                    putimage(explosion[i].x, explosion[i].y+128, explosion[i].spriteA[0], OR_PUT);
                    checkDestruction(explosion[i].x, explosion[i].y+128);
                }                
            }
        }
    }
}
void bombExplodes (int index, int x, int y){
    bomb[index].is_active = false;
    bomb[index].x = 9999;
    bomb[index].y = 9999;
    explosion[index].time = 250;
    explosion[index].x = x;
    explosion[index].y = y;    
}


void pressedButton(int btn_id){
	///btn_id code        
    ///10 --> up
    ///11 --> down
    ///12 --> left
    ///13 --> righ  
    ///14 --> bomb release  
        
    bool moved = false;    
    if (player[plyr_no].can_move == true){
        player[plyr_no].can_move = false;            
        
        int lin = player[plyr_no].y/64;
        int col = player[plyr_no].x/64;
		        
        if (btn_id == 10) {
            player[plyr_no].orientacao = 0;
            if (battle_field[lin-1][col]==0) {
                player[plyr_no].to_y -= 64;
                moved = true;
            }
        }
        if (btn_id == 11) {
        	player[plyr_no].orientacao = 1;
            if (battle_field[lin+1][col]==0) {
                player[plyr_no].to_y += 64;
                moved = true;
            }
        }
        if (btn_id == 12) {
        	player[plyr_no].orientacao = 2;
            if (battle_field[lin][col-1]==0) {
                player[plyr_no].to_x -= 64;
                moved = true;
            }
        }
        if (btn_id == 13) {
        	player[plyr_no].orientacao = 3;
            if (battle_field[lin][col+1]==0) {
                player[plyr_no].to_x += 64;
                moved = true;
            }
        }        
    }
    if (btn_id == 14 && player[plyr_no].x == player[plyr_no].to_x && player[plyr_no].y && player[plyr_no].to_y) {
        releaseBomb(plyr_no, player[plyr_no].x, player[plyr_no].y);
        moved = true;
    }
    if (moved) salvaPosicoes();
}

void releaseBomb(int plyr_num, int x, int y){
	int num = plyr_num*2;
	
    if (bomb[num].is_active == false) {
        bomb[num].time = 3999;
        bomb[num].x = x;
        bomb[num].y = y;
        bomb[num].is_active = true;
        btn_0.delay_time = 5;        
    }    
    else if (bomb[num+1].is_active == false) {
        bomb[num+1].time = 3999;
        bomb[num+1].x = x;
        bomb[num+1].y = y;
        bomb[num+1].is_active = true;
        btn_0.delay_time = 5;        
    }    
}


void salvaPosicoes(){
    string aux1, aux2;
    char buffer_envio[B_LEN];
    aux1 = "";
    
    aux2 = "0000" + to_string(player[plyr_no].to_x);
    aux1 = aux1 + aux2.substr(aux2.length()-4, 4);
            
    aux2 = "0000" + to_string(player[plyr_no].to_y);
    aux1 = aux1 + aux2.substr(aux2.length()-4, 4);
    
    
    aux2 = "0000" + to_string(bomb[(plyr_no*2)].x);
    aux1 = aux1 + aux2.substr(aux2.length()-4, 4);
    
    aux2 = "0000" + to_string(bomb[(plyr_no*2)].y);
    aux1 = aux1 + aux2.substr(aux2.length()-4, 4);
        
    aux2 = "0000" + to_string(bomb[(plyr_no*2)+1].x);
    aux1 = aux1 + aux2.substr(aux2.length()-4, 4);
    
    aux2 = "0000" + to_string(bomb[(plyr_no*2)+1].y);
    aux1 = aux1 + aux2.substr(aux2.length()-4, 4);
    
    strcpy(buffer_envio, aux1.c_str());
    enviaDados(buffer_envio);
    
}

void enviaDados(char *buffer_envio){    
    sendto(local_sckt, buffer_envio, B_LEN, 0, (struct sockaddr*)&remoteAddr, sizeRemoteAddr);   
}

void recebeDados(){    
    int recived;
    char r_buffer [B_LEN];
    char branco[] ="";
    while(true){    
        recived = recvfrom(local_sckt, r_buffer, B_LEN, 0, (struct sockaddr*)&remoteAddr, &sizeRemoteAddr);
        if (strcmp(r_buffer, branco) != 0){    
            retornoPosicoes(r_buffer);
        }
        memset(r_buffer, 0x0, B_LEN);
    }
}

void retornoPosicoes(char *enemy_pos){
    int enemy_no = 0;
    char c_char[B_LEN];
    string aux1, aux2;    
    
    aux2 = enemy_pos;
    enemy_no = (plyr_no==0) ? 1 : 0;    
    
    aux1 = aux2.substr(0,4);
    strcpy(c_char, aux1.c_str());
    player[enemy_no].to_x = atoi(c_char);
    
    aux1 = aux2.substr(4,4);
    strcpy(c_char, aux1.c_str());
    player[enemy_no].to_y = atoi(c_char);    
    
    int bomb_x = 9999;
    int bomb_y = 9999;
    aux1 = aux2.substr(8,4);
    strcpy(c_char, aux1.c_str());
    bomb_x = atoi(c_char);
    
    aux1 = aux2.substr(12,4);
    strcpy(c_char, aux1.c_str());
    bomb_y = atoi(c_char);
	    
    if (bomb_x < 9999 && bomb_y < 9999) {
        releaseBomb (enemy_no, bomb_x, bomb_y);
    }
    
    bomb_x = 9999;
    bomb_y = 9999;
    aux1 = aux2.substr(16,4);
    strcpy(c_char, aux1.c_str());
    bomb_x = atoi(c_char);
    
    aux1 = aux2.substr(20,4);
    strcpy(c_char, aux1.c_str());
    bomb_y = atoi(c_char);   
        
    if (bomb_x < 9999 && bomb_y < 9999) {
        releaseBomb (enemy_no, bomb_x, bomb_y);
    }    
}

void drawPlayers() {
	string life;
	char c_life[256];
    int sprite_no = 0;
    int passo     = 8;
    for (int i=0; i<2; i++) {
    	player[i].damage_time = (player[i].damage_time > 0) ? (player[i].damage_time -=1) : 0;    	
    	
        if (player[i].y > player[i].to_y) {
            player[i].y -= passo;
            player[i].orientacao = 0;
        }                        
        if (player[i].y < player[i].to_y) {
            player[i].y += passo;
            player[i].orientacao = 1;
        }        
        if (player[i].x > player[i].to_x) {
            player[i].x -= passo;
            player[i].orientacao = 2;
        }
        if (player[i].x < player[i].to_x) {
            player[i].x += passo;
            player[i].orientacao = 3;
        }
        
        player[i].current_sprite ++;
        if (player[i].current_sprite > 3) player[i].current_sprite = 0;
        
        if ((player[i].x == player[i].to_x) && (player[i].y == player[i].to_y)) {
            player[i].current_sprite = 0;
            player[i].can_move = true;
        }
        
        sprite_no = player[i].current_sprite + (player[i].orientacao * 4);
        if (player[i].damage_time % 2 == 0){
            putimage(player[i].x-16, player[i].y-32, player[i].spriteB[sprite_no], AND_PUT);
            putimage(player[i].x-16, player[i].y-32, player[i].spriteA[sprite_no], OR_PUT);
        }
        
        life = "Player " + to_string(i+1) + " = " + to_string(player[i].hp);        
        strcpy(c_life, life.c_str());
        
        setcolor(0);
        settextstyle(0, 0, 2);
        outtextxy(i*512 + 150, 600, c_life);
    }    
}

void drawBattleField(){    
    int tile_no = 0;
    for (int i=0; i<9; i++){
        for (int j=0; j<15; j++){
            tile_no = battle_field[i][j];
            putimage(j*64, i*64, tile.sprite[tile_no],0);
        }
    }
}

void drawBombs() {
    int sprite_num = 0;
    for (int i=0; i<4; i++){
        if (bomb[i].time > 0) {
            sprite_num = bomb[i].time / 1000;
            putimage (bomb[i].x, bomb[i].y, bomb[i].spriteB[sprite_num], AND_PUT);
            putimage (bomb[i].x, bomb[i].y, bomb[i].spriteA[sprite_num], OR_PUT);
        }
    }
}

void checkDestruction(int x, int y) {
	int difx = 9999;
	int dify = 9999;
	for (int i=0; i<2; i++) {
		if (player[i].damage_time == 0){		
		    difx = abs(player[i].x - x);
		    dify = abs(player[i].y - y);		
		    if (difx < 32 && dify < 32) {
			    player[i].hp -= 1;
			    player[i].damage_time = 15;
		    }
	    }		
	}
	for (int i=0; i<4; i++) {
		if (bomb[i].time > 0) {
		    difx = abs(bomb[i].x - x);
		    difx = abs(bomb[i].y - y);
		    if (difx < 32 && dify < 32) {
			    bomb[i].time = 1;
		    }
	    }
	}
}

void drawCredits() {
	string texto;
	char c_aux[B_LEN];
	int vencedor;
	
	vencedor = (player[0].hp > player[1].hp) ? 1 : 2;
	
	setcolor(0);
	texto = "Player "+to_string(vencedor)+" foi o vencedor.";
	strcpy(c_aux, texto.c_str());
	settextstyle(0, 0, 50);
	outtextxy(20, 120, c_aux);	
	
	
	settextstyle(0, 0, 30);
	outtextxy(20, 500, "Integrantes:");
	outtextxy(80, 520, "CAROLINE GONCALVES DE FELIPE");
	outtextxy(80, 540, "HEITOR RYOKICHI NAKAMURA");
	outtextxy(80, 560, "RENAN ALVES NOWAK");
	outtextxy(80, 580, "WILSON RICARDO DA SILVA FABOZI");
	
		
}

int initSockets(){
    char s_buffer[B_LEN], r_buffer [B_LEN];
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0){
        printf("Failed. Error Code : %d",WSAGetLastError());
        return 1;
    }
    printf("Initialised.\n");
         
    //Create a socket    
    local_sckt = socket(AF_INET, SOCK_DGRAM, 0);
    if (local_sckt < 0){
        printf("Falha. Nao foi possivel criar socket\n");
        return 1;
    }
    else {
        printf("Sucesso. Socket criado\n");
    }
   
    local_port  = (is_server) ? PORT_S : PORT_C;
    remote_port = (is_server) ? PORT_C : PORT_S;
    strcpy(remote_ip,  ((is_server) ? IP_C : IP_S));
    
    localAddr.sin_family     = AF_INET;
    localAddr.sin_port       = htons(local_port);
    memset(localAddr.sin_zero, 0x0, 8);

    remoteAddr.sin_family      = AF_INET;
    remoteAddr.sin_port        = htons(remote_port);
    remoteAddr.sin_addr.s_addr = inet_addr(remote_ip);
    memset(remoteAddr.sin_zero, 0x0, 8);

    sizeLocalAddr  = sizeof(localAddr);
    sizeRemoteAddr = sizeof(remoteAddr);

    bind_sckt = bind(local_sckt, (struct sockaddr*)&localAddr, sizeLocalAddr);
    if (bind_sckt < 0){
        printf("Falha. Erro ao vincular porta com socket\n");
        return 1;
    }
    else
        printf("Sucesso. Vinculo de porta com socket efetuado\n");

    if (is_server){
        printf("Aguardando conexao do cliente\n");
        recvfrom(local_sckt, r_buffer, B_LEN, 0, (struct sockaddr*)&remoteAddr, &sizeRemoteAddr);
        printf("MSG_cliente: %s \n", r_buffer);
        memset(r_buffer, 0x0, B_LEN);
            
        memset(s_buffer, 0x0, B_LEN);
        strcpy(s_buffer, "Saudacoes cliente, estou pronto. Vamos sim.\n");
        sendto(local_sckt, s_buffer, B_LEN, 0, (struct sockaddr*)&remoteAddr, sizeRemoteAddr);
    }
    else {
        memset(s_buffer, 0x0, B_LEN);
        strcpy(s_buffer, "Saudacoes servidor, estou pronto. Vamos jogar?\n");
        sendto(local_sckt, s_buffer, B_LEN, 0, (struct sockaddr*)&remoteAddr, sizeRemoteAddr);
        recvfrom(local_sckt, r_buffer, B_LEN, 0, (struct sockaddr*)&remoteAddr, &sizeRemoteAddr);
        printf("Msg servidor: %s \n", r_buffer);
    }
    return 0;
}
