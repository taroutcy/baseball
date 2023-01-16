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
int ball_state = 0;   // ゲーム開始したか
SDL_Rect strike_zone = {450, 500, 300, 200};

//画像関連変数
SDL_Surface *bat[3] = {NULL, NULL, NULL}, *stadium;
SDL_Texture *bat_tex[3], *stadium_tex;
SDL_Surface *base1, *base2;
SDL_Surface *light_g1, *light_g2, *light_y1, *light_y2, *light_r, *light_r2;
SDL_Surface *tex_B, *tex_S, *tex_O;
SDL_Surface *hit1, *hit2, *hit3;
SDL_Surface *text_p, *text_b, *text_s, *text_o, *text_w, *text_l;
SDL_Texture *bg, *base_out, *base_in;
SDL_Texture *texture_g1, *texture_g2, *texture_y1, *texture_y2, *texture_r, *texture_r2;
SDL_Texture *B, *S, *O;
SDL_Texture *HIT1, *HIT2, *HOME;
SDL_Texture *play, *strike, *ba, *out, *win, *lose;

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

//画像サイズ
float sc = 2.4;	// サイズの倍率
int base_show_x, base_show_y, base_show_w, base_show_h;
int base_w, base_h;
int count_x, count_y, count_w, count_h;
int light_s;

int runner = 0; //ランナー数
int b_count = 0;    //ボール数
int s_count = 0;    //ストライク数
int o_count = 0;    //アウト数

int i_key = 0;

int base[3] = { 0, 0, 0 };

int text_num = 0;
int flg_judge = 0;

int previous_time=0, current_time;

// ball_count receive;

ball_count pre = { 0, 0, 0 };

