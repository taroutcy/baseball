/*****************************************************************
ファイル名	: client_win.c
機能		: クライアントのユーザーインターフェース処理
*****************************************************************/

#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <joyconlib.h>  //Joy-Conを動かすために必要なヘッダファイルをインクルード
#include <time.h>

#include "client_func.h"
#include "common.h"

static SDL_Window *gMainWindow;
static SDL_Rect gButtonRect[MAX_CLIENTS + 2];
static int CheckButtonNO(int x, int y, int num);

//野球の描写および打つ判定の関数
void *draw(void *param);
void ball_straight(SDL_Renderer *renderer, ball_param ball);
int judge_swing(int flg_swing, SDL_Point pos_ball, SDL_Rect rect_bat);
int judge_strike(int flg_swing, ball_param ball, SDL_Rect rect_bat);

//野球関連
int flg_swing = 0, bat_disp = 0;
int flg_ball_pattern = 0; // 選択した球種
int flg_select_ball = 0;  // 球種選択したか
int flag_swing_pi = 0;   // ゲーム開始したか
SDL_Rect strike_zone = {450, 500, 300, 200};

//画像関連変数
SDL_Surface *bat[3] = {NULL, NULL, NULL}, *stadium;
SDL_Texture *bat_tex[3], *stadium_tex;

int bat_num[3] = {40, 23, 16};
// JoyConの状態を格納する変数
joyconlib_t jc;

//一度だけアニメーションを動作させるための鍵 0:ストップ, 1：実行
int Batter_key = 1;
int Pitya_key = 1;

//バッターがどの速度でスイングするのかを示す. Speedが0, 1, 2はそれぞれ遅い, 普通, 早い
int Batter_Speed = 0, Batter_Speed_True = 0;
int Batter_Speed_back = -1;

//
int Onsei_key = 1;

int BallType = 0;


void *animeBatter();

/*****************************************************************
関数名	: InitWindows
機能	: メインウインドウの表示，設定を行う
引数	: int	clientID		: クライアント番号
          int	num				: 全クライアント数
出力	: 正常に設定できたとき0，失敗したとき-1
*****************************************************************/
int InitWindows(int clientID, int num, char name[][MAX_NAME_SIZE]) {
    int i;
    SDL_Texture *texture;
    SDL_Rect src_rect;
    SDL_Rect dest_rect;

    char title[10];

    /* 引き数チェック */
    assert(0 < num && num <= MAX_CLIENTS);

    /* SDLの初期化, Joyconの初期化 */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        printf("failed to initialize SDL.\n");
        return -1;
    }

    SDL_Event quit_event = {SDL_QUIT};  // 特定のイベント名を格納

    // SDL_mixerの初期化（MP3ファイルを使用）
    Mix_Init(MIX_INIT_MP3);

    // オーディオデバイスの初期化
    if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
        printf("failed to initialize SDL_mixer.\n");
        SDL_Quit();
        exit(-1);
    }

    // BGMと効果音のサウンドファイルの読み込み
    if((cheering = Mix_LoadMUS("cheering.mp3")) == NULL || (start = Mix_LoadMUS("start.mp3")) == NULL || (hit = Mix_LoadWAV("hit.wav")) == NULL 
    || (catch = Mix_LoadWAV("catch.wav")) == NULL || (karaburi = Mix_LoadWAV("karaburi.wav")) == NULL || (makyu = Mix_LoadWAV("makyu.wav")) == NULL
    || (curve1 = Mix_LoadWAV("curve1.wav")) == NULL) {
        printf("failed to load music and chunk.\n");
        Mix_CloseAudio(); // オーディオデバイスの終了
        SDL_Quit();
        exit(-1);
    }

     Mix_PlayMusic(cheering, -1); // BGMの再生（繰り返し再生）
     Mix_VolumeMusic(MIX_MAX_VOLUME/5); // BGMの音量を半減
    

    /* メインのウインドウを作成する */
    if ((gMainWindow = SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800, 0)) == NULL) {
        printf("failed to initialize videomode.\n");
        return -1;
    }

    gMainRenderer = SDL_CreateRenderer(gMainWindow, -1, SDL_RENDERER_SOFTWARE);  // SDL_RENDERER_ACCELERATED |SDL_RENDERER_PRESENTVSYNC);//0);

    /* ウインドウのタイトルをセット */
    sprintf(title, "client%d", clientID + 1);
    SDL_SetWindowTitle(gMainWindow, title);

    /* 背景を白にする */
    SDL_RenderClear(gMainRenderer);

    // BGMを流す\

    SDL_JoystickID joyid[2] = {};
    printf("%d\n", SDL_NumJoysticks());
    // 接続されているジョイスティックの名前を表示
    for (int i = 0; i < 2; i++) {                 // 接続されているジョイスティックの数だけ繰り返す
        SDL_Joystick *joy = SDL_JoystickOpen(i);  // ジョイスティックを開く
    }
    // joycon接続しないとエラーするのでJoyCon処理しない方はコメントアウトのまま
    if (joycon_open(&jc, JOYCON_R)) {
        printf("joycon open failed.\n");
        return -1;
    }

    bat[0] = IMG_Load("picture/bat_slow.png");
    bat[1] = IMG_Load("picture/bat_normal.png");
    bat[2] = IMG_Load("picture/bat_fast.png");
    stadium = IMG_Load("picture/stadium.png");

    if (!bat[0] || !bat[1] || !bat[2] || !stadium) {
        printf("image not load");
    }

    bat_tex[0] = SDL_CreateTextureFromSurface(gMainRenderer, bat[0]);
    bat_tex[1] = SDL_CreateTextureFromSurface(gMainRenderer, bat[1]);
    bat_tex[2] = SDL_CreateTextureFromSurface(gMainRenderer, bat[2]);
    stadium_tex = SDL_CreateTextureFromSurface(gMainRenderer, stadium);

    SDL_FreeSurface(bat[0]);
    SDL_FreeSurface(bat[1]);
    SDL_FreeSurface(bat[2]);
    SDL_FreeSurface(stadium);

    SDL_RenderPresent(gMainRenderer);  //描写
}

