/*****************************************************************
ファイル名	: common.h
機能		: サーバーとクライアントで使用する定数の宣言を行う
*****************************************************************/

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <assert.h>
#include <math.h>
#include <SDL2/SDL.h> // SDLを用いるために必要なヘッダファイルをインクルード

#define PORT (u_short)8888  /* ポート番号 */

#define MAX_CLIENTS 4       /* クライアント数の最大値 */
#define MAX_NAME_SIZE 10    /* ユーザー名の最大値*/

#define MAX_DATA 200        /* 送受信するデータの最大値 */

#define WINDOW_X 800 //ウィンドウサイズx
#define WINDOW_Y 450 //ウィンドウサイズy

#define END_COMMAND 'E'     /* プログラム終了コマンド */
#define JUDGE_WIN 'W'       /* 勝ちを示すコマンド */
#define JUDGE_LOSE 'L'      /* 負けを示すコマンド */
#define JUDGE_GOBU 'G'      /* 引き分けを示すコマンド */

#define Batter_Swing_COMMAND 1
#define PITI 'P'
#define BALL_PRAM_X 'X' /* ボールの座標を示すコマンド */
#define BALL_PRAM_Y 'Y' /* ボールの座標を示すコマンド */


#define BALL_PRAM_YP 'P' /* ボールの座標を示すコマンド */

#define JUDGE_HIT 'H'       /* ヒットを示すコマンド */
#define JUDGE_TWOBASE 'T'    /* ツーベースを示すコマンド */
#define JUDGE_HOMERUN 'R'    /* ホームランを示すコマンド */

SDL_Renderer *gMainRenderer;

// timerID
SDL_TimerID my_timer_id;

int flag_swing_pi;
int y;

typedef struct // ボール用構造体
{
    int x; // 座標
    int y;
    int r;  // 半径
    int xp; // 移動量(座標をどれだけ変化させるか)
    int yp;
} ball_param;

#endif
