/*****************************************************************
ファイル名	: client_func.h
機能		: クライアントの外部関数の定義
*****************************************************************/

#ifndef _CLIENT_FUNC_H_
#define _CLIENT_FUNC_H_

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

//タイマ処理によって描写関数を呼び出す
extern Uint32 draw_timer(Uint32 interval, void *param);
extern Uint32 draw_timer_bat(Uint32 interval, void *param);

/* client_command.c */
extern int ExecuteCommand(char command);
extern void SendEndCommand(void);
extern void SendBall_x(SDL_Point pos_ball);
extern void SendBall_y(SDL_Point pos_ball);
extern void SendBatter_swing(void);
extern void SendPiti(void);

#endif
