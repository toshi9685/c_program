/*****************************************************************
ファイル名	: server_common.h
機能		: サーバーで使用する定数の宣言を行う
*****************************************************************/

#ifndef _SERVER_COMMON_H_
#define _SERVER_COMMON_H_

#include"common.h"

#define ALL_CLIENTS	-1   /* 全クライアントにデータを送る時に使用する */

typedef struct{
	int ctype;
	int hit;//キャラクタが相手に当てた数
	SDL_Rect pos;//キャラクタの位置
	SDL_Rect tar;
	int thr;
	char team;
	int skillf;
	int bexist;
} CONTAINER;
typedef struct{
	int round;
	int timer;
	int shohai[2];
	int skilltimer[MAX_CLIENTS]; 
} GAMEINFO;
/*typedef struct{
	int flag[MAX_OBJECTS];
	int otype[MAX_OBJECTS];
	SDL_Rect pos[MAX_OBJECTS];
} OBJECTS;*/
CONTAINER maincharadata[MAX_CLIENTS];
GAMEINFO info;
#endif
