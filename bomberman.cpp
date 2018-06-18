/*Obs: o comando scanf tem algumas pecualiaridades que não conheço bem 
**quando se trata de de ler uma string com espaços.
**Por exemplo ele ve ser excrito exatamente assim:
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
int  local_sckt, remote_sckt, bind_sckt;
int  i, j, align, collision ;
int sprite_size;
int battle_field[9][15];
char remote_ip[16];
//char s_buffer[B_LEN], r_buffer[B_LEN];
bool is_server, end_game = false;

Button btn_up, btn_down, btn_left, btn_right, btn_0, btn_1;

int plyr_no = 0;
Player player1, player2, player_aux;
Player player[3] = {player_aux, player1, player2};

Tile tile;


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
///-------------------------------------------------------------------------///
///-------------------------------------------------------------------------///

int main(){
    clock_t t_ini;
    int delta_time, pg;
    int sprite_no = 0;

    pg          = 0;
    delta_time  = 0;    
    player[1].x = 64*1;
    player[1].y = 64*1;
    player[2].x = 64*13;    
    player[2].y = 64*7;

    player[1].to_x = player[1].x;
    player[1].to_y = player[1].y;
    player[2].to_x = player[2].x;    
    player[2].to_y = player[2].y;
    ///-------------------------///
    srand(time(NULL));
    is_server = whoWillBeServer();
    initSockets();
    openWindow();
    
    plyr_no = (is_server) ? 1 : 2;    
    
    player[1] = readSprites(player[1], "res/img/player1.bmp", imagesize(0,0,63,63));    
    player[2] = readSprites(player[2], "res/img/player2.bmp", imagesize(0,0,63,63));
    tile = readTiles(tile);
    setMaze("res/img/maze1.bmp", battle_field);
    ///-------------------------///
  

    thread receber (recebeDados);
    t_ini = clock();
    setvisualpage(pg);   
    while (!end_game){        
        delta_time = clock() - t_ini;
        if (delta_time >= 80){
            t_ini = clock();
                            
            pg = (pg==0) ? 1 : 0;
            setactivepage(pg);
            cleardevice();    
            
            drawBattleField();
            drawPlayers();
            
            if (GetKeyState(VK_ESCAPE)&0x8000) end_game = true;
            if (btn_up.is_active && (GetKeyState(VK_UP)&0x8000)){            
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
            
            btn_up.delay_time    = (btn_up.delay_time    > 0) ? (btn_up.delay_time--)    : 0;
            btn_down.delay_time  = (btn_down.delay_time  > 0) ? (btn_down.delay_time--)  : 0;
            btn_left.delay_time  = (btn_left.delay_time  > 0) ? (btn_left.delay_time--)  : 0;
            btn_right.delay_time = (btn_right.delay_time > 0) ? (btn_right.delay_time--) : 0;
            
            btn_up.is_active    = (btn_up.delay_time    == 0) ? true : false;
            btn_down.is_active  = (btn_down.delay_time  == 0) ? true : false;
            btn_left.is_active  = (btn_left.delay_time  == 0) ? true : false;
            btn_right.is_active = (btn_right.delay_time == 0) ? true : false;
            
            setvisualpage(pg);            
        }
    }
    closegraph();
    printf("---- FIM DE JOGO ----");
}

void pressedButton(int btn_id){    
    if (player[plyr_no].can_move == true){
        player[plyr_no].can_move = false;    
        ///btn_id code
        ///0 --> release current bomb
        ///10 --> up
        ///11 --> down
        ///12 --> left
        ///13 --> righ        
        
        int lin = player[plyr_no].y/64;
        int col = player[plyr_no].x/64;

        bool moved = false;
        if (btn_id == 10) {
            player[plyr_no].orientacao = 0;
            if (battle_field[lin-1][col]==0) {                        
                player[plyr_no].to_y -= 64;
                moved = true;
            }
        }
        if (btn_id == 11) {            
            if (battle_field[lin+1][col]==0) {                      
                player[plyr_no].to_y += 64;
                moved = true;
            }
        }
        if (btn_id == 12) {            
            if (battle_field[lin][col-1]==0) {                      
                player[plyr_no].to_x -= 64;
                moved = true;
            }
        }
        if (btn_id == 13) {            
            if (battle_field[lin][col+1]==0) {                      
                player[plyr_no].to_x += 64;
                moved = true;
            }
        }        
        if (moved) salvaPosicoes();
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
    
    strcpy(buffer_envio, aux1.c_str());    
    enviaDados(buffer_envio);    
    
}

void enviaDados(char *buffer_envio){
    sendto(local_sckt, buffer_envio, B_LEN, 0, (struct sockaddr*)&remoteAddr, sizeRemoteAddr);   
}

void recebeDados(){    
    int recived;
    char r_buffer [B_LEN];
    while(true){    
        recived = recvfrom(local_sckt, r_buffer, B_LEN, 0, (struct sockaddr*)&remoteAddr, &sizeRemoteAddr);        
        retornoPosicoes(r_buffer);       
    }
}

void retornoPosicoes(char *enemy_pos){
    int enemy_no = 0;
    char c_char[B_LEN];
    string aux1, aux2;    
    
    aux2 = enemy_pos;
    enemy_no = (plyr_no==1) ? 2 : 1;    
    
    aux1 = aux2.substr(0,4);
    strcpy(c_char, aux1.c_str());
    player[enemy_no].to_x = atoi(c_char);
    
    aux1 = aux2.substr(4,4);
    strcpy(c_char, aux1.c_str());
    player[enemy_no].to_y = atoi(c_char);
    
}

void drawPlayers() {    
    int sprite_no = 0;
    int passo     = 8;    
    for (int i=1; i<=2; i++){        
        if (player[i].y > player[i].to_y) {
            player[i].y -= passo;
            player[i].orientacao = 1;
        }                        
        if (player[i].y < player[i].to_y) {
            player[i].y += passo;
            player[i].orientacao = 0;
        }        
        if (player[i].x < player[i].to_x) {
            player[i].x += passo;
            player[i].orientacao = 2;
        }
        if (player[i].x > player[i].to_x) {
            player[i].x -= passo;
            player[i].orientacao = 3;
        }
        
        player[i].current_sprite ++;
        if (player[i].current_sprite > 3) player[i].current_sprite = 0;
        
        if ((player[i].x == player[i].to_x) && (player[i].y == player[i].to_y)){  
            player[i].current_sprite = 0;
            player[i].can_move = true;            
        }
        
        sprite_no = player[i].current_sprite + (player[i].orientacao * 4);        
        putimage(player[i].x, player[i].y, player[i].spriteB[sprite_no], AND_PUT);
        putimage(player[i].x, player[i].y, player[i].spriteA[sprite_no], OR_PUT);
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


/*
    string aux1, aux2;
    //string paranaue;   
    string algo;
    string algo2;
    algo = "01234abcdef4556987";
    algo2 = algo.substr(5,4);
    strcpy(s_buffer, e_msg.c_str());
    cout << s_buffer << endl;
    cout << "algo2 = " << algo2 << endl;
    string paranaue ("Another character sequence", 12);
    for (int i=0; i<paranaue.length(); i++){
        printf("%c\n", paranaue[i]);
    }
    cout << paranaue << endl;
    cout << paranaue.length() << endl;
    */    

