/*****************************************************************
  ファイル名	: client_win.c
  機能		: クライアントのユーザーインターフェース処理
 *****************************************************************/

#include"client_func.h"
#define BALL_WIDTH 24
static void sizeofball(void);//雪玉のサイズを変える
static void calcBall(double* angle,int i);//雪玉の軌道を計算する
static void drawBall(int id);//雪玉の表示
static void selectwin(void);//キャラクタ選択画面の描画
static void timer(void);//タイマーの描画
static void resultwin(void);//結果表示画面の描画
static void Initobjects(void);//オブジェクトの初期化
static SDL_Surface *window,*image[10],*tar,*background,*string,*score,*obje;
static Mix_Music *music;
TTF_Font* font;
SDL_Color black = {0x00, 0x00, 0x00};
static SDL_Rect back,curreticle[MAX_CLIENTS],curplayer[MAX_CLIENTS];
int myid;
int initflag = 0;
char chimage[8][25] = {"images/Chino.png","images/Denki.png","images/Kikai.png","images/Kagaku.png","images/Kensetsu.png","images/Seibutsu.png","images/Hikari.png","images/Souka.png"};
char fontname[25] = {"images/ipaexg.ttf"};
STATUS player[MAX_CLIENTS];
Ball ball[MAX_CLIENTS];
int  anime[4][2] = {}; //[4][0]:ｘ座標のコマ数 [4][1]:ｙ座標のコマ数
/*****************************************************************
  関数名: InitWindows
  機能	: メインウインドウの表示，設定を行う
  出力	: 正常なら0
製作者：澁谷
 *****************************************************************/
int InitWindows(){
	SDL_Rect pre={0,0,WINDOW_WIDTH,WINDOW_HEIGHT};
	/* SDLの初期化 */
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0) {
		printf("failed to initialize SDL.\n");
		return -1;
	}
	//サウンドの初期化
	if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
		printf("failed to initialize SDL_mixer.\n");
		exit(-1);
	}
	/* メインのウインドウを作成する */
	if((window = SDL_SetVideoMode(WINDOW_WIDTH,WINDOW_HEIGHT, 32, SDL_SWSURFACE)) == NULL) {
		printf("failed to initialize videomode.\n");
		return -1;
	}
	//BGMの読み込み
	if((music = Mix_LoadMUS("music/music.ogg")) == NULL){
		printf("failed to load music.\n");
		exit(-1);
	}
	background = IMG_Load("images/start.png");
	SDL_BlitSurface(background,NULL,window,&pre);
	SDL_Flip(window);
	return 0;
}
/***********************************************************
  関数名：SelectWindows
  機能：　選択画面の初期化
　製作者：澁谷
 ***********************************************************/
int SelectWindows(int clientID){
    char *s,title[10];
    int i = 0;
    myid = clientID;
    printf("myid=%d\n",myid);
    SDL_Surface *select;
    TTF_Init();
    font = TTF_OpenFont(fontname, 40);
    select = IMG_Load("images/s.png");
    background = IMG_Load("images/selectwin.png");
    char str[]="チーム";
    /* ウインドウのタイトルをセット */
    sprintf(title,"%c%s",player[clientID].team,str);
    SDL_WM_SetCaption(title,NULL);

    string = TTF_RenderUTF8_Blended(font, "キャラクターを選択してください。", black);
    SDL_Rect moji = {0,0,string->w,string->h};
    SDL_BlitSurface(string, &moji, window, &moji);
    SDL_Rect pre1={0,0,WINDOW_WIDTH,WINDOW_HEIGHT};
    SDL_BlitSurface(background,NULL,window,&pre1);
    SDL_Rect pre = {0,0,100,100};
    SDL_Rect pre2 = {0,0,100,100};
    for(;i<8;i++){
        s = chimage[i];
        image[i] = IMG_Load(s);
        if(i <= 4){
            pre.x = 150+100*i;
            pre.y = 100;
        }else{
            pre.x = 150+100*(i-5);
            pre.y = 200;
        }
        SDL_BlitSurface(image[i],&pre2,window,&pre);
    }
    pre.x = 150;
    pre.y = 100;
    SDL_BlitSurface(select,NULL,window,&pre);
    SDL_Flip(window);
    int j;
    for(j=0;j<MAX_CLIENTS;j++)
        player[j].ctype=0;
    return 0;
}
/********************************************************
  関数名：GameWindows
  機能：　ゲーム画面、キャラクター画像の初期化
　製作者：澁谷
 ********************************************************/