int c_id;

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

    base_w = 30*sc;	// 塁の幅
    base_h = 20*sc;	// 塁の高さ
    base_show_w = 100*sc;	// 出塁画面の幅
    base_show_h = 70*sc;	// 出塁画面の高さ
    base_show_x = 1200-base_show_w-5*sc;	// 出塁画面のx座標
    base_show_y = 800-base_show_h-5*sc;	// 出塁画面のy座標

    count_w = 100*sc+2*sc;
    count_h = 80*sc+2*sc;
    count_x = 1200-count_w-4*sc;
    count_y = 800-count_h-10*sc-base_show_h;
    light_s = 18*sc;

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
    base1 = IMG_Load("white.png");
    base2 = IMG_Load("red.png");
    light_g1 = IMG_Load("g1.png");
    light_g2 = IMG_Load("g2.png");
    light_y1 = IMG_Load("y1.png");
    light_y2 = IMG_Load("y2.png");
    light_r = IMG_Load("r.png");
    light_r2 = IMG_Load("r2.png");
    tex_B = IMG_Load("B.png");
    tex_S = IMG_Load("S.png");
    tex_O = IMG_Load("O.png");
    hit1 = IMG_Load("hit.png");
    hit2 = IMG_Load("double.png");
    hit3 = IMG_Load("homerun.png");
    text_p =IMG_Load("play.png");
    text_b =IMG_Load("ball.png");
    text_s =IMG_Load("strike.png");hit1 = IMG_Load("hit.png");
    hit2 = IMG_Load("double.png");
    hit3 = IMG_Load("homerun.png");
    text_p =IMG_Load("play.png");
    text_b =IMG_Load("ball.png");
    text_s =IMG_Load("strike.png");
    text_o =IMG_Load("out.png");
    text_w =IMG_Load("win.png");
    text_l =IMG_Load("lose.png");
    text_o =IMG_Load("out.png");
    text_w =IMG_Load("win.png");
    text_l =IMG_Load("lose.png");

    if (!bat[0] || !bat[1] || !bat[2] || !stadium) {
        printf("image not load");
    }

    bat_tex[0] = SDL_CreateTextureFromSurface(gMainRenderer, bat[0]);
    bat_tex[1] = SDL_CreateTextureFromSurface(gMainRenderer, bat[1]);
    bat_tex[2] = SDL_CreateTextureFromSurface(gMainRenderer, bat[2]);
    stadium_tex = SDL_CreateTextureFromSurface(gMainRenderer, stadium);
    base_out = SDL_CreateTextureFromSurface(gMainRenderer, base1);
    base_in = SDL_CreateTextureFromSurface(gMainRenderer, base2);
    texture_g1 = SDL_CreateTextureFromSurface(gMainRenderer, light_g1);
    texture_g2 = SDL_CreateTextureFromSurface(gMainRenderer, light_g2);
    texture_y1 = SDL_CreateTextureFromSurface(gMainRenderer, light_y1);
    texture_y2 = SDL_CreateTextureFromSurface(gMainRenderer, light_y2);
    texture_r = SDL_CreateTextureFromSurface(gMainRenderer, light_r);
    texture_r2 = SDL_CreateTextureFromSurface(gMainRenderer, light_r2);
    B = SDL_CreateTextureFromSurface(gMainRenderer, tex_B);
    S = SDL_CreateTextureFromSurface(gMainRenderer, tex_S);
    O = SDL_CreateTextureFromSurface(gMainRenderer, tex_O);
    HIT1 = SDL_CreateTextureFromSurface(gMainRenderer, hit1);
    HIT2 = SDL_CreateTextureFromSurface(gMainRenderer, hit2);
    HOME = SDL_CreateTextureFromSurface(gMainRenderer, hit3);
    play = SDL_CreateTextureFromSurface(gMainRenderer, text_p);
    strike = SDL_CreateTextureFromSurface(gMainRenderer, text_s);
    ba = SDL_CreateTextureFromSurface(gMainRenderer, text_b);
    out = SDL_CreateTextureFromSurface(gMainRenderer, text_o);
    win = SDL_CreateTextureFromSurface(gMainRenderer, text_w);
    lose = SDL_CreateTextureFromSurface(gMainRenderer, text_l);

    SDL_FreeSurface(bat[0]);
    SDL_FreeSurface(bat[1]);
    SDL_FreeSurface(bat[2]);
    SDL_FreeSurface(stadium);
    SDL_FreeSurface(base1);
    SDL_FreeSurface(base2);
    SDL_FreeSurface(light_g1);
    SDL_FreeSurface(light_g2);
    SDL_FreeSurface(light_y1);
    SDL_FreeSurface(light_y2);
    SDL_FreeSurface(light_r);
    SDL_FreeSurface(light_r2);
    SDL_FreeSurface(tex_B);
    SDL_FreeSurface(tex_S);
    SDL_FreeSurface(tex_O);
    SDL_FreeSurface(hit1);
    SDL_FreeSurface(hit2);
    SDL_FreeSurface(hit3);
    SDL_FreeSurface(text_p);
    SDL_FreeSurface(text_s);
    SDL_FreeSurface(text_b);
    SDL_FreeSurface(text_o);
    SDL_FreeSurface(text_w);
    SDL_FreeSurface(text_l);

    SDL_RenderPresent(gMainRenderer);  //描写
}

