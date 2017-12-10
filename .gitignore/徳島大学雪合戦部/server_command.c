/*****************************************************************
  ファイル名	: server_command.c
  機能		: サーバーのコマンド処理
  製作者：澁谷
 *****************************************************************/

#include"server_common.h"
#include"server_func.h"

static int Collisiondetection(int id,int x,int y);//当たり判定
CONTAINER maincharadata[MAX_CLIENTS];
GAMEINFO info;
/*********************************
関数名：Initchar
機能：毎ラウンドキャラクタ構造体を初期化
 *********************************/
void Initchar(void){
    int i;
    for(i=0;i<MAX_CLIENTS;i++){
        maincharadata[i].pos.w = 100;
        maincharadata[i].pos.h = 100;
        maincharadata[i].tar.w = 32;
        maincharadata[i].tar.h = 32;
        maincharadata[i].hit = 0;
        maincharadata[i].thr = 0;
        maincharadata[i].ctype = 10;
        maincharadata[i].skillf =0;
        maincharadata[i].bexist=0;
        info.skilltimer[i] = 0;
    }
    info.timer = 60;
}
/*****************************************************************
  関数名	: ExecuteCommand1
  機能	: クライアントから送られてきたコマンドを元に，
	  引き数を受信し，実行する
	  ゲーム中のコマンド
 *****************************************************************/
int ExecuteCommand1(COMMAND data,int id)
{
    int		endFlag = 1;
    int i,j;
    /* 引き数チェック */
    assert(0<=id && id<MAX_CLIENTS);
    switch(data.command){
    case ATTAK:
        if(maincharadata[data.k].thr == 0){
            maincharadata[data.k].thr = 1;
            SendData(ALL_CLIENTS,&data,sizeof(COMMAND));
        }
        break;
    case SPECIAL:
        maincharadata[data.k].tar.y = data.j;
        maincharadata[data.k].tar.x = data.i;
        if(maincharadata[data.k].ctype == Kensetsu||maincharadata[data.k].ctype == Seibutsu){
            if(data.j>=350&&info.skilltimer[data.k] ==0){
                maincharadata[data.k].skillf = 1;
                info.skilltimer[data.k] = 10;
                SendData(ALL_CLIENTS,&data,sizeof(COMMAND));
            }				
        }else if(maincharadata[data.k].ctype == Hikari){
            if(info.skilltimer[data.k] == 0){
                maincharadata[data.k].skillf = 1;
                info.skilltimer[data.k] = 3;
                SendData(ALL_CLIENTS,&data,sizeof(COMMAND));
            }
        }else{
            if(data.j<=300&&maincharadata[data.k].thr == 0 && info.skilltimer[data.k] ==0){
                maincharadata[data.k].skillf = 1;
                maincharadata[data.k].thr = 1;
                data.command = ATTAK;
                SendData(ALL_CLIENTS,&data,sizeof(COMMAND));
            }
        }
        break;
    case MOVE:
        maincharadata[data.k].pos.y = data.j;
        maincharadata[data.k].pos.x = data.i;
        /* 全ユーザーに送る */
        SendData(id,&data,sizeof(COMMAND));
        break;
    case HIT:
        if(maincharadata[data.k].thr == 1){
            Collisiondetection(data.k,data.i,data.j);
            maincharadata[data.k].thr = 0;
            printf("maincharadata.hit[%d] = %d\n",data.k,maincharadata[data.k].hit);
        }
        break;
    case STATAS:
        maincharadata[data.k].bexist = data.i;
        break;
    case END_COMMAND:
        endFlag = 0;
        data.command = END_COMMAND;
        SendData(ALL_CLIENTS,&data,sizeof(CONTAINER));
        break;
    default:
        fprintf(stderr,"EX1 0x%02x is not command!\n",data.command);
        break;
    }
    return endFlag;
}
/*****************************************************************
  関数名	: ExecuteCommand2
  機能	: クライアントから送られてきたコマンドを元に，
	  引き数を受信し，実行する
	  選択中のコマンド
 *****************************************************************/
int ExecuteCommand2(COMMAND data,int id)
{
    int		endFlag = 2;
    int i,j;
    /* 引き数チェック */
    assert(0<=id && id<MAX_CLIENTS);
    switch(data.command){
    case SELECT:
        SendData(id,&data,sizeof(COMMAND));
        break;
    case DECISION:
        maincharadata[data.k].ctype = data.i;
        int j,count = 0;
        for(j=0;j<MAX_CLIENTS;j++){
            if(maincharadata[j].ctype != 10)
                count++;
        }
        if(count == MAX_CLIENTS){
            endFlag = 1;
            SendData(ALL_CLIENTS,&data,sizeof(COMMAND));
            data.command = INFO;
            data.i = info.timer;
            info.round += 1;
            data.j = info.round;
            SendData(ALL_CLIENTS,&data,sizeof(COMMAND));
            data.command = SCORE;
            data.i = 0;
            data.j = 0;
            SendData(ALL_CLIENTS,&data,sizeof(COMMAND));				
        }
        break;
    case CANCEL:
        maincharadata[data.k].ctype = 10;
        break;
    case END_COMMAND:
        endFlag = 0;
        data.command = END_COMMAND;
        SendData(ALL_CLIENTS,&data,sizeof(CONTAINER));
        break;
    default:
        fprintf(stderr,"EX2 0x%02x is not command!\n",data.command);
        break;
    }
    return endFlag;
}
/********************************************
関数名：ExcuteCommand3
機能：結果表示中のコマンド処理
 *******************************************/
