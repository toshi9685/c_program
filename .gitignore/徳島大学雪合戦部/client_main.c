/*****************************************************************
  ファイル名	: client_main.c
  機能		: クライアントのメインルーチン
 *****************************************************************/

#include"client_func.h"

static int NetworkEvent(void *data);//ネットワークのイベント処理スレッド
static void WindowEvent(int myid);//ラウンド中の操作
static void SelectEvent(int myid);//キャラクタ選択中の操作
static void endevent(int myid);//結果表示中の操作
static void Cheakpos(int myid);//キャラクタ,ターゲットの位置の判定,修正
static int Home(int Flag);//タイトル画面
static void team(void);//チーム分け

wiimote_t wiimote = WIIMOTE_INIT;// Wiiリモコンの状態格納用
STATUS player[MAX_CLIENTS];
OBJECTS object[MAX_OBJECTS];
SDL_Rect prect;//移動前の座標
int selectflag = 0;//キャラクタ選択の操作フラグ決定後１
/**********************************
製作者：澁谷
 ********************************/
int main(int argc,char *argv[])
{
    int		num;
    char	localHostName[]="localhost";
    char	*serverName;
    int		clientID;
    int		endFlag = 1;
    SDL_Thread	*thr,*Dthread,*Cthread;
    u_short port = DEFAULT_PORT;
    char server_name[MAX_LEN_NAME];
    // Wiiリモコンを用いるための構造体を宣言（初期化）
    wiimote_report_t report = WIIMOTE_REPORT_INIT;// レポートタイプ用
    /*引数チェック*/
    switch(argc) {
    case 3:
        sprintf(server_name, "%s", argv[1]);
        break;
    default:
        fprintf(stderr, "Usage: %s [server name] [wiimote ID]\n", argv[0]);
        return 1;
    }


    // ***** Wiiリモコン処理 *****
    // Wiiリモコンの接続（１つのみ）
    if (wiimote_connect(&wiimote, argv[2]) < 0) {	// コマンド引数に指定したWiiリモコン識別情報を渡して接続
        printf("unable to open wiimote: %s\n", wiimote_get_error());
        exit(1);
    }
    wiimote.led.one  = 1;	// WiiリモコンのLEDの一番左を点灯させる（接続を知らせるために）
    endFlag=Home(endFlag);//ホーム画面
    /* サーバーとの接続 */
    if(SetUpClient(server_name,&clientID,&num,port)==-1){
        fprintf(stderr,"setup failed : SetUpClient\n");
        return -1;
    }
    team();
    if(SelectWindows(clientID)==-1){
        fprintf(stderr,"setup failed : InitWindows\n");
        return -1;
    }
    /* ネットワークイベント処理スレッドの作成 */
    thr = SDL_CreateThread(NetworkEvent, &endFlag);
    if(thr == NULL){
        printf("\nSDL_CreateThread failure: %s\n", SDL_GetError());
        exit(-1);
    }
    Dthread = SDL_CreateThread(thread_Draw, &endFlag);
    if(Dthread == NULL){
        printf("\nSDL_CreateThread failure: %s\n", SDL_GetError());
        exit(-1);
    }
    Cthread = SDL_CreateThread(thread_Calc,&endFlag);
    if(Cthread == NULL){
        printf("\nSDL_CreateThread failure: %s\n", SDL_GetError());
        exit(-1);
    }
    Uint32 timer=SDL_GetTicks();
    /* メインイベントループ */
    while(endFlag){
        switch(endFlag){
        case 1:	
            /* ウインドウの初期化 */
			
            if(GameWindows(clientID)==-1){
                fprintf(stderr,"setup failed : GameWindows\n");
                return -1;
            }
			
            while(endFlag==1){
                WindowEvent(clientID);
            }
            //endFlag=2;
            Loadwin();
            break;
        case 2:
            selectflag = 0;
            if(SelectWindows(clientID)==-1){
                fprintf(stderr,"setup failed : InitWindows\n");
                return -1;
            }
            wiimote.mode.acc = 0;
            wiimote.mode.bits = 48;
            while(endFlag == 2){
                SelectEvent(clientID);
            }
		
            Loadwin();
            wiimote.mode.bits = 52;//mode切り替え一回だけ
            wiimote.mode.acc = 1;// 加速度センサをON（センサを受け付ける）
            break;
        case 3:
            while(endFlag == 3){
                endevent(clientID);
            }
            //endFlag=0;
            break;
        default:
            break;		
        }
        //SDL_Delay(300);
    }

    /* 終了処理 */
    SDL_WaitThread(thr,NULL);
    SDL_WaitThread(Cthread, NULL);
    SDL_WaitThread(Dthread, NULL);
    DestroyWindow();
    CloseSoc();
    return 0;
}
/************************************************************
関数名：team
機能：チーム分け
製作者：澁谷
************************************************************/
void team(void){
	if(MAX_CLIENTS/2 == 2){
		player[0].team = 'A';
		player[1].team = 'A';
		player[2].team = 'B';
		player[3].team = 'B';
	}else if(MAX_CLIENTS/2 == 1){
		player[0].team = 'A';
		player[1].team = 'B';
	}else{
		player[0].team = 'A';	
	}

}
/****************************************************************
関数名：Home
機能：ホーム待機画面
引数：int Flag 
出力：Flag :0　ゲーム終了　2 ゲーム開始
製作者：澁谷
****************************************************************/
int Home(int Flag){
	InitWindows();//初期画面
	while(Flag==1){
		if(wiimote_is_open(&wiimote)){
			// Wiiリモコンの状態を取得・更新する
			if (wiimote_update(&wiimote) < 0) {
				wiimote_disconnect(&wiimote);
			}
			// ***** Wiiのキー（ボタン）ごとに処理 *****
			// HOMEボタンが押された時
			if (wiimote.keys.home) {
				wiimote_speaker_free(&wiimote);	// Wiiリモコンのスピーカを解放
				wiimote_disconnect(&wiimote);	// Wiiリモコンとの接続を解除
				DestroyWindow();
				Flag = 0;
			}
			if(wiimote.keys.a){
				Flag = 2;
				Loadwin();
			}
		}
	}
	return Flag;
}
/*****************************************************************
  関数名: WindowEvent
  機能	: ラウンド中のイベント処理を行う
  引数	: int myid: クライアントID
  出力	: なし
　製作者：小松
 *****************************************************************/
