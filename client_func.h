/*****************************************************************
ファイル名	: client_func.h
機能		: クライアントの外部関数の定義
*****************************************************************/

#ifndef _CLIENT_FUNC_H_
#define _CLIENT_FUNC_H_

#define WINDOW_X 1200
#define WINDOW_Y 800

#define STRAIGHT 1
#define ZIGZAG 2
#define DISAPPEAR 3
#define CURVE_R 4
#define CURVE_L 5
#define ACCELERATE 6

#include"common.h"

/* client_net.c */
extern int SetUpClient(char* hostName,int *clientID,int *num,char clientName[][MAX_NAME_SIZE]);
extern void CloseSoc(void);
extern int RecvIntData(int *intData);
extern void SendData(void *data,int dataSize);
extern int SendRecvManager(void);

/* client_win.c */
extern int InitWindows(int clientID,int num,char name[][MAX_NAME_SIZE]);
extern void DestroyWindow(void);
extern void WindowEvent(int num,int clientID);
extern void Present(int i);
extern void Base_present();
extern void Count_present();
extern void Text(int i);
extern ball_count count;
extern runners_list runners_recv;
extern int flg_batter_win_game;
extern int flg_increase_out;
extern int flg_recv_homerun;
extern int flg_recv_twobase;
extern int flg_recv_hit;
extern int ball_state;
extern int ball_state_Speed;

//タイマ処理によって描写関数を呼び出す
extern Uint32 draw_timer(Uint32 interval, void *param);
extern Uint32 draw_timer_bat(Uint32 interval, void *param);

/* client_command.c */
extern int ExecuteCommand(char command);
extern void SendEndCommand(void);
extern void SendBall_x(SDL_Point pos_ball);
extern void SendBall_y(SDL_Point pos_ball);
extern void SendBatter_swing(void);
extern void SendPiti(int i);
extern void Send_JUDGE(void);

#endif
