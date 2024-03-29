/*****************************************************************
ファイル名	: server_command.c
機能		: サーバーのコマンド処理
*****************************************************************/

#include "server_common.h"
#include "server_func.h"


static void SetIntData2DataBlock(void *data, int intData, int *dataSize);
static void SetCharData2DataBlock(void *data, char charData, int *dataSize);
static int GetRandomInt(int n);


//バットが打ち返せる四角の枠の値
static ball_param ball = {600, 100, 10, 0, 0};

static SDL_Rect rect_bat = {475, 525, 250, 150};
static SDL_Rect rect_bat2 = {550, 575, 100, 50};
static SDL_Rect rect_bat3 = {600, 600, 20, 20};

static SDL_Point pos_ball;
SDL_Rect strike_zone = {450, 500, 300, 200};

SDL_Rect windowSize = {0, 0, 1200, 800};
SDL_Point ballPlace = {0, 0};

runners_list runners = {false, false, false, false};


int judge_swing(int flg_swing, SDL_Point pos_ball, SDL_Rect rect_bat) {
    if (flg_swing && SDL_PointInRect(&pos_ball, &rect_bat))
        return 1;
    else
        return 0;
}

int judge_strike(int flg_swing, ball_param ball, SDL_Rect rect_bat) {
    if (ball.y == WINDOW_Y && (ball.x >= (strike_zone.x) || ball.x <= (strike_zone.x + strike_zone.w)))
        return 1;  // TODO&DISCUSS:ボール球の追加
    else
        return 0;
}

