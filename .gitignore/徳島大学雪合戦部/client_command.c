/*****************************************************************
  ファイル名	: client_command.c
  機能		: クライアントのコマンド処理
制作者：澁谷
 *****************************************************************/

#include"client_func.h"
STATUS player[MAX_CLIENTS];
OBJECTS object[MAX_OBJECTS];
/*****************************************************************
  関数名	: ExecuteCommand１
  機能	: ラウンド中にサーバーから送られてきたコマンドを元に，
  引き数を受信し，実行する
  引数	: CONTAINER com		: コマンド
 *****************************************************************/
int ExecuteCommand1(COMMAND com)
{
    int	endflag = 1;
    switch(com.command){
    case ATTAK:
        targetimport(com.k,com.i,com.j);
        endflag = 1;
        break;
    case SPECIAL:
        initobject(com.k,com.i,com.j);
        if(com.i==0&&com.j==0)
            resetobject(com.k);
        break;
    case MOVE:
        Repos(com.k,com.i,com.j);
        endflag = 1;
        break;
    case HIT:
        break;
    case ROUNDEND:
        endflag = 2;
        if(com.i == 0&&com.j==0&&com.k== 0)
            endflag=3;
        break;
    case INFO:
        timerset(com.i,com.j);
        endflag = 1;
        break;
    case SCORE:
        scoreset(com.i,com.j);
        break;
    case END_COMMAND:
        endflag = 0;
        break;
    default:
        fprintf(stderr,"EX1 0x%02x is not command!\n",com.command);
    }
    return endflag;
}
/***************************************
関数名：Executecommand2
機能：キャラクタ選択画面中にサーバーから送られてきたコマンドを処理する。
 *************************************/
int ExecuteCommand2(COMMAND com)
{
    int	endflag = 2;
    switch(com.command){
    case SELECT:
        ctypein(com.i,com.k);
        endflag = 2;
        break;
    case DECISION:
        endflag = 1;
        break;
    case END_COMMAND:
        endflag = 0;
        break;
    default:
        fprintf(stderr,"EX2 0x%02x is not command!\n",com.command);
    }
    return endflag;
}
/****************************************
関数名：ExecuteCommand3
機能：リザルト画面中のコマンド処理
 ***************************************/
int ExecuteCommand3(COMMAND com)
{
    int	endflag = 3;
    switch(com.command){
    case SCORE:
        result(com.i,com.j);
        break;
    case END_COMMAND:
        endflag = 0;
        break;
    default:
        fprintf(stderr,"EX2 0x%02x is not command!\n",com.command);
    }

    return endflag;
}
/*****************************************************************
  関数名	: SendCommand
  機能	:  サーバーにデータを送る
  引数	: int		pos
  出力	: なし
 *****************************************************************/
void SendCommand(char p,int a,int b,int c)
{
    COMMAND data;
    /* コマンドのセット */
    data.command = p;
    data.i = a;
    data.j = b;
    data.k = c;
    /* データの送信 */
    SendData(&data, sizeof(COMMAND));
}

/*****************************************************************
  関数名	: SendEndCommand
  機能	: プログラムの終了を知らせるために，
  サーバーにデータを送る
  引数	: なし
  出力	: なし
 *****************************************************************/
void SendEndCommand(void)
{
    COMMAND 		data;
    /* コマンドのセット */
    data.command = END_COMMAND;
    /* データの送信 */
    SendData(&data, sizeof(COMMAND));
}