int GameWindows(int clientID)
{
	int i;

	/* 引き数チェック */
	assert(0<num && num<=MAX_CLIENTS);
	//import
	tar = IMG_Load("images/reticule.png");
	background = IMG_Load("images/stage_white.png");
	// 初期画面
	back.x = 0;
	back.y = 0;
	back.w = WINDOW_WIDTH;
	back.h = WINDOW_HEIGHT;
	if(clientID%2 == 0){
		player[clientID].pos.x = 300;
		player[clientID].pos.y = 450;
		player[clientID].target.x = 300;
		player[clientID].target.y = 100;
	}else{
		player[clientID].pos.x = 600;
		player[clientID].pos.y = 450;
		player[clientID].target.x = 600;
		player[clientID].target.y = 100;
	}
	int ci;	
	SDL_Rect src = {0,0,100,100};
	for(ci = 0;ci<MAX_CLIENTS;ci++){
		player[ci].isThrowing=0;
		player[ci].target.w = 32;
		player[ci].target.h = 32;
		player[ci].pos.w = 100;
		player[ci].pos.h = 100;
	}
	SDL_BlitSurface(background,NULL,window,&back);
	SDL_BlitSurface(tar, NULL, window, &(player[clientID].target));
	SDL_BlitSurface(image[clientID], &src, window, &(player[clientID].pos));
	SDL_Flip(window);
	SendCommand(MOVE,player[clientID].pos.x,player[clientID].pos.y,clientID);
        initflag = 1;
	return 0;
}
/**************************************************
関数名：Initobjects
機能：オブジェクトの初期化。
毎ラウンド呼び出される。
製作者：澁谷
**************************************************/
void Initobjects(){
	int i;	
	for(i=0;i<MAX_CLIENTS;i++){
		ball[i].x = 0;
		ball[i].y = 0;
		ball[i].w = 0;	
	}
	for(i=0;i<MAX_OBJECTS;i++){
		object[i].flag=0;
		object[i].otype=-1;
		object[i].pos.x = 0;
		object[i].pos.y = 0;
		object[i].pos.w = 0;
		object[i].pos.h = 0;
	}
	obje=IMG_Load("images/Objects.png");
}
/******************************************************
  関数名：Loadwin
  機能：　画面遷移の間に入れる
　製作者：澁谷
 ******************************************************/
void Loadwin(void){
	SDL_Rect a = {0,0,WINDOW_WIDTH,WINDOW_HEIGHT};
	background = IMG_Load("images/start1.png");
	SDL_BlitSurface(background,NULL,window,&a);
	SDL_Flip(window);
}
/*****************************************************************
  関数名	: DestroyWindow
  機能	: SDLを終了する
  引数	: なし
  出力	: なし
　製作者：澁谷
 *****************************************************************/
void DestroyWindow()
{
	TTF_CloseFont(font);
	Mix_FreeMusic(music); // BGMの解放
	TTF_Quit();
	SDL_Quit();
}
/*********************************************
関数名：ctypein
  機能	:キャラタイプの読み込み
　引数：int ctype ：キャラクタの種類
　　　　int id　　：クライアントID
　製作者：澁谷
 *********************************************/
void ctypein(int ctype,int id){
	player[id].ctype = ctype;
	printf("id=%d ctype= %d\n",id,player[id].ctype);
}
/****************************************
関数名：timerset
機能：サーバーから送られてきたタイマーの内容を文字としてセット
引数：int t:残り時間
      int r:現在ラウンド
製作者：澁谷
 ***************************************/
void timerset(int t,int r){
	font = TTF_OpenFont(fontname, 24);
	char str[30];
	char str1[] ="残り時間:";
	char str2[] ="現在ラウンド:";
	sprintf(str,"%s%d  %s%d",str1,t,str2,r);
	string = TTF_RenderUTF8_Blended(font,str, black);
}
/*******************************************
関数名：scoreset
機能：サーバーから送られてきたポイント状況を文字としてセット
引数：int i:Aチームのスコア
　　　int j:Bチームのスコア
製作者：澁谷
 *****************************************/
