/*****************************************************************
	ファイル名	: common.h
	機能		: サーバーとクライアントで使用する定数の宣言を行う
 *****************************************************************/

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_mixer.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>
#include <math.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define DEFAULT_PORT 51000
#define MAX_LEN_NAME 10
#define MAX_CLIENTS 4/* クライアント数の最大値 */
#define MAX_OBJECTS 4//オブジェクトの最大個数
#define MAX_ROUND 3//最大ラウンド数

#define MAX_DATA		200				/* 送受信するデータの最大値 */

#define END_COMMAND			'E'				/*終了*/
#define ATTAK				'A'				/*攻撃*/
#define MOVE				'M'				/*移動*/
#define HIT				'H'				/*あたり判定*/
#define SPECIAL				'T'				/*特殊*/
#define SELECT				'S'				/*選択*/
#define DECISION			'D'				/*決定*/
#define CANCEL				'C'				/*取り消し*/
#define ROUNDEND			'R'				/*ラウンド終了*/
#define INFO				'I'				/*情報表示*/
#define SCORE				'O'				/*スコア*/
#define STATAS				'J'				/*状態*/
typedef struct {
	char command;
	int i;
	int j;
	int k;
} COMMAND;
enum {
    Chino    = 0,
    Denki    = 1,
    Kikai    = 2,
    Kagaku   = 3,
    Kensetsu = 4,
    Seibutsu = 5,
    Hikari   = 6,
    Souka    = 7
};
#endif