void WindowEvent(int myid)
{
	int i,dir = 4;
	double dis;
	SDL_Event event;
	if(SDL_PollEvent(&event)){
		switch(event.type){
			case SDL_QUIT:
				SendEndCommand();
				break;
		}
	}
	if(wiimote_is_open(&wiimote)){
		// Wiiリモコンの状態を取得・更新する
		if (wiimote_update(&wiimote) < 0) {
			wiimote_disconnect(&wiimote);
		}
		// ***** Wiiのキー（ボタン）ごとに処理 *****
		// HOMEボタンが押された時
		if (wiimote.keys.home) {
			wiimote_speaker_free(&wiimote);// Wiiリモコンのスピーカを解放
			wiimote_disconnect(&wiimote);// Wiiリモコンとの接続を解除
			SendEndCommand();
		}
		
		//wiimote.mode.bits = 52;
		//キャラクター操作
		prect.x = player[myid].pos.x;
		prect.y = player[myid].pos.y;
		if(wiimote.keys.left){
			player[myid].pos.x -= 1;
			if(player[myid].ctype == Souka)
				player[myid].pos.x -= 1;
			//anime[myid][0] = (anime[myid][0] + 1) % 4;
		}else if(wiimote.keys.right){
			player[myid].pos.x += 1;
			if(player[myid].ctype == Souka)
				player[myid].pos.x += 1;
			//anime[myid][0] = (anime[myid][0] + 1) % 4;
		}
		if(wiimote.keys.up){
			player[myid].pos.y -= 1;
			//anime[myid][0] = (anime[myid][0] + 1) % 4;
		}else if(wiimote.keys.down){
			player[myid].pos.y += 1;
			//anime[myid][0] = (anime[myid][0] + 1) % 4;
		}
		Cheakpos(myid);
		SendCommand(MOVE,player[myid].pos.x,player[myid].pos.y,myid);
		if(wiimote.ext.nunchuk.joyx > 136){
		  	player[myid].target.x += (wiimote.ext.nunchuk.joyx - 110 )/50;
		  }else if(wiimote.ext.nunchuk.joyx < 116){
		  	player[myid].target.x -= (126 - wiimote.ext.nunchuk.joyx)/50;
		  }
		if(wiimote.ext.nunchuk.joyy > 136){
		  	player[myid].target.y -= (wiimote.ext.nunchuk.joyy - 120 )/50;
		  }else if(wiimote.ext.nunchuk.joyy < 116){
		  	player[myid].target.y += (136 - wiimote.ext.nunchuk.joyy)/50;
		  }
		//投げた判定
		if(player[myid].isThrowing == 0){
			if(wiimote.force.z < 0 && sqrt(wiimote.force.x*wiimote.force.x+wiimote.force.y*wiimote.force.y+wiimote.force.z*wiimote.force.z) > 3){
				if(wiimote.keys.b){//Bボタン押しながら
                                    if(player[myid].ctype != Chino && player[myid].ctype != Souka){
                                        SendCommand(SPECIAL,player[myid].target.x+16,player[myid].target.y+16,myid);
                                    }
                                }else{
					//anime[myid][1] = 1;
					SendCommand(ATTAK,player[myid].target.x+16,player[myid].target.y+16,myid);
				}
			}
		}
	}
}
/*************************************
関数名：SelectEvent
機能：キャラクタ選択中の操作
引数：クライアントID
製作者：澁谷
 ************************************/
