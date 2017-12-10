/*****************************************************************
ファイル名	: server_main.c
機能		: サーバーのメインルーチン
製作者：澁谷
*****************************************************************/

#include<SDL/SDL.h>
#include"server_common.h"
static COMMAND data;
CONTAINER maincharadata[MAX_CLIENTS];
GAMEINFO info;
static void sendinfo(void);
static void timeof(void);
static void team(void);
static Uint32 SignalHandler(Uint32 interval, void *param);

int main(int argc, char *argv[])
{
	int	num;
	int	endFlag = 2;
	u_short port = DEFAULT_PORT;
	switch(argc) {
  		case 1:
    		break;
    		default:
    			fprintf(stderr, "Weite: ./server \n");
    		return 1;
	}
	/* SDLの初期化 */
	if(SDL_Init(SDL_INIT_TIMER) < 0) {
		printf("failed to initialize SDL.\n");
		exit(-1);
	}

	/* クライアントとの接続 */
	if(SetUpServer(MAX_CLIENTS,port) == -1){
		fprintf(stderr,"Cannot setup server\n");
		exit(-1);
	}
	team();
	Uint32 sec = SDL_GetTicks();
	info.round = 0;
	/* メインイベントループ */
	while(endFlag){
		int i;
		int a=0,b=0;
		switch(endFlag){
			case 1:
				while(endFlag==1){
					endFlag = SendRecvManager(endFlag);
					if(SDL_GetTicks() > sec){
						sec += 1000;
						sendinfo();
						if(info.timer == 0){
							a=0;
							b=0;
							for(i = 0;i<MAX_CLIENTS;i++){
								if(maincharadata[i].team == 'A')		
									a += maincharadata[i].hit;
								if(maincharadata[i].team == 'B')
									b += maincharadata[i].hit;
		
							}
							if(a>b)
								info.shohai[0]+=1;
							else if(b>a)
								info.shohai[1]+=1;
							if(info.round == MAX_ROUND||info.shohai[0] == 2||info.shohai[1] == 2){
								endFlag = 3;
								data.command = ROUNDEND;
								data.i = 0;
								data.j = 0;
								data.k = 0;
								SendData(ALL_CLIENTS,&data,sizeof(COMMAND));
							}else{
								data.command = ROUNDEND;
								data.i = -1;
								data.j = -1;
								data.j = -1;
								SendData(ALL_CLIENTS,&data,sizeof(COMMAND));
								endFlag = 2;
							}
						}
					}
				}
				
				break;
			case 2:
				Initchar();
				while(endFlag == 2){
					endFlag = SendRecvManager(endFlag);
				}
				sec = SDL_GetTicks()+2000;
				break;
			case 3:
				data.command = SCORE;
				data.i = info.shohai[0];
				data.j = info.shohai[1];
				SendData(ALL_CLIENTS,&data,sizeof(COMMAND));
				while(endFlag == 3){
					endFlag = SendRecvManager(endFlag);
				}
				break;
		}
	};

	/* 終了処理 */
	Ending();

	return 0;
}
void timeof(){
	int i;
	for(i=0;i<MAX_CLIENTS;i++){
		if(maincharadata[i].skillf == 1){
			if(info.skilltimer[i]>0){
				info.skilltimer[i] -= 1;
				if(info.skilltimer[i] <= 0){
					maincharadata[i].skillf = 0;
					info.skilltimer[i] = 0;
					data.command = SPECIAL;
					data.i = 0;
					data.j = 0;
					data.k = i;
					SendData(ALL_CLIENTS,&data,sizeof(COMMAND));		
				}
			}
		}
			
	}
}
/********************************
関数名：sendinfo
機能：ゲーム情報の整理と送信
********************************/
void sendinfo(){
	timeof();
	info.timer -= 1;
	data.command = INFO;
	data.i = info.timer;
	data.j = info.round;
	SendData(ALL_CLIENTS,&data,sizeof(COMMAND));
	int i;
	int a=0,b=0;
	for(i = 0;i<MAX_CLIENTS;i++){
		if(maincharadata[i].team == 'A')		
			a += maincharadata[i].hit;
		if(maincharadata[i].team == 'B')
			b += maincharadata[i].hit;
		
	}
	data.command = SCORE;
	data.i = a;
	data.j = b;
	SendData(ALL_CLIENTS,&data,sizeof(COMMAND));	
}
/************************************************************
関数名：team
機能：チーム管理
 ************************************************************/
void team(void){
	if(MAX_CLIENTS/2 == 2){
		maincharadata[0].team = 'A';
		maincharadata[1].team = 'A';
		maincharadata[2].team = 'B';
		maincharadata[3].team = 'B';
	}else if(MAX_CLIENTS/2 == 1){
		maincharadata[0].team = 'A';
		maincharadata[1].team = 'B';
	}else{
		maincharadata[0].team = 'A';
	}
}