void scoreset(int i,int j){
	font = TTF_OpenFont(fontname, 24);
	char str[30];
	char str1[] ="Aチーム:";
	char str2[] =":Bチーム";
	sprintf(str,"%s%d vs %d%s",str1,i,j,str2);
	score = TTF_RenderUTF8_Blended(font,str, black);

}
/*********************************************
関数名：result
機能：サーバーから送られてきたゲーム終了時の結果をセットする
引数：int i:Aチームのラウンド取得数
　　　int j:Bチームのラウンド取得数
製作者：澁谷
 ********************************************/
void result(int i,int j){
	font = TTF_OpenFont(fontname, 60);
	char str[30];
	if(i>j){
		char stra[]="Aチームの勝ちです!";
		string = TTF_RenderUTF8_Blended(font,stra, black);
		sprintf(str,"%d   vs   %d",i,j);
	}else if(i<j){
		char strb[]="Bチームの勝ちです!";
		string = TTF_RenderUTF8_Blended(font,strb, black);
		sprintf(str,"%d   vs   %d",i,j);
	}else{
		char strd[]="引き分けです!";
		string = TTF_RenderUTF8_Blended(font,strd, black);
		sprintf(str,"%d   vs   %d",i,j);
	}
	score = TTF_RenderUTF8_Blended(font,str, black);
	
	SDL_Rect a = {0,0,800,600};
	SDL_Surface *result;
	result = IMG_Load("images/result.png");
	SDL_BlitSurface(result,NULL,window,&a);
	SDL_Flip(window);

}

/************************************************
関数名：resultwin
機能：結果を画面に表示
製作者：澁谷
 ***********************************************/
void resultwin(void){
	SDL_Rect a = {0,0,800,600};
	SDL_Surface *result;
	result = IMG_Load("images/result.png");
	SDL_BlitSurface(result,NULL,window,&a);
	SDL_Rect moji ={(400-((score->w)/2)),300,score->w,score->h};
	SDL_BlitSurface(score,NULL,window,&moji);
	SDL_Rect moji2 ={(400-((string->w)/2)),400,string->w,string->h};
	SDL_BlitSurface(string,NULL,window,&moji2);
	SDL_Flip(window);
	SDL_FreeSurface(result);
	

}
/**********************************************
  関数名：timer
  機能：時間の描画
　製作者：澁谷
 **********************************************/
void timer(){
	SDL_Rect moji ={0,0,string->w,string->h};
	SDL_BlitSurface(string,&moji,window,&moji);
	SDL_Rect s ={400,0,score->w,score->h};
	SDL_BlitSurface(score,NULL,window,&s);
}
/********************************************************
  関数名:Repos
  機能	:座標変換
  引数	:int id キャラクタid
  int x キャラクタのx座標
  int y キャラクタのy座標
  返り値:なし
　製作者：澁谷
 ********************************************************/
void Repos(int id,int x,int y){
		if(player[id].team == player[myid].team){	
			player[id].pos.x = x;
			player[id].pos.y = y;
		}else{
			player[id].pos.x = WINDOW_WIDTH - player[myid].pos.w - x;
			player[id].pos.y = WINDOW_HEIGHT - player[myid].pos.h - y;
		}
}
/**************************************
  関数名：targetimport
  機能：雪玉を投げる目標
　引数：int id:クライアントID
       int x:ターゲットのx座標
　　　　int y:ターゲットのy座標
　製作者：澁谷
 **************************************/
void targetimport(int id,int x,int y){
	if(player[id].team == player[myid].team){
		curreticle[id].x = x;
		curreticle[id].y = y;
	}else{
		curreticle[id].x = WINDOW_WIDTH - x;
		curreticle[id].y = WINDOW_HEIGHT - y;
	}
	player[id].isThrowing = 1;


}
/*****************************************
  関数名:initobject
機能：オブジェクトが設置された時の代入
引数：int id:クライアントID
      int i:ターゲットのx座標
　　　int j:ターゲットのy座標
製作者：澁谷
 *****************************************/