void SelectEvent(int myid){
    SDL_Event event;
    if(SDL_PollEvent(&event)){
        switch(event.type){
        case SDL_QUIT:
            SendEndCommand();
            break;
        }
    }
    if(wiimote_is_open(&wiimote)){
        // Wiiリモコンの状態を取得・更新する
        if (wiimote_update(&wiimote) < 0) {
            wiimote_disconnect(&wiimote);
        }
		
        // ***** Wiiのキー（ボタン）ごとに処理 *****
        // HOMEボタンが押された時
        if (wiimote.keys.home) {
            wiimote_speaker_free(&wiimote);	// Wiiリモコンのスピーカを解放
            wiimote_disconnect(&wiimote);	// Wiiリモコンとの接続を解除
            SendEndCommand();
        }
        if(selectflag== 0){
            if(wiimote.keys.left){
                if(player[myid].ctype == 0)
                    player[myid].ctype = 8;
                player[myid].ctype =abs((player[myid].ctype-1))%10;
            }else if(wiimote.keys.right){
                if(player[myid].ctype == 7)
                    player[myid].ctype = 9;
                player[myid].ctype =abs((player[myid].ctype+1))%10;
            }else if(wiimote.keys.up){
                if(player[myid].ctype == 3||player[myid].ctype == 4)
                    player[myid].ctype -= 5;
                player[myid].ctype = abs((player[myid].ctype+5))%10;
            }else if(wiimote.keys.down){
                if(player[myid].ctype == 3||player[myid].ctype == 4)
                    player[myid].ctype -= 5;
                player[myid].ctype = abs((player[myid].ctype+5))%10;
            }else if(wiimote.keys.one){
                selectflag = 1;
                SendCommand(DECISION,player[myid].ctype,0,myid);
            }
            SendCommand(SELECT,player[myid].ctype,0,myid);
        }
        if(wiimote.keys.two){
            selectflag = 0;
            SendCommand(CANCEL,0,0,myid);
        }
    }
}
/**************************************
関数名：endevent
機能：結果表示中の操作
引数：int myid :クライアントID
製作者：小松
 ************************************/
void endevent(int myid){
	SDL_Event event;
	if(SDL_PollEvent(&event)){
		switch(event.type){
			case SDL_QUIT:
				SendEndCommand();
				break;
		}
	}
	if(wiimote_is_open(&wiimote)){
		// Wiiリモコンの状態を取得・更新する
		if (wiimote_update(&wiimote) < 0) {
			wiimote_disconnect(&wiimote);
		}
		wiimote.mode.acc = 0;
		wiimote.mode.bits = 48;
		// ***** Wiiのキー（ボタン）ごとに処理 *****
		// HOMEボタンが押された時
		if (wiimote.keys.home) {
			wiimote_speaker_free(&wiimote);	// Wiiリモコンのスピーカを解放
			wiimote_disconnect(&wiimote);	// Wiiリモコンとの接続を解除
			SendEndCommand();
		}
	}
}
/**********************************
関数名:Cheakpos
機能：キャラクタとターゲットの位置を確認。
画面外に出るとき、もとにもどす。
キャラクタとオブジェクトとの当たり判定。
引数：int myid:クライアントID
製作者：持田
 *********************************/
