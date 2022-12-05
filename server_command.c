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
static SDL_Rect rect_bat = {450, 500, 300, 200};
static SDL_Point pos_ball;
SDL_Rect strike_zone = {350, 300, 100, 100};

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

    /* 引き数チェック */
    assert(0 <= pos && pos < MAX_CLIENTS);

#ifndef NDEBUG
    printf("#####\n");
    printf("ExecuteCommand()\n");
    printf("Get command %c\n", command);
#endif
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
            pos_ball.x = intData;
            ball.x = pos_ball.x;
            break;
        case BALL_PRAM_Y:
            /* クライアント番号とIntデータを受信する */
            RecvIntData(pos, &intData);
            pos_ball.y = intData;
            ball.y = pos_ball.y;
            //ストライクでません　
            if (judge_strike(1, ball, rect_bat)) {  // ストライク判定(仮)
                printf("ストライク\n");
            }
            break;
        case Batter_Swing_COMMAND:
            dataSize = 0;
            printf("Batter Swing");

            if (judge_swing(1, pos_ball, rect_bat)) {
                printf("OK!");
                /* コマンドのセット */
                SetCharData2DataBlock(data, command, &dataSize);

                //数値のセット
                SetIntData2DataBlock(data, -10, &dataSize);
                /* 全ユーザーに送る */
                SendData(ALL_CLIENTS, data, dataSize);
            }
            break;
        case PITI:
            dataSize = 0;

            //数値のセット
            SetCharData2DataBlock(data, command, &dataSize);
            /* 全ユーザーに送る */
            SendData(ALL_CLIENTS, data, dataSize);

            break;
        default:
            /* 未知のコマンドが送られてきた */
            fprintf(stderr, "0x%02x is not command!\n", command);
    }
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
