/*****************************************************************
ファイル名	: client_win.c
機能		: クライアントのユーザーインターフェース処理
*****************************************************************/

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <joyconlib.h> //Joy-Conを動かすために必要なヘッダファイルをインクルード
#include <time.h>
#include "common.h"
#include "client_func.h"

static SDL_Window *gMainWindow;
static SDL_Rect gButtonRect[MAX_CLIENTS + 2];
static int CheckButtonNO(int x, int y, int num);

//野球の描写および打つ判定の関数
void *draw(void *param);
void ball_straight(SDL_Renderer *renderer, ball_param ball);
int judge_swing(int flg_swing, SDL_Point pos_ball, SDL_Rect rect_bat);
int judge_strike(int flg_swing, ball_param ball, SDL_Rect rect_bat);

//野球関連
int flg_swing = 0;
SDL_Rect strike_zone = {350, 300, 100, 100};

//画像関連変数
SDL_Texture *texture2, *texture3, *texture4;
SDL_Surface *image, *image2, *image3, *image4;

// JoyConの状態を格納する変数
joyconlib_t jc;

//一度だけアニメーションを動作させるための鍵 0:ストップ, 1：実行
int Batter_key = 1;
int Pitya_key = 1;

//バッターがどの速度でスイングするのかを示す. Speedが0, 1, 2はそれぞれ遅い, 普通, 早い
int Batter_Speed = 0;
int Batter_Speed_back = -1;

/*****************************************************************
関数名	: InitWindows
機能	: メインウインドウの表示，設定を行う
引数	: int	clientID		: クライアント番号
		  int	num				: 全クライアント数
出力	: 正常に設定できたとき0，失敗したとき-1
*****************************************************************/
int InitWindows(int clientID, int num, char name[][MAX_NAME_SIZE])
{
	int i;
	SDL_Texture *texture;
	SDL_Rect src_rect;
	SDL_Rect dest_rect;
	char title[10];

	/* 引き数チェック */
	assert(0 < num && num <= MAX_CLIENTS);

	/* SDLの初期化, Joyconの初期化 */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
	{
		printf("failed to initialize SDL.\n");
		return -1;
	}

	SDL_Event quit_event = {SDL_QUIT}; // 特定のイベント名を格納


	/* メインのウインドウを作成する */
	if ((gMainWindow = SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 450, 0)) == NULL)
	{
		printf("failed to initialize videomode.\n");
		return -1;
	}

	gMainRenderer = SDL_CreateRenderer(gMainWindow, -1, SDL_RENDERER_SOFTWARE); // SDL_RENDERER_ACCELERATED |SDL_RENDERER_PRESENTVSYNC);//0);

	/* ウインドウのタイトルをセット */
	sprintf(title, "client%d", clientID + 1);
	SDL_SetWindowTitle(gMainWindow, title);

	/* 背景を白にする */
	SDL_SetRenderDrawColor(gMainRenderer, 0, 0, 0, 255);
	SDL_RenderClear(gMainRenderer);

	// BGMを流す\

	SDL_JoystickID joyid[2] = {};
	printf("%d\n", SDL_NumJoysticks());
	// 接続されているジョイスティックの名前を表示
	for (int i = 0; i < 2; i++)
	{											 // 接続されているジョイスティックの数だけ繰り返す
		SDL_Joystick *joy = SDL_JoystickOpen(i); // ジョイスティックを開く
	}
	// joycon接続しないとエラーするのでJoyCon処理しない方はコメントアウトのまま
	if (joycon_open(&jc, JOYCON_R))
	{
		printf("joycon open failed.\n");
		return -1;
	}

	SDL_RenderPresent(gMainRenderer); //描写
}

//クライアントの勝敗の結果を画面に表示する
void Present(int i)
{

	//白色で塗りつぶす
	boxColor(gMainRenderer, 50, 65, 380, 400, 0xffffffff); //

	//ヒットの時
	if (i == 1)
	{
	}
}