int ExecuteCommand3(COMMAND data,int id){
    int		endFlag = 3;
    switch(data.command){
    case END_COMMAND:
        endFlag = 0;
        data.command = END_COMMAND;
        SendData(ALL_CLIENTS,&data,sizeof(CONTAINER));
        break;
    default:
        fprintf(stderr,"EX2 0x%02x is not command!\n",data.command);
        break;
    }
    return endFlag;
}
/*************************************************************
  関数名:Collisiondetection
  機能	: 雪玉があたったかどうかを判定.
  引数	: int id
  int x
  int y
  返り値:正常に終了０
 *************************************************************/
int Collisiondetection(int id,int x,int y){
    int i;

    if(id>=MAX_CLIENTS/2){
        for(i = 0;i<MAX_CLIENTS/2;i++){
            if(WINDOW_WIDTH-maincharadata[i].pos.w-maincharadata[i].pos.x < x && x < WINDOW_WIDTH-maincharadata[i].pos.x){
                if(WINDOW_HEIGHT-maincharadata[i].pos.h-maincharadata[i].pos.y < y && y < WINDOW_HEIGHT-maincharadata[i].pos.y){
                    if(maincharadata[i].bexist == 0){
                        maincharadata[id].hit += 1;
                        if(maincharadata[id].ctype == Souka)
                            maincharadata[id].hit += 1;
                        if(maincharadata[id].ctype == Chino && maincharadata[i].ctype == Souka)
                            maincharadata[id].hit +=3;
                        if(maincharadata[id].ctype == Denki){
                            if(maincharadata[id].skillf == 1){
                                info.skilltimer[id] = 5;
                                COMMAND data={SPECIAL,maincharadata[id].tar.x,maincharadata[id].tar.y,id};
                                SendData(ALL_CLIENTS,&data,sizeof(COMMAND));				
                            }	
                        }
                    }
                }else{
                    if(maincharadata[id].ctype == Kagaku){
                        if(maincharadata[id].skillf == 1){
                            info.skilltimer[id] = 5;
                            COMMAND data={SPECIAL,maincharadata[id].tar.x,maincharadata[id].tar.y,id};
                            SendData(ALL_CLIENTS,&data,sizeof(COMMAND));				
                        }	
                    } 
                }
            }else{
                if(maincharadata[id].ctype == Kagaku){
                    if(maincharadata[id].skillf == 1){
                        info.skilltimer[id] = 5;
                        COMMAND data={SPECIAL,maincharadata[id].tar.x,maincharadata[id].tar.y,id};
                        SendData(ALL_CLIENTS,&data,sizeof(COMMAND));				
                    }	
                }
            }
        }
    }else{
        for(i = MAX_CLIENTS/2;i<MAX_CLIENTS;i++){
            printf("maincharadata[%d].bexist=%d\n",i,maincharadata[i].bexist);
            if(WINDOW_WIDTH-maincharadata[i].pos.w-maincharadata[i].pos.x < x && x < WINDOW_WIDTH-maincharadata[i].pos.x){
                if(WINDOW_HEIGHT-maincharadata[i].pos.h-maincharadata[i].pos.y < y && y < WINDOW_HEIGHT-maincharadata[i].pos.y){
                    if(maincharadata[i].bexist == 0){
                        maincharadata[id].hit += 1;
                        if(maincharadata[id].ctype == Souka)
                            maincharadata[id].hit += 1;
                        if(maincharadata[id].ctype == Chino && maincharadata[i].ctype == Souka)
                            maincharadata[id].hit +=3;
                        if(maincharadata[id].ctype == Denki){
                            if(maincharadata[id].skillf == 1){
                                info.skilltimer[id] = 5;
                                COMMAND data={SPECIAL,maincharadata[id].tar.x,maincharadata[id].tar.y,id};
                                SendData(ALL_CLIENTS,&data,sizeof(COMMAND));				
                            }	
                        }
                    }
                }else{
                    if(maincharadata[id].ctype == Kagaku){
                        if(maincharadata[id].skillf == 1){
                            info.skilltimer[id] = 5;
                            COMMAND data={SPECIAL,maincharadata[id].tar.x,maincharadata[id].tar.y,id};
                            SendData(ALL_CLIENTS,&data,sizeof(COMMAND));				
                        }	
                    }
                }
            }else{
                if(maincharadata[id].ctype == Kagaku){
                    if(maincharadata[id].skillf == 1){
                        info.skilltimer[id] = 5;
                        COMMAND data={SPECIAL,maincharadata[id].tar.x,maincharadata[id].tar.y,id};
                        SendData(ALL_CLIENTS,&data,sizeof(COMMAND));				
                    }	
                }
            }
        }
    }
    return 0;
}

