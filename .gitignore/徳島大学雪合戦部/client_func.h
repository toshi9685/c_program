/*****************************************************************
  ファイル名	: client_func.h
  機能		: クライアントの外部関数の定義
製作者：澁谷
 *****************************************************************/

#include<libcwiimote/wiimote.h>
#include<libcwiimote/wiimote_api.h>
#ifndef _CLIENT_FUNC_H_
#define _CLIENT_FUNC_H_
#include"common.h"
/* client_net.c */
extern int SetUpClient(char* hostName,int *clientID,int *num,u_short port);//サーバーと接続
extern void CloseSoc(void);//サーバーとの接続を切断する
extern int RecvIntData(int *intData);//クライアント番号の読み込み
extern void SendData(void *data,int dataSize);//サーバーにデータを送る
extern int SendRecvManager(int endFlag);//サーバーからデータが届いているか確認する。届いていたら処理をする。

/* client_win.c */
extern int InitWindows(void);//windowの初期化　待機画面の初期化。
extern int SelectWindows(int clientID);//キャラクタ選択画面の初期化。
extern int GameWindows(int clientID);//ゲーム画面の初期化。
extern void DestroyWindow(void);//SDLの終了処理。
extern void Repos(int id,int x,int y);//プレイヤーの座標を相対位置に変換する。
extern void ctypein(int ctype,int id);//キャラクタの種類を格納する。
extern void targetimport(int id,int x,int y);//プレイヤーが雪玉を投げる位置を格納する。
extern int thread_Calc(void *args);//雪玉の軌道計算をするスレッド。
extern int thread_Draw(void *args);//描画をするスレッド。
extern void Loadwin(void);//画面遷移の間に入れるロード画面。
extern void timerset(int t,int r);//タイマーをセットする。
extern void scoreset(int i,int j);//スコアをセットする。
extern void initobject(int id,int i,int j);//オブジェクト情報を格納する。
extern void result(int i,int j);//リザルト画面
extern void resetobject(int id);//オブジェクトのフラグを初期化する。
/* client_command.c */
extern int ExecuteCommand1(COMMAND com);//ラウンド中のコマンド処理
extern int ExecuteCommand2(COMMAND com);//キャラクタ選択画面中のコマンド処理
extern int ExecuteCommand3(COMMAND com);//リザルト画面中のコマンド処理
extern void SendCommand(char p,int a,int b,int c);//サーバーに送るデータをセットして送る。
extern void SendEndCommand(void);//終了コマンドをセットしてサーバーに送る。

typedef struct {
    int ctype; // 学科別に設定
    SDL_Rect target; // 雪玉を投げる位置
    SDL_Rect pos; //位置
    int isThrowing;//投げたかどうかのフラグ
    char team;//チーム
    int bexist;//キャラクタの状態
} STATUS;
typedef struct {
    double x;
    double y;
    int w;
} Ball;
typedef struct{
    int flag;//オブジェクトが存在するかどうか
    int otype;//オブジェクトの種類
    SDL_Rect pos;//オブジェクトを配置する位置
} OBJECTS;
extern STATUS player[MAX_CLIENTS];//キャラクタ情報
extern Ball ball[MAX_CLIENTS];//雪玉の情報
extern OBJECTS object[MAX_OBJECTS];//オブジェクト情報
#endif