//クライアントの勝敗の結果を画面に表示する
void Present(int i) {
    //白色で塗りつぶす
    boxColor(gMainRenderer, 50, 65, 380, 400, 0xffffffff);  //

    if(Onsei_key == 1){
        Onsei_key = 0;
        //ヒットの時
        if (i == 1) {
            Mix_PlayChannel(1, hit, 0);
            printf("hit"); 
        }else if(i == 2){
            printf("Tow");
            Mix_PlayChannel(1, hit, 0); 
        }else if(i == 3){
            printf("homerun");
            Mix_PlayChannel(1, hit, 0); 
        }
    }
}

/*****************************************************************
関数名	: DestroyWindow
機能	: SDLを終了する
引数	: なし
出力	: なし
*****************************************************************/
void DestroyWindow(void) {
    SDL_Quit();
}

Uint32 draw_timer(Uint32 interval, void *param) {
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

static ball_param ball = {600, 100, 10, 0, 0};
static SDL_Rect rect_bat = {450, 500, 300, 200};
static SDL_Rect rect_bat2 = {500, 550, 200, 100};
static SDL_Rect rect_bat3 = {560, 560, 80, 80};
static SDL_Point pos_ball;

//割り込みで呼び出す描写関数
void *draw(void *param) {  // 描画関数

    static int flg_erase_ball = 0;
    static int flg_hit = 0;

    gMainRenderer = (SDL_Renderer *)param;

    /* 描画 */

    SDL_Rect stdRect = {0, 0, 1800, 1200};
    SDL_Rect drawStdRect = {0, 0, 1200, 800};

    SDL_RenderCopy(gMainRenderer, stadium_tex, &stdRect, &drawStdRect);

    SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawRect(gMainRenderer, &rect_bat);  // バット(枠)の描画 (hit)
    SDL_SetRenderDrawColor(gMainRenderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawRect(gMainRenderer, &rect_bat2);  // バット(枠)の描画 (Two)
    SDL_SetRenderDrawColor(gMainRenderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawRect(gMainRenderer, &rect_bat3);  // バット(枠)の描画 (three)
        

    if (flg_erase_ball == 0)
    {
        filledCircleColor(gMainRenderer, ball.x, ball.y, ball.r, 0xffffffff);
        SDL_RenderPresent(gMainRenderer); // ボールの描画
    }
    else
    {
        SDL_RenderPresent(gMainRenderer);
    }

    // 判定用定数に格納
    pos_ball.x = ball.x;
    pos_ball.y = ball.y;

    SendBall_x(pos_ball);
    SendBall_y(pos_ball);

    //ボールの中心座標が枠内にない時にスイングされると初期化
    if (!(SDL_PointInRect(&pos_ball, &rect_bat))) {
        flg_swing = 0;
    }

    if (flag_swing_pi == 0) {
        ball.x = 600;
        ball.y = 100;
        ball.r = 10;
        ball.xp = 0;
        ball.yp = 0;
        Pitya_key = 1;
        y = 0;
    }
    if(Reset == 1){
        Batter_key = 1;
        Onsei_key = 1;
    }
    Reset = 0;

    if(BallType == STRAIGHT && Bat_swing == 0 && ball.y > 800){
        printf("sutoraiku");
    }
    if(BallType == ZIGZAG && Bat_swing == 0 && ball.y > 800){
        printf("sutoraiku");
    }
    if(BallType == DISAPPEAR && Bat_swing == 0 && ball.y > 800){
        printf("ball");
    }
    if(BallType == DISAPPEAR && Bat_swing == 1 && ball.y > 800){
        printf("strike");
    }
    if(BallType == CURVE_R && Bat_swing == 0 && (ball.x > 1200 || ball.x < 0)){
        printf("ball");
    }
    if(BallType == CURVE_R && Bat_swing == 1 && (ball.x > 1200 || ball.x < 0)){
        printf("strike");
    }
    if(BallType == CURVE_L && Bat_swing == 0 && (ball.x > 1200 || ball.x < 0)){
        printf("ball");
    }
    if(BallType == CURVE_L && Bat_swing == 1 && (ball.x > 1200 || ball.x < 0)){
        printf("strike");
    }
    if(BallType == ACCELERATE && Bat_swing == 0 && ball.y > 800){
        printf("sutoraiku");
    }
    


    switch (flg_ball_pattern)
    {
    case STRAIGHT:
        if (flag_swing_pi == 1)
        {
            ball.yp = 15;
            flag_swing_pi = 2;
            BallType = STRAIGHT;
        }
        break;
    case ZIGZAG:
        if (flag_swing_pi == 1)
        {
            ball.xp = 50;
            ball.yp = 10;
            flag_swing_pi = 2;
            BallType = ZIGZAG;
        }
        if (ball.x <= 300 || ball.x >= 900)
            ball.xp *= -1;
        break;
    case DISAPPEAR:
        if (flag_swing_pi == 1)
        {
            ball.yp = 15;
            flag_swing_pi = 2;
            Mix_PlayChannel(1,makyu,0);
            Mix_Volume(1,MIX_MAX_VOLUME);
            BallType = DISAPPEAR;
        }
        if (flg_hit == 0 && ball.y >= 350 && flag_swing_pi == 2)
            flg_erase_ball = 1;
        break;
    case CURVE_R:
        if(flag_swing_pi == 1)
        {
            Mix_PlayChannel(1,curve1,1);
            Mix_Volume(1,MIX_MAX_VOLUME);
            ball.yp = 15;
            flag_swing_pi = 2;
            BallType = CURVE_R;
        }
        if(flag_swing_pi == 2)
        {
            ball.xp += 4; 
        }
        if(flag_swing_pi == 3)
        {
            ball.xp -= 4; 
        }
        if(ball.y >= 225 && flg_hit == 0){
            flag_swing_pi = 3;
        }
        break;
    
    case CURVE_L:
        if(flag_swing_pi == 1)
        {
            Mix_PlayChannel(1,curve1,1);
             Mix_Volume(1,MIX_MAX_VOLUME);
            ball.yp = 15;
            flag_swing_pi = 2;
            BallType = CURVE_L;
        }
        if(flag_swing_pi == 2)
        {
            ball.xp -= 4; 
        }
        if(flag_swing_pi == 3)
        {
            ball.xp += 4; 
        }
        if(ball.y >= 225 && flg_hit == 0){
            flag_swing_pi = 3;
        }
        break;
    case ACCELERATE:
        if(flag_swing_pi == 1)
        {
            ball.yp = 10;
            flag_swing_pi = 2;
            BallType = ACCELERATE;
        }
        if(flag_swing_pi == 2)
        {
            ball.yp += 3; 
        }
        break;
    }

    //サーバーからの合図がきた
    if (y == 1 && flg_erase_ball == 0) {
        ball.xp = 0;
        ball.yp = -20;
    }
    else if (y == 1 && flg_erase_ball == 1){
        flg_erase_ball = 0;
        ball.xp = 0;
        ball.yp = -20;
    }

    
    // 座標の更新
    ball.x = ball.x + ball.xp;
    ball.y = ball.y + ball.yp;
}

int judge_strike(int flg_swing, ball_param ball, SDL_Rect rect_bat) {
    if (ball.y == WINDOW_Y && (ball.x >= (strike_zone.x) || ball.x <= (strike_zone.x + strike_zone.w)))
        return 1;  // TODO&DISCUSS:ボール球の追加
    else
        return 0;
}

Uint32 draw_timer_bat(Uint32 interval, void *param) {
    SDL_Event event;
    SDL_UserEvent userevent;

    /* コールバックでSDL_USEREVENTイベントをキューに入れる。
    このコールバック関数は一定の周期で再び呼ばれる */

    userevent.type = SDL_USEREVENT;
    userevent.code = 0;
    userevent.data1 = &animeBatter;
    userevent.data2 = param;

    event.type = SDL_USEREVENT;
    event.user = userevent;

    SDL_PushEvent(&event);
    return (interval);
}

//バッターアニメーションを流す
void *animeBatter() {
    static int count_disp = 0;  // 表示した回数

    SDL_Surface *img = NULL;
    int img_size = 1000;  // 一枚の画像の縦と横の大きさ
    int draw_size = 600;

    if (bat_disp == 1) {
        SDL_Rect imgRect = (SDL_Rect){img_size * count_disp, 0, img_size, img_size};
        SDL_Rect drawRect = (SDL_Rect){80, 260, draw_size, draw_size};

        SDL_RenderCopy(gMainRenderer, bat_tex[Batter_Speed_True], &imgRect, &drawRect);
        SDL_RenderPresent(gMainRenderer);

        //printf("butter speed: %d\n", Batter_Speed_True);
        count_disp++;

        if (count_disp >= bat_num[Batter_Speed_True]) {
            bat_disp = 0;
            count_disp = 0;
        }
    }
}

//どのバッターのスイングアニメーションを流すのかを判断する
void animeBatter_JUDGE(void) {
    //バッターアニメーションは一度目か
    if (Batter_key == 1) {       
        Batter_key = 0;  //連続でスイングできないように初期化
        // 遅いスイングアニメーションを流す
        if (Batter_Speed == 0) {
            printf("遅い");
            SendBatter_swing();
            Send_JUDGE();
            Batter_Speed_True = 0;
            Mix_PlayChannel(1,karaburi,0);
            Mix_Volume(1,MIX_MAX_VOLUME*10);
            // printf("%d",pos_ball.x);
        }
        //普通のスイングアニメーションを流す
        if (Batter_Speed == 1) {
            printf("普通");
            SendBatter_swing();
            Send_JUDGE();
            Batter_Speed_True = 1;
            Mix_PlayChannel(1,karaburi,0);
            Mix_Volume(1,MIX_MAX_VOLUME*100);
        }
        //早いスイングアニメーションを流す
        if (Batter_Speed == 2) {
            printf("早い");
            SendBatter_swing();
            Send_JUDGE();
            Batter_Speed_True = 2;
            Mix_PlayChannel(1,karaburi,0);
            Mix_Volume(1,MIX_MAX_VOLUME*150);
        }
    }
}

//どのピッチャーアニメーションを流すのかを判断する
void animePitya_JUDGE(void) {
    //バッターアニメーションは一度目か
    if (Pitya_key == 1) {
        Pitya_key = 0;  //連続でスイングできないように初期化
        SendPiti();
    }
}

/*****************************************************************
関数名	: WindowEvent
機能	: メインウインドウに対するイベント処理を行う
引数	: int		num		: 全クライアント数
出力	: なし
*****************************************************************/
void WindowEvent(int num, int clientID) {
    SDL_Event event;
    SDL_MouseButtonEvent *mouse;
    int buttonNO;

    joycon_get_state(&jc);

    /* 引き数チェック */
    assert(0 < num && num <= MAX_CLIENTS);

    SDL_SetRenderDrawColor(gMainRenderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(gMainRenderer);

    // イベントを取得したなら
    while (SDL_PollEvent(&event)) {
        // イベントの種類ごとに処理
        switch (event.type) {
                /* タイマー処理 */
            case SDL_USEREVENT: {
                void (*p)(void *) = event.user.data1;
                p(event.user.data2);
                break;
            }
            case SDL_KEYDOWN:  // キーボードのキーが押された時
                // 押されたキーごとに処理
                switch (event.key.keysym.sym) {
                    case SDLK_RETURN:  // Enterキーが押された時
                        printf("enter\n");
                        break;
                    case SDLK_ESCAPE:
                        SendEndCommand();
                        break;
                }
                break;
            // ジョイコンの加速度設定
            case SDL_JOYAXISMOTION:
                //もし打者ならば
                if (clientID == 0) {
                    if (jc.axis[2].acc_y > 25.0 || jc.axis[2].acc_y < -25.0) {
                        Batter_Speed = 2;
                        animeBatter_JUDGE();
                    } else if (jc.axis[2].acc_y > 20.0 || jc.axis[2].acc_y < -20.0) {
                        Batter_Speed = 1;
                        animeBatter_JUDGE();
                    } else if (jc.axis[2].acc_y > 15.0 || jc.axis[2].acc_y < -15.0) {
                        Batter_Speed = 0;
                        animeBatter_JUDGE();
                    }
                }
                //もし投手ならば
                if (clientID == 1) {
                    if (jc.axis[2].acc_y > 60.0 || jc.axis[2].acc_y < -60.0) {
                        Batter_Speed = 2;
                        animePitya_JUDGE();
                    } else if (jc.axis[2].acc_y > 40.0 || jc.axis[2].acc_y < -40.0) {
                        // printf("中");
                        Batter_Speed = 1;
                        animePitya_JUDGE();
                    } else if (jc.axis[2].acc_y > 20.0 || jc.axis[2].acc_y < -20.0) {
                        // printf("小");
                        Batter_Speed = 0;
                        animePitya_JUDGE();
                    }
                }
                break;
            // ジョイスティックのボタンが押された時
            case SDL_JOYBUTTONDOWN:
                if(clientID == 1){
                    //ジョイコンのボタンと対応したを挿入
                    if (jc.button.btn.A == 1 && flag_swing_pi == 0) {
                        SendBallType(STRAIGHT);
                        printf("選択された球種: ストレート\n");
                    }
                    if (jc.button.btn.B == 1 && flag_swing_pi == 0) {
                        SendBallType(ZIGZAG);
                        printf("選択された球種: ジグザグ\n");
                    }
                    if(jc.button.btn.Y == 1 && flag_swing_pi == 0){
                        SendBallType(DISAPPEAR);
                        printf("選択された球種: 消えるストレート\n");
                    }
                    if(jc.button.btn.X == 1 && flag_swing_pi == 0){
                        SendBallType(CURVE_R);
                        printf("選択された球種: 右カーブ\n");
                    }
                    if(jc.button.btn.R == 1 && flag_swing_pi == 0){
                        SendBallType(CURVE_L);
                        printf("選択された球種: 左カーブ\n");
                    }
                    if(jc.button.btn.ZR == 1 && flag_swing_pi == 0){
                        SendBallType(ACCELERATE);
                        printf("選択された球種: 加速ストレート\n");
                    }
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
static int CheckButtonNO(int x, int y, int num) {
    int i;

    for (i = 0; i < num + 2; i++) {
        if (gButtonRect[i].x < x &&
            gButtonRect[i].y < y &&
            gButtonRect[i].x + gButtonRect[i].w > x &&
            gButtonRect[i].y + gButtonRect[i].h > y) {
            return i;
        }
    }
    return -1;
}
