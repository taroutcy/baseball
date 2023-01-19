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
#include <stdbool.h>
#include <SDL2/SDL.h> // SDLを用いるために必要なヘッダファイルをインクルード
#include <SDL2/SDL_mixer.h> // SDLでサウンドを用いるために必要なヘッダファイルをインクルード

#define PORT (u_short)8888  /* ポート番号 */

#define MAX_CLIENTS 4       /* クライアント数の最大値 */
#define MAX_NAME_SIZE 10    /* ユーザー名の最大値*/

#define MAX_DATA 200        /* 送受信するデータの最大値 */

#define WINDOW_X 1200 //ウィンドウサイズx
#define WINDOW_Y 800 //ウィンドウサイズy

#define END_COMMAND 'E'     /* プログラム終了コマンド */
#define JUDGE_WIN 'W'       /* 勝ちを示すコマンド */
#define JUDGE_LOSE 'L'      /* 負けを示すコマンド */
#define JUDGE_GOBU 'G'      /* 引き分けを示すコマンド */

#define Batter_Swing_COMMAND 'S'
#define PITI 'P'
#define BALL_PRAM_X 'X' /* ボールの座標を示すコマンド */
#define BALL_PRAM_Y 'Y' /* ボールの座標を示すコマンド */


#define BALL_PRAM_YP 'p' /* ボールの座標を示すコマンド */

#define JUDGE_HIT 'H'       /* ヒットを示すコマンド */
#define JUDGE_TWOBASE 'T'    /* ツーベースを示すコマンド */
#define JUDGE_HOMERUN 'R'    /* ホームランを示すコマンド */
#define RESET 'A'
#define JUDGE 'J'

#define SEND_COUNT 'C'
#define STRIKE 'k'
#define BALL 'b'
#define OUT 'o'

#define NONE -1
#define STRAIGHT 1
#define ZIGZAG 2
#define DISAPPEAR 3
#define CURVE_R 4
#define CURVE_L 5
#define ACCELERATE 6

#define FIRST 'f'
#define SECOND 'z'
#define THIRD 't'

SDL_Renderer *gMainRenderer;

// timerID
SDL_TimerID my_timer_id;
SDL_TimerID my_timer_id_bat;

int ball_state;
int y;
int flg_ball_pattern; // 選択した球種
int flg_select_ball;  // 球種選択したか
int bat_disp;
int Reset;
int Bat_swing;

typedef struct { // ボールカウント格納用
    int ball; 
    int strike;
    int out;
} ball_count;

//music
Mix_Music *cheering,*start; // BGMデータ格納用構造体
Mix_Chunk *hit,*karaburi,*catch,*makyu,*curve1; // 効果音データ格納用構造体

int Batter_key;

typedef struct // ボール用構造体
{
    int x; // 座標
    int y;
    int r;  // 半径
    int xp; // 移動量(座標をどれだけ変化させるか)
    int yp;
} ball_param;

typedef struct {
    bool first;
    bool second;
    bool third;
} runners_list;

#endif