void initobject(int id,int i,int j){
	
	if(player[id].team == player[myid].team){
		object[id].pos.x = i-50;
		object[id].pos.y = j-30;
	}else{
		object[id].pos.x = WINDOW_WIDTH - player[id].target.w - i-34;
		object[id].pos.y = WINDOW_HEIGHT - player[id].target.h - j;
	}
	object[id].otype = player[id].ctype;
	object[id].flag = 1;
	object[id].pos.w = 100;
	object[id].pos.h = 60;	
}
/************************************************
関数名：resetobject
機能：オブジェクトのフラグを0に戻す
引数：int id:クライアントID
製作者：澁谷
************************************************/
void resetobject(int id){
	object[id].flag = 0;
}
/*************************************************************
関数名：sizeofball
機能：雪玉のサイズを変更する
製作者：天羽
 *************************************************************/
void sizeofball(void)
{
	int i;
	int j;
	int dist = 12;
	for(j = 0;j<MAX_CLIENTS;j++){
		for(i = 0;i < dist;i++){
			if(ball[j].y > (double)((WINDOW_HEIGHT / dist) * (dist-i)) && ball[j].y < (double)((WINDOW_HEIGHT / dist) * (dist-(i-1)))){
				ball[j].w = BALL_WIDTH - 2*i;
			}
		}
	}
}
/*******************************************************
関数名：calcBall
機能：雪玉の軌道を計算する
引数：int i:クライアントID
製作者：天羽
 *******************************************************/
void calcBall(double* angle,int i)
{
    int yspeed;
    if(player[i].team == player[myid].team){
        if(ball[i].y > curreticle[i].y){
            sizeofball();
            ball[i].y -= 8;
            yspeed = 8;
            if(player[i].ctype==Kikai)
                yspeed = 10;
            if(curplayer[i].x < curreticle[i].x){
                ball[i].x += (double)(yspeed*(curreticle[i].x-curplayer[i].x))/(double)(curplayer[i].y-curreticle[i].y);
            }else{
                ball[i].x += (double)(yspeed*(curplayer[i].x-curreticle[i].x))/(double)(curreticle[i].y-curplayer[i].y);
            }
        }else{
            SendCommand(HIT,curreticle[i].x,curreticle[i].y,i);
            player[i].isThrowing = 0;
        }

    }else{
		
        if(ball[i].y < curreticle[i].y){
            sizeofball();
            ball[i].y += 8;
            yspeed = 8;
            if(player[i].ctype==Kikai)
                ball[i].y += 10;
            yspeed = 10;
            if(curplayer[i].x < curreticle[i].x){
                ball[i].x += (double)(yspeed*(curplayer[i].x-curreticle[i].x))/(double)(curplayer[i].y-curreticle[i].y);
            }else{
                ball[i].x += (double)(yspeed*(curplayer[i].x-curreticle[i].x))/(double)(curplayer[i].y-curreticle[i].y);
            }
        }else{
            player[i].isThrowing = 0;
        }
    }
}

/*************************************************************
  関数名:thread_Clc
  機能:雪玉の軌道計算
製作者：天羽
 *************************************************************/
int thread_Calc(void *args){
	int *endFlag;
	double angle[MAX_CLIENTS];
	endFlag = (int*)args;
	int i;
	Uint32 timer_calc = SDL_GetTicks();
	while(*endFlag){
		switch(*endFlag){
			case 1:
				if(SDL_GetTicks() >= timer_calc){
					for(i = 0;i<MAX_CLIENTS;i++){
						if(player[i].isThrowing != 0){
							calcBall(angle,i);
						}else{
							
							curplayer[i] = player[i].pos;
							ball[i].x = player[i].pos.x;
							ball[i].y = player[i].pos.y;
							ball[i].w = BALL_WIDTH;
							if(player[i].team != player[myid].team)
								ball[i].w = 0;
						}
					}
					timer_calc += 30;
				}
				break;
			case 2:

				break;
		}
	}
	return 0;
}
/********************************************
関数名：drawBall
機能：雪玉の描画
製作者：天羽
 *******************************************/
void drawBall(int id)
{
	filledCircleColor(window, ball[id].x/* + (double)(player[id].pos.w/2)*/, ball[id].y, ball[id].w, 0x88888888);
}
/************************************************
関数名：selectwin
機能：キャラクタ選択画面の描画
製作者：澁谷
 ***********************************************/