/*****************************************************************
関数名	: ExecuteCommand
機能	: クライアントから送られてきたコマンドを元に，
          引き数を受信し，実行する
引数	: char	command		: コマンド
          int	pos			: コマンドを送ったクライアント番号
出力	: プログラム終了コマンドが送られてきた時には0を返す．
          それ以外は1を返す
*****************************************************************/
int ExecuteCommand(char command, int pos) {
    unsigned char data[MAX_DATA];
    int dataSize, intData;
    int endFlag = 1;
    static int flg_send_count = 0;
    static int flg_reset_disappear = 0;

    static int BallType = NONE;
    static int s_count = 0, b_count = 0, o_count = 0;

    /* 引き数チェック */
    assert(0 <= pos && pos < MAX_CLIENTS);

#ifndef NDEBUG
    printf("#####\n");
    printf("ExecuteCommand()\n");
    printf("Get command %c\n", command);
#endif
    // printf("i receive %c from clients.\n", command);
    if (command != BALL_PRAM_X && command != BALL_PRAM_Y){
        printf("i receive %c from clients.\n", command);
    }
    switch (command) {
        case END_COMMAND:
            dataSize = 0;
            /* コマンドのセット */
            SetCharData2DataBlock(data, command, &dataSize);

            /* 全ユーザーに送る */
            SendData(ALL_CLIENTS, data, dataSize);

            endFlag = 0;
            break;
        //ボールの座標をクライアントからもらう
        case BALL_PRAM_X:
            /* クライアント番号とIntデータを受信する */
            RecvIntData(pos, &intData);
            if (0 <= pos_ball.x || pos_ball.x <= 1200) {
                pos_ball.x = intData;
                ball.x = pos_ball.x;
                // 初期化命令を送信
            } else {
                flg_send_count = 0;
            }
            if(SDL_PointInRect(&pos_ball, &rect_bat)){
                flg_send_count = 0;
            }
            break;
        case BALL_PRAM_Y:
            /* クライアント番号とIntデータを受信する */
            RecvIntData(pos, &intData);
            // printf("pos_ball.y: %d\n", pos_ball.y);
            if (0 <= pos_ball.y || pos_ball.y <= 800) {
                pos_ball.y = intData;
                ball.y = pos_ball.y;
            } else {
                flg_send_count = 0;
            }

            if(SDL_PointInRect(&pos_ball, &rect_bat)){
                flg_send_count = 0;
            }

            //ストライクでません　
            /*if (judge_strike(1, ball, rect_bat)) {  // ストライク判定(仮)
                printf("ストライク\n");
                //Mix_PlayChannel(1,catch,0);
                // Mix_Volume(1,MIX_MAX_VOLUME/5);   
            }
            */
            /*if (!SDL_PointInRect(&pos_ball, &windowSize)) {
                // 初期化命令を送信
                // コマンドのセット 
                //SetCharData2DataBlock(data, RESET, &dataSize);

                // カウント送信のフラグ初期化
                //flg_send_count = 0;

                // 全ユーザーに送る
                // SendData(ALL_CLIENTS, data, dataSize);
            }*/
            break;
        case Batter_Swing_COMMAND:
            printf("Batter Swing\n");
            /* コマンドのセット */
            SetCharData2DataBlock(data, command, &dataSize);

            /* 全ユーザーに送る */
            SendData(ALL_CLIENTS, data, dataSize);
            Bat_swing = 1;
            break;
        case JUDGE:
            dataSize = 0;
            flg_reset_disappear = 0;
            
            if (judge_swing(1, pos_ball, rect_bat3)) {
                //printf("OK!!!\n");
                /* コマンドのセット */
                // SetCharData2DataBlock(data, JUDGE_HOMERUN, &dataSize);

                /* 全ユーザーに送る */
                // SendData(ALL_CLIENTS, data, dataSize);

                dataSize = 0;
                b_count = 0;
                s_count = 0;

                /* コマンドのセット */
                SetCharData2DataBlock(data, SCORE, &dataSize);

                /* 全ユーザーに送る */
                SendData(ALL_CLIENTS, data, dataSize);

                dataSize = 0;

                /* コマンドのセット */
                SetCharData2DataBlock(data, HOMERUN, &dataSize);

                /* データの送信 */
                SendData(ALL_CLIENTS, data, dataSize);

            }else if (judge_swing(1, pos_ball, rect_bat2)) {
                if(runners.first) {
                    runners.first = false;
                    runners.second = true;
                    runners.third = true;
                } else if(!runners.first && !runners.second && !runners.third) { // ランナーがいない場合
                    runners.second = true;
                } else {
                /* コマンドのセット */
                SetCharData2DataBlock(data, SCORE, &dataSize);

                /* 全ユーザーに送る */
                SendData(ALL_CLIENTS, data, dataSize);
                }

                b_count = 0;
                s_count = 0;

                /* コマンドのセット */
                SetCharData2DataBlock(data, TWOBASE, &dataSize);

                /* データの送信 */
                SendData(ALL_CLIENTS, data, dataSize);
                //printf("OK!!\n");
                /* コマンドのセット */
                // SetCharData2DataBlock(data, JUDGE_TWOBASE, &dataSize);


                /* 全ユーザーに送る */
                // SendData(ALL_CLIENTS, data, dataSize);
            }else if (judge_swing(1, pos_ball, rect_bat)) {
                if(runners.third) {
                    /* コマンドのセット */
                    SetCharData2DataBlock(data, SCORE, &dataSize);

                    /* 全ユーザーに送る */
                    SendData(ALL_CLIENTS, data, dataSize);
                } else if(runners.second){
                    runners.third = true;
                    runners.second = false;
                    if(runners.first = true) {
                        runners.second = true;
                    }
                } else if(runners.first) {
                    runners.second = true;
                } else if(!runners.first && !runners.second && !runners.third){
                    runners.first = true;
                } else {
                /* コマンドのセット */
                SetCharData2DataBlock(data, SCORE, &dataSize);

                /* 全ユーザーに送る */
                SendData(ALL_CLIENTS, data, dataSize);
                }


                b_count = 0;
                s_count = 0;

                /* コマンドのセット */
                SetCharData2DataBlock(data, HIT, &dataSize);

                /* データの送信 */
                SendData(ALL_CLIENTS, data, dataSize);

                //printf("OK!\n");
                /* コマンドのセット */
                //SetCharData2DataBlock(data, JUDGE_HIT, &dataSize);

                /* 全ユーザーに送る */
                //SendData(ALL_CLIENTS, data, dataSize);
            }

            // "1塁のランナーを送信するという合図"を送信する
            SetCharData2DataBlock(data, FIRST, &dataSize);
            SendData(ALL_CLIENTS, data, dataSize);  

            //　1塁のランナーをセット
            SetIntData2DataBlock(data, runners.first, &dataSize);
            SendData(ALL_CLIENTS, data, dataSize);

            // "2塁のランナーを送信するという合図"を送信する
            SetCharData2DataBlock(data, SECOND, &dataSize);
            SendData(ALL_CLIENTS, data, dataSize);  
            //　2塁のランナーの数をセット
            SetIntData2DataBlock(data, runners.second, &dataSize);
            SendData(ALL_CLIENTS, data, dataSize); 

            // "3塁のランナーを送信するという合図"を送信する
            SetCharData2DataBlock(data, THIRD, &dataSize);
            SendData(ALL_CLIENTS, data, dataSize);
            //　3塁のランナーの数をセット
            SetIntData2DataBlock(data, runners.third, &dataSize);
            SendData(ALL_CLIENTS, data, dataSize); 

            // "ボールを送信するという合図"を送信する
            SetCharData2DataBlock(data, BALL, &dataSize);
            SendData(ALL_CLIENTS, data, dataSize);  

            //ボールの数をセット
            SetIntData2DataBlock(data, b_count, &dataSize);
            SendData(ALL_CLIENTS, data, dataSize);
            printf("ball: %d\n", b_count);

            
            // "ストライクを送信するという合図"を送信する
            SetCharData2DataBlock(data, STRIKE, &dataSize);
            SendData(ALL_CLIENTS, data, dataSize);  
            //ストライクの数をセット
            SetIntData2DataBlock(data, s_count, &dataSize);
            SendData(ALL_CLIENTS, data, dataSize); 
            printf("strike: %d\n", s_count);
            

            // "アウトを送信するという合図"を送信する
            SetCharData2DataBlock(data, OUT, &dataSize);
            SendData(ALL_CLIENTS, data, dataSize);  
            //アウトの数をセット
            SetIntData2DataBlock(data, o_count, &dataSize);
            SendData(ALL_CLIENTS, data, dataSize); 
            printf("out: %d\n", o_count);
            printf("\n");

            // 初期化命令を送信
            //コマンドのセット 
            //SetCharData2DataBlock(data, RESET, &dataSize);

            // カウント送信のフラグ初期化
            flg_send_count = 1;

            //全ユーザーに送る
            //SendData(ALL_CLIENTS, data, dataSize);
            
            break;
        case PITI:
            dataSize = 0;
            
            //数値のセット
            SetCharData2DataBlock(data, command, &dataSize);
            /* 全ユーザーに送る */
            SendData(ALL_CLIENTS, data, dataSize);
            break;
        case PITI_2:
            dataSize = 0;

            //数値のセット
            SetCharData2DataBlock(data, command, &dataSize);
            /* 全ユーザーに送る */
            SendData(ALL_CLIENTS, data, dataSize);
            break;
        case PITI_3:
            dataSize = 0;

            //数値のセット
            SetCharData2DataBlock(data, command, &dataSize);
            /* 全ユーザーに送る */
            SendData(ALL_CLIENTS, data, dataSize);
            break;
        case STRAIGHT:

            BallType = STRAIGHT;
            dataSize = 0;

            //数値のセット
            SetCharData2DataBlock(data, command, &dataSize);
            /* 全ユーザーに送る */
            SendData(ALL_CLIENTS, data, dataSize);      
            // printf("server:1\n");      
            break;
        case ZIGZAG:
            BallType = ZIGZAG;
            dataSize = 0;

            //数値のセット
            SetCharData2DataBlock(data, command, &dataSize);
            /* 全ユーザーに送る */
            SendData(ALL_CLIENTS, data, dataSize);    
            break;  
        case DISAPPEAR:
            BallType = DISAPPEAR;
            dataSize = 0;

            //数値のセット
            SetCharData2DataBlock(data, command, &dataSize);
            /* 全ユーザーに送る */
            SendData(ALL_CLIENTS, data, dataSize);    
            break; 
        case CURVE_R:
            BallType = CURVE_R;
            dataSize = 0;

            //数値のセット
            SetCharData2DataBlock(data, command, &dataSize);
            /* 全ユーザーに送る */
            SendData(ALL_CLIENTS, data, dataSize);    
            break;  
        case CURVE_L:
            BallType = CURVE_L;
            dataSize = 0;

            //数値のセット
            SetCharData2DataBlock(data, command, &dataSize);
            /* 全ユーザーに送る */
            SendData(ALL_CLIENTS, data, dataSize);    
            break;
        case ACCELERATE:
            BallType = ACCELERATE;
            dataSize = 0;

            //数値のセット
            SetCharData2DataBlock(data, command, &dataSize);
            /* 全ユーザーに送る */
            SendData(ALL_CLIENTS, data, dataSize);    
            break;   
        case NONE:
            BallType = NONE;
            break; 
        default:
            /* 未知のコマンドが送られてきた */
            fprintf(stderr, "0x%02x is not command!\n", command);
    }

    // if(BallType == STRAIGHT && Bat_swing == 0 && ball.y > 800){
    //     printf("strike 1\n");
    //     s_count++;
    // }
    // if(BallType == ZIGZAG && Bat_swing == 0 && ball.y > 800){
    //     printf("strike 2\n");
    //     s_count++;
    // }

    // if(BallType == ACCELERATE && Bat_swing == 0 && ball.y > 800){
    //     printf("strike 6\n");
    //     s_count++;
    // }
    //printf("flg value is %d, BallType is %d\n", flg_send_count, BallType);

    if(!SDL_PointInRect(&pos_ball, &windowSize) && flg_send_count == 0) {
        printf("inside PointInRect\n");

        /*if(!Batter_key) {
            // 初期化命令を送信
            //コマンドのセット 
            SetCharData2DataBlock(data, RESET, &dataSize);

            // カウント送信のフラグ初期化
            flg_send_count = 0;

            //全ユーザーに送る
            SendData(ALL_CLIENTS, data, dataSize);

        }
        else{
            printf("aiueo\n");
        }*/
        printf("ball type: %d\n", BallType);


        if( (pos_ball.y >= 800)){
            printf("BallType: %d\n", BallType);
            switch (BallType) {
                case CURVE_R:
                    if(!Bat_swing) {
                        b_count++;
                    } else {
                        s_count++; // not swing -> ball, swing -> strike
                    }
                    break;
                
                case CURVE_L:
                    if(!Bat_swing) {
                        b_count++;
                    } else {
                        s_count++; // not swing -> ball, swing -> strike
                    }
                    break;
                case DISAPPEAR:
                    printf("Bat_swing: %d\n", Bat_swing);
                    if(!Bat_swing) {
                        b_count++;
                    } else {
                    s_count++; // not swing -> ball, swing -> strike
                    }
                    break;
                
                case STRAIGHT:
                    printf("strike\n");
                    s_count++;
                    break;

                case ZIGZAG:
                    printf("zigzag\n");
                    s_count++;
                    break;
                    
                case 6:
                    printf("accelerate\n");
                    s_count++;
                    break;

                }
            }
 
        BallType = NONE;


        if(s_count > 2){
            b_count = 0;
            s_count = 0;
            o_count++;
            /* コマンドのセット */
            SetCharData2DataBlock(data, INCREASE_OUT, &dataSize);

            /* 全ユーザーに送る */
            SendData(ALL_CLIENTS, data, dataSize);
        }
        if(b_count > 3){
            b_count = 0;
            s_count = 0;
            if (runners.third && !runners.second && !runners.first) {
                runners.first = true;
                // printf("a\n");
            }
            else if (!runners.third && runners.second && !runners.first) {
                runners.first = true;
                // printf("b\n");
            }
            else if(!runners.third && !runners.second && runners.first) {
                runners.second = true;
                // printf("c\n");
            }
            else if(runners.third && !runners.second && runners.first) {
                runners.second = true;
                // printf("d\n");
            }
            else if(runners.third && runners.second && !runners.first) {
                runners.first = true;
                // printf("e\n");
                
            }
            else if(!runners.third && runners.second && runners.first) {
                runners.third = true;
                // printf("f\n");
            }
            else if(runners.third && runners.second && runners.first) {
                // printf("g\n");
                dataSize = 0;

                /* コマンドのセット */
                SetCharData2DataBlock(data, SCORE, &dataSize);

                /* 全ユーザーに送る */
                SendData(ALL_CLIENTS, data, dataSize);
            }
            else {
                runners.first = true;
                // printf("h\n");
            }

            // "1塁のランナーを送信するという合図"を送信する
            SetCharData2DataBlock(data, FIRST, &dataSize);
            SendData(ALL_CLIENTS, data, dataSize);  

            //　1塁のランナーをセット
            SetIntData2DataBlock(data, runners.first, &dataSize);
            SendData(ALL_CLIENTS, data, dataSize);

            // "2塁のランナーを送信するという合図"を送信する
            SetCharData2DataBlock(data, SECOND, &dataSize);
            SendData(ALL_CLIENTS, data, dataSize);  
            //　2塁のランナーの数をセット
            SetIntData2DataBlock(data, runners.second, &dataSize);
            SendData(ALL_CLIENTS, data, dataSize); 

            // "3塁のランナーを送信するという合図"を送信する
            SetCharData2DataBlock(data, THIRD, &dataSize);
            SendData(ALL_CLIENTS, data, dataSize);
            //　3塁のランナーの数をセット
            SetIntData2DataBlock(data, runners.third, &dataSize);
            SendData(ALL_CLIENTS, data, dataSize); 
        }

        // "ボールを送信するという合図"を送信する
        SetCharData2DataBlock(data, BALL, &dataSize);
        SendData(ALL_CLIENTS, data, dataSize);  

        //ボールの数をセット
        SetIntData2DataBlock(data, b_count, &dataSize);
        SendData(ALL_CLIENTS, data, dataSize);
        printf("ball: %d\n", b_count);

        
        // "ストライクを送信するという合図"を送信する
        SetCharData2DataBlock(data, STRIKE, &dataSize);
        SendData(ALL_CLIENTS, data, dataSize);
        
        //ストライクの数をセット
        SetIntData2DataBlock(data, s_count, &dataSize);
        SendData(ALL_CLIENTS, data, dataSize); 
        printf("strike: %d\n", s_count);
        

        // "アウトを送信するという合図"を送信する
        SetCharData2DataBlock(data, OUT, &dataSize);
        SendData(ALL_CLIENTS, data, dataSize);  
        //アウトの数をセット
        SetIntData2DataBlock(data, o_count, &dataSize);
        SendData(ALL_CLIENTS, data, dataSize); 
        printf("out: %d\n", o_count);
        printf("\n");

        flg_send_count = 1;
    
        // 初期化命令を送信
        //コマンドのセット 
        SetCharData2DataBlock(data, RESET, &dataSize);
        
        //全ユーザーに送る
        SendData(ALL_CLIENTS, data, dataSize);
    }
    if(pos_ball.y > 800 && flg_reset_disappear == 0){
        s_count++;
        printf("ball_y: %d, flg_reset: %d\n", pos_ball.y, flg_reset_disappear);

        if(s_count > 2){
            b_count = 0;
            s_count = 0;
            o_count++;
            /* コマンドのセット */
            SetCharData2DataBlock(data, INCREASE_OUT, &dataSize);

            /* 全ユーザーに送る */
            SendData(ALL_CLIENTS, data, dataSize);
        }
        // "ストライクを送信するという合図"を送信する
        SetCharData2DataBlock(data, STRIKE, &dataSize);
        SendData(ALL_CLIENTS, data, dataSize);
        
        //ストライクの数をセット
        SetIntData2DataBlock(data, s_count, &dataSize);
        SendData(ALL_CLIENTS, data, dataSize); 
        printf("strike in reset: %c\n", s_count);

        // "アウトを送信するという合図"を送信する
        SetCharData2DataBlock(data, OUT, &dataSize);
        SendData(ALL_CLIENTS, data, dataSize);  
        //アウトの数をセット
        SetIntData2DataBlock(data, o_count, &dataSize);
        SendData(ALL_CLIENTS, data, dataSize); 
        printf("out: %d\n", o_count);

        // 初期化命令を送信
        //コマンドのセット 
        SetCharData2DataBlock(data, RESET, &dataSize);
    
        //全ユーザーに送る
        SendData(ALL_CLIENTS, data, dataSize);

        flg_reset_disappear = 1;
    }

    // // Judge Disapper Ball
    // if(BallType == DISAPPEAR && !SDL_PointInRect(&pos_ball, &windowSize)) {
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
    // }
    // //if(BallType == CURVE_L && Bat_swing == 0 && (flag_over_l_edge == 1 || flag_over_r_edge == 1)){
    // if(BallType == CURVE_L && Bat_swing == 0 && (ball.x < 0 || ball.x > 1200)){
    //     printf("ball 3\n");
    //     b_count++;
    // }
    // //if(BallType == CURVE_L && Bat_swing == 1 && (flag_over_l_edge == 1 || flag_over_r_edge == 1)){
    // if(BallType == CURVE_L && Bat_swing == 1 && (ball.x < 0 || ball.x > 1200)){
    //     printf("strike 5\n");
    //     s_count++;
    // }
    
    return endFlag;
}