/*****************************************************************
関数名	: DestroyWindow
機能	: SDLを終了する
引数	: なし
出力	: なし
*****************************************************************/
void DestroyWindow(void)
{
	SDL_Quit();
}

Uint32 draw_timer(Uint32 interval, void *param)
{
	SDL_Event event;
	SDL_UserEvent userevent;

	/* コールバックでSDL_USEREVENTイベントをキューに入れる。
	このコールバック関数は一定の周期で再び呼ばれる */

	userevent.type = SDL_USEREVENT;
	userevent.code = 0;
	userevent.data1 = &draw;
	userevent.data2 = param;

	event.type = SDL_USEREVENT;
	event.user = userevent;

	SDL_PushEvent(&event);
	return (interval);
}

static ball_param ball = {400, 100, 10, 0, 0};
static SDL_Rect rect_bat = {350, 300, 100, 100};
static SDL_Point pos_ball;

//割り込みで呼び出す描写関数
void *draw(void *param)
{ // 描画関数

	static int flg_reverse = 0;

	gMainRenderer = (SDL_Renderer *)param;

	/* 描画 */
	SDL_SetRenderDrawColor(gMainRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(gMainRenderer); // 背景の描画

	SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(gMainRenderer, &rect_bat); // バット(枠)の描画

	filledCircleColor(gMainRenderer, ball.x, ball.y, ball.r, 0xffffffff);

	// 判定用定数に格納
	pos_ball.x = ball.x;
	pos_ball.y = ball.y;

	SendBall_x(pos_ball);
	SendBall_y(pos_ball);

	//ボールの中心座標が枠内にない時にスイングされると初期化
	if (!(SDL_PointInRect(&pos_ball, &rect_bat)))
	{
		flg_swing = 0;
	}
	//サーバーからの合図がきた
	if (y == 1)
	{
		ball.yp = -10;
	}
	if (flag_swing_pi == 1)
	{
		ball.yp = 10;
		flag_swing_pi = 0;
	}
	ball.y = ball.y + ball.yp; // 座標の更新
}

int judge_strike(int flg_swing, ball_param ball, SDL_Rect rect_bat)
{
	if (ball.y == WINDOW_Y && (ball.x >= (strike_zone.x) || ball.x <= (strike_zone.x + strike_zone.w)))
		return 1; // TODO&DISCUSS:ボール球の追加
	else
		return 0;
}

//バッターアニメーションを流す
void animeBatter(int i)
{
}

//どのバッターのスイングアニメーションを流すのかを判断する
void animeBatter_JUDGE(void)
{
	//バッターアニメーションは一度目か
	if (Batter_key == 1)
	{
		Batter_key = 0; //連続でスイングできないように初期化
		// 遅いスイングアニメーションを流す
		if (Batter_Speed == 0)
		{
			printf("遅い");
			animeBatter(0);
			SendBatter_swing();
			// printf("%d",pos_ball.x);
		}
		//普通のスイングアニメーションを流す
		if (Batter_Speed == 1)
		{
			printf("普通");
			animeBatter(1);
			SendBatter_swing();
		}
		//早いスイングアニメーションを流す
		if (Batter_Speed == 2)
		{
			printf("早い");
			animeBatter(2);
			SendBatter_swing();
		}
	}
}

//どのピッチャーアニメーションを流すのかを判断する
void animePitya_JUDGE(void)
{
		//バッターアニメーションは一度目か
	if (Pitya_key == 1)
	{
		Pitya_key = 0; //連続でスイングできないように初期化
		SendPiti();
	}
}

/*****************************************************************
関数名	: WindowEvent
機能	: メインウインドウに対するイベント処理を行う
引数	: int		num		: 全クライアント数
出力	: なし
*****************************************************************/
void WindowEvent(int num, int clientID)
{
	SDL_Event event;
	SDL_MouseButtonEvent *mouse;
	int buttonNO;

	joycon_get_state(&jc);

	/* 引き数チェック */
	assert(0 < num && num <= MAX_CLIENTS);

	SDL_RenderPresent(gMainRenderer); // ボールの描画

	// イベントを取得したなら
	while (SDL_PollEvent(&event))
	{
		// イベントの種類ごとに処理
		switch (event.type)
		{
			/* タイマー処理 */
		case SDL_USEREVENT:
		{
			void (*p)(void *) = event.user.data1;
			p(event.user.data2);
			break;
		}
		case SDL_KEYDOWN: // キーボードのキーが押された時
			// 押されたキーごとに処理
			switch (event.key.keysym.sym)
			{
			case SDLK_RETURN: // Enterキーが押された時
				printf("enter\n");

				flag_swing_pi = 1;
				break;
			case SDLK_ESCAPE:
				SendEndCommand();
				break;
			}
			break;
		// ジョイコンの加速度設定
		case SDL_JOYAXISMOTION:
			//もし打者ならば
			if (clientID == 0)
			{
				if (jc.axis[2].acc_y > 40.0 || jc.axis[2].acc_y < -40.0)
				{
					Batter_Speed = 2;
					flg_swing = 1;
					SendBall_x(pos_ball); //これによってボールのｘ座標を送る
					SendBall_y(pos_ball); //これによってボールのy座標を送る
					animeBatter_JUDGE();
				}
				else if (jc.axis[2].acc_y > 27.0 || jc.axis[2].acc_y < -27.0)
				{
					Batter_Speed = 1;
					flg_swing = 1;
					SendBall_x(pos_ball); //これによってボールのｘ座標を送る
					SendBall_y(pos_ball); //これによってボールのy座標を送る
					animeBatter_JUDGE();
				}
				else if (jc.axis[2].acc_y > 22.0 || jc.axis[2].acc_y < -22.0)
				{
					Batter_Speed = 0;
					flg_swing = 1;
					SendBall_x(pos_ball); //これによってボールのｘ座標を送る
					SendBall_y(pos_ball); //これによってボールのy座標を送る
					animeBatter_JUDGE();
				}
			}
			//もし投手ならば
			if (clientID == 1)
			{
				if (jc.axis[2].acc_y > 60.0 || jc.axis[2].acc_y < -60.0)
				{
					Batter_Speed = 2;
					animePitya_JUDGE();
				}
				else if (jc.axis[2].acc_y > 40.0 || jc.axis[2].acc_y < -40.0)
				{
					// printf("中");
					Batter_Speed = 1;
					animePitya_JUDGE();
				}
				else if (jc.axis[2].acc_y > 20.0 || jc.axis[2].acc_y < -20.0)
				{
					// printf("小");
					Batter_Speed = 0;
					animePitya_JUDGE();
				}
			}
			break;
		// ジョイスティックのボタンが押された時
		case SDL_JOYBUTTONDOWN:
			//ジョイコンのボタンと対応したギターコードを挿入
			if (jc.button.btn.A == 1)
			{
				printf("\n%d\n", Batter_Speed);
				Batter_Speed = 0; //リセット
				Batter_Speed_back = -1;
				Batter_key = 1;
			}
			if (jc.button.btn.B == 1)
			{
			}
			break;
		// SDLの利用を終了する時（イベントキューにSDL_QUITイベントが追加された時）
		case SDL_QUIT:
			SendEndCommand();
			break;
		}
	}
	SDL_Delay(10);
}

/*****
static
*****/
/*****************************************************************
関数名	: CheckButtonNO
機能	: クリックされたボタンの番号を返す
引数	: int	   x		: マウスの押された x 座標
		  int	   y		: マウスの押された y 座標
		  char	   num		: 全クライアント数
出力	: 押されたボタンの番号を返す
		  ボタンが押されていない時は-1を返す
*****************************************************************/
static int CheckButtonNO(int x, int y, int num)
{
	int i;

	for (i = 0; i < num + 2; i++)
	{
		if (gButtonRect[i].x < x &&
			gButtonRect[i].y < y &&
			gButtonRect[i].x + gButtonRect[i].w > x &&
			gButtonRect[i].y + gButtonRect[i].h > y)
		{
			return i;
		}
	}
	return -1;
}