void selectwin(){
	int i,j;
	SDL_Surface *b;
	SDL_Surface *back;
	back = IMG_Load("images/selectwin.png");
	b = IMG_Load("images/s.png");
	SDL_Rect a={0,0,100,100};
	SDL_Rect s={0,0,100,100};
	for(i=0;i<8;i++){
		if(i <= 4){
			a.x = 150+100*i;
			a.y = 100;
		}else{
			a.x = 150+100*(i-5);
			a.y = 200;
		}
		SDL_BlitSurface(back,&s,window,&a);
		SDL_BlitSurface(image[i],&s,window,&a);
		if(i == player[myid].ctype){
			SDL_BlitSurface(b,NULL,window,&a);
		}
	}
	if(MAX_CLIENTS/2 <=1){
		for(i=0;i<MAX_CLIENTS;i++){

			a.x = 150+400*i;
			a.y = 350;
			SDL_BlitSurface(back,&s,window,&a);
			SDL_BlitSurface(image[player[i].ctype],&s,window,&a);
		}
	}else{
		for(i=0;i<MAX_CLIENTS;i++){
			a.x = 150+100*i;
			a.y = 350;
			if(i>=2)
				a.x += 100;
			SDL_BlitSurface(back,&s,window,&a);
			SDL_BlitSurface(image[player[i].ctype],&s,window,&a);
		}
	}
	SDL_Rect moji = {0,0,string->w,string->h};
	SDL_BlitSurface(string, &moji, window, &moji);
	SDL_Flip(window);
	SDL_FreeSurface(back);
	SDL_FreeSurface(b);
}
/************************************************************
  関数名:thread_Draw
  機能:描画
　製作者：澁谷
 ************************************************************/
int thread_Draw(void *args){
    int i,j;
    int *endFlag;
    endFlag = (int*)args;
    Uint32 next_frame=SDL_GetTicks();
    // *****描画処理*****
    while(*endFlag){
        switch(*endFlag){
        case 1:
            SDL_Delay(1000);
            Mix_PlayMusic(music, -1); // BGMの再生（繰り返し再生）
            Initobjects();
            while(*endFlag==1 && initflag==1){

                if (SDL_GetTicks() >= next_frame){
                    SDL_BlitSurface(background,NULL,window,&back);
                    SDL_Rect src[MAX_CLIENTS] = {};
                        for(i = 0;i<MAX_CLIENTS;i++){
                            src[i].w = 100;
                            src[i].h = 100;
                            if(player[i].isThrowing == 1){
                                drawBall(i);
                            }					
                            src[i].y = 0;
                            if(player[i].ctype == Hikari&&object[i].flag == 1){
                                src[i].x = src[i].w * player[i].ctype;
                                SDL_BlitSurface(obje,&src[i],window,&(player[i].pos));
                            }else{
                                src[i].x=0;
                                SDL_BlitSurface(image[player[i].ctype], &src[i], window, &(player[i].pos));
                            }
                        }
                    for(j = 0;j<MAX_OBJECTS;j++){
                        if(object[j].flag== 1){
                            src[j].y=40;
                            if(player[j].ctype ==Denki ){
                                src[j].x = src[j].w * player[j].ctype;
                            }else if(player[j].ctype ==Kagaku){
                                src[j].x = src[j].w * player[j].ctype;								
                            }else if(player[j].ctype ==Kensetsu){
                                src[j].y=0;
                                src[j].x = src[j].w * player[j].ctype;								
                            }else if(player[j].ctype == Seibutsu){
                                object[j].pos.h = 100;
                                src[j].x = src[j].w * player[j].ctype;
                                src[j].y=0;								
                            }

                            SDL_BlitSurface(obje,&src[j],window,&(object[j].pos));
                        }
                    }
                    timer();
                    SDL_BlitSurface(tar,NULL,window,&(player[myid].target));
                    SDL_Flip(window);// 画面表示（反映）
                    next_frame += 30;
                }
            }
            initflag =0;
            Mix_HaltMusic();      // BGMの停止
            break;
        case 2:
            SDL_Delay(1000);
            while(*endFlag==2){
                selectwin();
            }
            break;
        case 3:
            SDL_Delay(1000);
            resultwin();
            break;
        }
    }
    return 0;
}