/*****
static
*****/
/*****************************************************************
関数名	: SetIntData2DataBlock
機能	: int 型のデータを送信用データの最後にセットする
引数	: void		*data		: 送信用データ
          int		intData		: セットするデータ
          int		*dataSize	: 送信用データの現在のサイズ
出力	: なし
*****************************************************************/
static void SetIntData2DataBlock(void *data, int intData, int *dataSize) {
    int tmp;

    /* 引き数チェック */
    assert(data != NULL);
    assert(0 <= (*dataSize));

    tmp = htonl(intData);

    /* int 型のデータを送信用データの最後にコピーする */
    memcpy(data + (*dataSize), &tmp, sizeof(int));
    /* データサイズを増やす */
    (*dataSize) += sizeof(int);
}

/*****************************************************************
関数名	: SetCharData2DataBlock
機能	: char 型のデータを送信用データの最後にセットする
引数	: void		*data		: 送信用データ
          int		intData		: セットするデータ
          int		*dataSize	: 送信用データの現在のサイズ
出力	: なし
*****************************************************************/
static void SetCharData2DataBlock(void *data, char charData, int *dataSize) {
    /* 引き数チェック */
    assert(data != NULL);
    assert(0 <= (*dataSize));

    /* int 型のデータを送信用データの最後にコピーする */
    *(char *)(data + (*dataSize)) = charData;
    /* データサイズを増やす */
    (*dataSize) += sizeof(char);
}

/*****************************************************************
関数名	: GetRandomInt
機能	: 整数の乱数を得る
引数	: int		n	: 乱数の最大値
出力	: 乱数値
*****************************************************************/
static int GetRandomInt(int n) {
    return rand() % n;
}