void Cheakpos(int myid){
    if(player[myid].pos.x <0)
        player[myid].pos.x = 0;
    else if(player[myid].pos.x > WINDOW_WIDTH-player[myid].pos.w)
        player[myid].pos.x = WINDOW_WIDTH-player[myid].pos.w;
    if(player[myid].pos.y <WINDOW_HEIGHT/2)
        player[myid].pos.y = WINDOW_HEIGHT/2;
    else if(player[myid].pos.y > WINDOW_HEIGHT-player[myid].pos.h)
        player[myid].pos.y = WINDOW_HEIGHT-player[myid].pos.h;

    if(player[myid].target.x > WINDOW_WIDTH-player[myid].target.w)
        player[myid].target.x = WINDOW_WIDTH-player[myid].target.w;
    if(player[myid].target.y > WINDOW_HEIGHT-player[myid].target.h)
        player[myid].target.y = WINDOW_HEIGHT-player[myid].target.h;
    int posx = player[myid].pos.x;
    int posy = player[myid].pos.y;
    int posw = player[myid].pos.w;
    int posh = player[myid].pos.h;
    int reset = 0;
    player[myid].bexist = 0;
    SDL_Rect c = {player[myid].pos.x+(player[myid].pos.w/2),player[myid].pos.y+(player[myid].pos.h/2),0,0}; //キャラの中心
    int i,j;
    for(i = 0;i<MAX_CLIENTS;i++){
        if(object[i].flag==1){
            if(object[i].otype == Denki){
                if(player[myid].team != player[i].team){
                    if(object[i].pos.x < posx && posx < (object[i].pos.x + object[i].pos.w)){//キャラの左側
                        if(object[i].pos.y < posy && posy < (object[i].pos.y + object[i].pos.h)){//キャラの上側
                            reset = 1;
                        }
                        else  if(posy < object[i].pos.y && object[i].pos.y < (posy + posh)){//キャラの下側
                            reset = 1;
                        }
                    }
                    else if(object[i].pos.x < (posx + posw) && (posx + posw) < (object[i].pos.x + object[i].pos.w)){//キャラの右側
                        if(object[i].pos.y < posy && posy < (object[i].pos.y + object[i].pos.h)){//キャラの上側
                            reset = 1;
                        }
                        else  if(posy < object[i].pos.y && object[i].pos.y < (posy + posh)){//キャラの下側
                            reset = 1;
                        } 
                    }
                    else if(object[i].pos.x < posx+(posw/2) && posx+(posw/2) < (object[i].pos.x + object[i].pos.w)){
                        if(object[i].pos.y < posy && posy < (object[i].pos.y + object[i].pos.h)){//キャラの上側
                            reset = 1;
                        }
                        else  if(posy < object[i].pos.y && object[i].pos.y < (posy + posh)){//キャラの下側
                            reset = 1;
                        } 
                    }

                    if(reset == 1){
                        player[myid].pos.x = prect.x;
                        player[myid].pos.y = prect.y;
                    }
                }
            }
            if(object[i].otype == Kagaku){
                if(player[myid].team != player[i].team){
                    if(object[i].pos.x < posx && posx < (object[i].pos.x + object[i].pos.w)){//キャラの左側
                        if(object[i].pos.y < posy && posy < (object[i].pos.y + object[i].pos.h)){//キャラの上側
                            reset = 1;
                        }
                        else  if(posy < object[i].pos.y && object[i].pos.y < (posy + posh)){//キャラの下側
                            reset = 1;
                        }
                    }
                    else if(object[i].pos.x < (posx + posw) && (posx + posw) < (object[i].pos.x + object[i].pos.w)){//キャラの右側
                        if(object[i].pos.y < posy && posy < (object[i].pos.y + object[i].pos.h)){//キャラの上側
                            reset = 1;
                        }
                        else  if(posy < object[i].pos.y && object[i].pos.y < (posy + posh)){//キャラの下側
                            reset = 1;
                        } 
                    }
                    else if(object[i].pos.x < posx+(posw/2) && posx+(posw/2) < (object[i].pos.x + object[i].pos.w)){
                        if(object[i].pos.y < posy && posy < (object[i].pos.y + object[i].pos.h)){//キャラの上側
                            reset = 1;
                        }
                        else  if(posy < object[i].pos.y && object[i].pos.y < (posy + posh)){//キャラの下側
                            reset = 1;
                        } 
                    }

                    if(reset == 1){
                        player[myid].pos.x = prect.x;
                        player[myid].pos.y = prect.y;
                    }
                }
            }
            if(object[i].otype == Kensetsu){
                if(player[myid].team == player[i].team){
                    if(object[i].pos.x <= c.x && c.x <= object[i].pos.x +object[i].pos.w){
                        if(object[i].pos.y <= c.y && c.y <= object[i].pos.y + object[i].pos.h){
                            player[myid].bexist = 1;
				
                        }
                    }
                    if(object[i].pos.x < posx && posx < (object[i].pos.x + object[i].pos.w)){//キャラの左側
                        if(object[i].pos.y < posy && posy < (object[i].pos.y + object[i].pos.h/2)){//キャラの上側
                            reset = 1;
                        }
                        else  if(posy < object[i].pos.y && object[i].pos.y < (posy + posh)){//キャラの下側
                            reset = 1;
                        }
                    }
                    else if(object[i].pos.x < (posx + posw) && (posx + posw) < (object[i].pos.x + object[i].pos.w)){//キャラの右側
                        if(object[i].pos.y < posy+50 && posy < (object[i].pos.y + object[i].pos.h/2)){//キャラの上側
                            reset = 1;
                        }
                        else  if(posy < object[i].pos.y && object[i].pos.y < (posy + posh)){//キャラの下側
                            reset = 1;
                        } 
                    }
                    else if(object[i].pos.x < posx+(posw/2) && posx+(posw/2) < (object[i].pos.x + object[i].pos.w)){
                        if(object[i].pos.y < posy+50 && posy < (object[i].pos.y + object[i].pos.h/2)){//キャラの上側
                            reset = 1;
                        }
                        else  if(posy < object[i].pos.y && object[i].pos.y < (posy + posh)){//キャラの下側
                            reset = 1;
                        } 
                    }

                    if(reset == 1){
                        player[myid].pos.x = prect.x;
                        player[myid].pos.y = prect.y;
                    }
                }
            }
            if(object[i].otype == Seibutsu){
                if(player[myid].team == player[i].team){
                    if(object[i].pos.x <= c.x && c.x <= object[i].pos.x +object[i].pos.w){
                        if(object[i].pos.y <= c.y && c.y <= object[i].pos.y + object[i].pos.h){
                            player[myid].bexist = 1;
				
                        }
                    }
                    if(object[i].pos.x < posx && posx < (object[i].pos.x + object[i].pos.w)){//キャラの左側
                        if(object[i].pos.y < posy && posy < (object[i].pos.y + object[i].pos.h/2)){//キャラの上側
                            reset = 1;
                        }
                        else  if(posy < object[i].pos.y && object[i].pos.y < (posy + posh)){//キャラの下側
                            reset = 1;
                        }
                    }
                    else if(object[i].pos.x < (posx + posw) && (posx + posw) < (object[i].pos.x + object[i].pos.w)){//キャラの右側
                        if(object[i].pos.y < posy && posy < (object[i].pos.y + object[i].pos.h/2)){//キャラの上側
                            reset = 1;
                        }
                        else  if(posy < object[i].pos.y && object[i].pos.y < (posy + posh)){//キャラの下側
                            reset = 1;
                        } 
                    }
                    else if(object[i].pos.x < posx+(posw/2) && posx+(posw/2) < (object[i].pos.x + object[i].pos.w)){
                        if(object[i].pos.y < posy && posy < (object[i].pos.y + object[i].pos.h/2)){//キャラの上側
                            reset = 1;
                        }
                        else  if(posy < object[i].pos.y && object[i].pos.y < (posy + posh)){//キャラの下側
                            reset = 1;
                        } 
                    }

                    if(reset == 1){
                        player[myid].pos.x = prect.x;
                        player[myid].pos.y = prect.y;
                    }
                }
            }
        }
    }
        SendCommand(STATAS,player[myid].bexist,0,myid);
}
/*****************************************************************
  関数名  : NetworkEvent
  機能    : ネットワークのイベントループ
  引数    : void		*data		: 終了判定フラグ
  出力    : スレッド終了時に０を返す
 製作者：澁谷
 *****************************************************************/
static int NetworkEvent(void *data)
{	int *endFlag;
	endFlag = (int*)data;
	while(*endFlag){
		*endFlag = SendRecvManager(*endFlag);
	}
	return 0;
}