//クライアントの勝敗の結果を画面に表示する
void Present(int i) {
    base[runner] = i;

    for ( int j=0; j<runner ; j++) {
    	base[j] += i;
    }
    
    runner++;

    if(Onsei_key == 1){
        Onsei_key = 0;
        //ヒットの時
        if (i == 1) {
            Mix_PlayChannel(1, hit, 0);
            printf("hit"); 
        }else if(i == 2){
            printf("TwoBase");
            Mix_PlayChannel(1, hit, 0); 
        }else if(i == 4){
            printf("homerun");
            Mix_PlayChannel(1, hit, 0);
            flg_judge = 2;
        }
    }
    /*text_num = i;
    previous_time = SDL_GetTicks();*/
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
static SDL_Rect rect_bat3 = {580, 580, 40, 40};
static SDL_Point pos_ball;

//割り込みで呼び出す描写関数
void *draw(void *param) {  // 描画関数

    static int flg_erase_ball = 0;
    static int flg_hit = 0;

    static int flag_over_l_edge = 0;
    static int flag_over_r_edge = 0;

    gMainRenderer = (SDL_Renderer *)param;

    /* 描画 */

    SDL_Rect stdRect = {0, 0, 1800, 1200};
    SDL_Rect drawStdRect = {0, 0, 1200, 800};

    SDL_RenderCopy(gMainRenderer, stadium_tex, &stdRect, &drawStdRect);

    Count_present();
    Base_present();

    SDL_SetRenderDrawColor(gMainRenderer, 255, 255, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawRect(gMainRenderer, &rect_bat);  // バット(枠)の描画 (hit)
    SDL_SetRenderDrawColor(gMainRenderer, 0, 0, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawRect(gMainRenderer, &rect_bat2);  // バット(枠)の描画 (Two)
    SDL_SetRenderDrawColor(gMainRenderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderDrawRect(gMainRenderer, &rect_bat3);  // バット(枠)の描画 (three)

    /*if (pre.strike != count.strike){
        pre.strike = count.strike;
        text_num = 5;
        previous_time = SDL_GetTicks();
    }
    if (pre.ball != count.ball){
        pre.ball = count.ball;
        text_num = 6;
        previous_time = SDL_GetTicks();
    }
    if (pre.out != count.out){
        pre.out = count.out;
        text_num = 7;
        previous_time = SDL_GetTicks();
    }*/

    /*current_time = SDL_GetTicks(); // 経過時間を整数型変数に格納
    if (current_time < previous_time + 2000){ // 前のタイミングから2秒間
        Text(text_num);
    }*/
    /*if ( count.out >2 ) {
        flg_judge = 1;
    }*/
    if (flg_judge == 2){
        if (c_id == 0){
            Text(10);
        }
        if (c_id == 1){
            Text(11);
        }
    }
    if (flg_judge == 1){
        if (c_id == 0){
            Text(11);
        }
        if (c_id == 1){
            Text(10);
        }
    }
        

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

    if (ball_state == 0) {
        ball.x = 600;
        ball.y = 100;
        ball.r = 10;
        ball.xp = 0;
        ball.yp = 0;
        Pitya_key = 1;
        y = 0;
        i_key = 0;
    }
    if(Reset == 1){
        Batter_key = 1;
        Onsei_key = 1;
    }
    Reset = 0;

    if(i_key == 0){
        if (ball.x > 1200){
            flag_over_r_edge = 1;
            i_key = 1;
        }
        if (ball.x < 0) {
            flag_over_l_edge = 1;
            i_key = 1;
        }
    }
    

    // ---------------
    // if(BallType == STRAIGHT && Bat_swing == 0 && ball.y > 800){
    //     printf("strike 1\n");
    //     s_count++;
    // }
    // if(BallType == ZIGZAG && Bat_swing == 0 && ball.y > 800){
    //     printf("strike 2\n");
    //     s_count++;
    // }

    // // Judge Disapper Ball
    // if(BallType == DISAPPEAR && ball.y > 800) {
    //     switch (Bat_swing) {
    //         case 0:
    //             printf("ball of disapper\n");
    //             b_count++;
    //             break;
    //         case 1:
    //             printf("strike of disapper\n");
    //             s_count++;
    //             break;
    //     }

    // }

    // // Judge Right Curve
    // if(BallType == CURVE_R  && (ball.x < 0 || ball.x > 1200)){ 
    //     switch (Bat_swing){
    //         case 0:
    //             printf("ball of right curve\n");
    //             b_count++;
    //             break;
    //         case 1:
    //             printf("strike of right curve\n");
    //             s_count++;
    //             break;
    //     }
    //     flag_over_l_edge = 0;
    //     flag_over_r_edge = 0;
    // }
    // //if(BallType == CURVE_L && Bat_swing == 0 && (flag_over_l_edge == 1 || flag_over_r_edge == 1)){
    // if(BallType == CURVE_L && Bat_swing == 0 && (ball.x < 0 || ball.x > 1200)){
    //     printf("ball 3\n");
    //     flag_over_l_edge = 0;
    //     flag_over_r_edge = 0;
    //     b_count++;
    // }
    // //if(BallType == CURVE_L && Bat_swing == 1 && (flag_over_l_edge == 1 || flag_over_r_edge == 1)){
    // if(BallType == CURVE_L && Bat_swing == 1 && (ball.x < 0 || ball.x > 1200)){
    //     printf("strike 5\n");
    //     flag_over_l_edge = 0;
    //     flag_over_r_edge = 0;
    //     s_count++;
    // }
    
    // if(BallType == ACCELERATE && Bat_swing == 0 && ball.y > 800){
    //     printf("strike 6\n");
    //     flag_over_l_edge = 0;
    //     flag_over_r_edge = 0;
    //     s_count++;
    // }
    
    // if(s_count > 2){
    //     s_count = 0;
    //     o_count++;
    // }
    // if(b_count > 3){
    //     b_count = 0;
    // }
    //-------------------------- 

    switch (flg_ball_pattern)
    {
    case STRAIGHT:
        if (ball_state == 1)
        {
            ball.yp = 15;
            ball_state = 2;
            BallType = STRAIGHT;
        }
        break;
    case ZIGZAG:
        if (ball_state == 1)
        {
            ball.xp = 50;
            ball.yp = 10;
            ball_state = 2;
            BallType = ZIGZAG;
        }
        if (ball.x <= 300 || ball.x >= 900)
            ball.xp *= -1;
        break;
    case DISAPPEAR:
        if (ball_state == 1)
        {
            ball.yp = 15;
            ball_state = 2;
            Mix_PlayChannel(1,makyu,0);
            Mix_Volume(1,MIX_MAX_VOLUME);
            BallType = DISAPPEAR;
        }
        if (flg_hit == 0 && ball.y >= 350 && ball_state == 2)
            flg_erase_ball = 1;
        break;
    case CURVE_R:
        if(ball_state == 1)
        {
            Mix_PlayChannel(1,curve1,1);
            Mix_Volume(1,MIX_MAX_VOLUME);
            ball.yp = 15;
            ball_state = 2;
            BallType = CURVE_R;
        }
        if(ball_state == 2)
        {
            ball.xp += 4; 
        }
        if(ball_state == 3)
        {
            ball.xp -= 4; 
            if (ball.xp < 0) {
                flag_over_l_edge = 1;
            }
        }
        if(ball.y >= 225 && flg_hit == 0){
            ball_state = 3;
        }
        break;
    
    case CURVE_L:
        if(ball_state == 1)
        {
            Mix_PlayChannel(1,curve1,1);
             Mix_Volume(1,MIX_MAX_VOLUME);
            ball.yp = 15;
            ball_state = 2;
            BallType = CURVE_L;
        }
        if(ball_state == 2)
        {
            ball.xp -= 4; 
        }
        if(ball_state == 3)
        {
            ball.xp += 4; 
            if (ball.xp > 1200) {
                flag_over_r_edge = 1;
            }
        }
        if(ball.y >= 225 && flg_hit == 0){
            ball_state = 3;
        }
        break;
    case ACCELERATE:
        if(ball_state == 1)
        {
            ball.yp = 10;
            ball_state = 2;
            BallType = ACCELERATE;
        }
        if(ball_state == 2)
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
    c_id = clientID;

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
                    if (jc.button.btn.A == 1 && ball_state == 0) {
                        SendBallType(STRAIGHT);
                        printf("選択された球種: ストレート\n");
                    }
                    if (jc.button.btn.B == 1 && ball_state == 0) {
                        SendBallType(ZIGZAG);
                        printf("選択された球種: ジグザグ\n");
                    }
                    if(jc.button.btn.Y == 1 && ball_state == 0){
                        SendBallType(DISAPPEAR);
                        printf("選択された球種: 消えるストレート\n");
                    }
                    if(jc.button.btn.X == 1 && ball_state == 0){
                        SendBallType(CURVE_R);
                        printf("選択された球種: 右カーブ\n");
                    }
                    if(jc.button.btn.R == 1 && ball_state == 0){
                        SendBallType(CURVE_L);
                        printf("選択された球種: 左カーブ\n");
                    }
                    if(jc.button.btn.ZR == 1 && ball_state == 0){
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

void Base_present()
{
    // 一塁座標
    SDL_Rect dst_rect1 = { base_show_x+65*sc, base_show_y+35*sc, base_w, base_h };
    // 二塁座標
    SDL_Rect dst_rect2 = { base_show_x+base_show_w/2-base_w/2, base_show_y+10*sc, base_w, base_h };
    // 三塁座標
    SDL_Rect dst_rect3 = { base_show_x+5*sc, base_show_y+35*sc, base_w, base_h };
    
    boxColor(gMainRenderer, base_show_x-sc, base_show_y-sc, base_show_x+base_show_w+sc, base_show_y+base_show_h+sc, 0xff000000);
    boxColor(gMainRenderer, base_show_x, base_show_y, base_show_x+base_show_w, base_show_y+base_show_h, 0xffffffff);

    SDL_RenderCopy(gMainRenderer, base_out, NULL, &dst_rect1);
    SDL_RenderCopy(gMainRenderer, base_out, NULL, &dst_rect2);
    SDL_RenderCopy(gMainRenderer, base_out, NULL, &dst_rect3);
    
    /*for ( int j=0; j<=runner ; j++) {
        if (base[j] == 1) {
            SDL_RenderCopy(gMainRenderer, base_in, NULL, &dst_rect1);
        }
        if (base[j] == 2) {
            SDL_RenderCopy(gMainRenderer, base_in, NULL, &dst_rect2);
        }
        if (base[j] == 3) {
            SDL_RenderCopy(gMainRenderer, base_in, NULL, &dst_rect3);
        }
    }*/

    if(runners_recv.first){
        SDL_RenderCopy(gMainRenderer, base_in, NULL, &dst_rect1);
    }
    if(runners_recv.second){
        SDL_RenderCopy(gMainRenderer, base_in, NULL, &dst_rect2);
    }
    if(runners_recv.third){
        SDL_RenderCopy(gMainRenderer, base_in, NULL, &dst_rect3);
    }

}

void Count_present()
{
    // アルファベット座標
    SDL_Rect dst_tex_B = { count_x+7*sc, count_y+6*sc, 15*sc, 19*sc };
    SDL_Rect dst_tex_S = { count_x+7*sc, count_y+31*sc, 15*sc, 19*sc };
    SDL_Rect dst_tex_O = { count_x+6*sc, count_y+56*sc, 17*sc, 19*sc };
    // ボール
    SDL_Rect dst_B1 = { count_x+30*sc, count_y+7*sc, light_s, light_s };
    SDL_Rect dst_B2 = { count_x+53*sc, count_y+7*sc, light_s, light_s };
    SDL_Rect dst_B3 = { count_x+76*sc, count_y+7*sc, light_s, light_s };
    // ストライク
    SDL_Rect dst_S1 = { count_x+30*sc, count_y+32*sc, light_s, light_s };
    SDL_Rect dst_S2 = { count_x+53*sc, count_y+32*sc, light_s, light_s };
    // アウト
    SDL_Rect dst_O1 = { count_x+30*sc, count_y+57*sc, light_s, light_s };
    SDL_Rect dst_O2 = { count_x+53*sc, count_y+57*sc, light_s, light_s };
    
    // ボールカウント囲い
    boxColor(gMainRenderer, count_x, count_y, count_x+count_w, count_y+count_h, 0xff000000);
    	
    SDL_RenderCopy(gMainRenderer, B, NULL, &dst_tex_B);
    SDL_RenderCopy(gMainRenderer, S, NULL, &dst_tex_S);
    SDL_RenderCopy(gMainRenderer, O, NULL, &dst_tex_O);
    // ボール

    switch(count.ball % 4) {
        case 0:
            SDL_RenderCopy(gMainRenderer, texture_g2, NULL, &dst_B1);
            SDL_RenderCopy(gMainRenderer, texture_g2, NULL, &dst_B2);
            SDL_RenderCopy(gMainRenderer, texture_g2, NULL, &dst_B3);
            break;

        case 1:
            SDL_RenderCopy(gMainRenderer, texture_g1, NULL, &dst_B1);
            SDL_RenderCopy(gMainRenderer, texture_g2, NULL, &dst_B2);
            SDL_RenderCopy(gMainRenderer, texture_g2, NULL, &dst_B3);
            break;

        case 2:
            SDL_RenderCopy(gMainRenderer, texture_g1, NULL, &dst_B1);
            SDL_RenderCopy(gMainRenderer, texture_g1, NULL, &dst_B2);
            SDL_RenderCopy(gMainRenderer, texture_g2, NULL, &dst_B3);
            break;

        case 3:
            SDL_RenderCopy(gMainRenderer, texture_g1, NULL, &dst_B1);
            SDL_RenderCopy(gMainRenderer, texture_g1, NULL, &dst_B2);
            SDL_RenderCopy(gMainRenderer, texture_g1, NULL, &dst_B3);
            break;
    }
    // if ( b_count == 0 ){
    //     SDL_RenderCopy(gMainRenderer, texture_g2, NULL, &dst_B1);
    //     SDL_RenderCopy(gMainRenderer, texture_g2, NULL, &dst_B2);
    //     SDL_RenderCopy(gMainRenderer, texture_g2, NULL, &dst_B3);
    // }
    // if ( b_count == 1 ){
    //     SDL_RenderCopy(gMainRenderer, texture_g1, NULL, &dst_B1);
    //     SDL_RenderCopy(gMainRenderer, texture_g2, NULL, &dst_B2);
    //     SDL_RenderCopy(gMainRenderer, texture_g2, NULL, &dst_B3);
    // }
    // if ( b_count == 2 ){
    //     SDL_RenderCopy(gMainRenderer, texture_g1, NULL, &dst_B1);
    //     SDL_RenderCopy(gMainRenderer, texture_g1, NULL, &dst_B2);
    //     SDL_RenderCopy(gMainRenderer, texture_g2, NULL, &dst_B3);
    // }
    // if ( b_count == 3 ){
    //     SDL_RenderCopy(gMainRenderer, texture_g1, NULL, &dst_B1);
    //     SDL_RenderCopy(gMainRenderer, texture_g1, NULL, &dst_B2);
    //     SDL_RenderCopy(gMainRenderer, texture_g1, NULL, &dst_B3);
    // }
    // ストライク
    if ( count.strike == 0 ){
        SDL_RenderCopy(gMainRenderer, texture_y2, NULL, &dst_S1);
        SDL_RenderCopy(gMainRenderer, texture_y2, NULL, &dst_S2);
    }
    if ( count.strike == 1 ){
        SDL_RenderCopy(gMainRenderer, texture_y1, NULL, &dst_S1);
        SDL_RenderCopy(gMainRenderer, texture_y2, NULL, &dst_S2);
    }
    if ( count.strike == 2 ){
        SDL_RenderCopy(gMainRenderer, texture_y1, NULL, &dst_S1);
        SDL_RenderCopy(gMainRenderer, texture_y1, NULL, &dst_S2);
    }
    // アウト
    if ( count.out == 0 ){
        SDL_RenderCopy(gMainRenderer, texture_r, NULL, &dst_O1);
        SDL_RenderCopy(gMainRenderer, texture_r, NULL, &dst_O2);
    }
    if ( count.out == 1 ){
        SDL_RenderCopy(gMainRenderer, texture_r2, NULL, &dst_O1);
        SDL_RenderCopy(gMainRenderer, texture_r, NULL, &dst_O2);
    }
    if ( count.out == 2 ){
        SDL_RenderCopy(gMainRenderer, texture_r2, NULL, &dst_O1);
        SDL_RenderCopy(gMainRenderer, texture_r2, NULL, &dst_O2);
    }
}

void Text(int i)
{
    SDL_Rect tex = { 370, 300, 470, 200 };
    
    if( i==1 ){
        SDL_RenderCopy(gMainRenderer, HIT1, NULL, &tex);
    }
    if( i==2 ){
        SDL_RenderCopy(gMainRenderer, HIT2, NULL, &tex);
    }
    if( i==4 ){
        SDL_RenderCopy(gMainRenderer, HOME, NULL, &tex);
    }
    
    if( i==5 ){
        SDL_RenderCopy(gMainRenderer, strike, NULL, &tex);
    }
    if( i==6 ){
        SDL_RenderCopy(gMainRenderer, ba, NULL, &tex);
    }
    if( i==7 ){
        SDL_RenderCopy(gMainRenderer, out, NULL, &tex);
    }
    if( i==10 ){
        SDL_RenderCopy(gMainRenderer, win, NULL, &tex);
    }
    if( i==11 ){
        SDL_RenderCopy(gMainRenderer, lose, NULL, &tex);
    }
    if( i==0 ){
        SDL_RenderCopy(gMainRenderer, play, NULL, &tex);
    }
}