/*****************************************************************
ファイル名	: client_command.c
機能		: クライアントのコマンド処理
*****************************************************************/

#include "common.h"
#include "client_func.h"

static void SetIntData2DataBlock(void *data, int intData, int *dataSize);
static void SetCharData2DataBlock(void *data, char charData, int *dataSize);

//サーバーからデータが送られてくるまでクライアントからデータを送らないようにするための変数
int key1 = 1;
int key2 = 1;

int ball_y = 0;

ball_count count = {0, 0, 0};
runners_list runners_recv = {0, 0, 0};
/*****************************************************************
関数名	: ExecuteCommand
機能	: サーバーから送られてきたコマンドを元に，
          引き数を受信し，実行する
引数	: char	command		: コマンド
出力	: プログラム終了コマンドがおくられてきた時には0を返す．
          それ以外は1を返す
*****************************************************************/
int ExecuteCommand(char command)
{
    int endFlag = 1;
    int dataSize, intData;
#ifndef NDEBUG
    printf("#####\n");
    printf("ExecuteCommand()\n");
    printf("command = %c\n", command);
#endif
    //key変数のリセット
    key1 = 1;
    key2 = 1;
    switch (command)
    {
    case END_COMMAND:
        endFlag = 0;
        break;
    case JUDGE_HIT:
        Present(1);
        y = 1; //ボールの速度を変更
        //printf("%d", intData);
        break;
    case JUDGE_TWOBASE:
        Present(2);
        y = 1;
        break;
    case JUDGE_HOMERUN:
        Present(4);
        y = 1;
        break;
    case Batter_Swing_COMMAND:
        bat_disp = 1;
        Bat_swing = 1;
        break;
    case STRAIGHT:
        flg_ball_pattern = STRAIGHT;
        break;
    case ZIGZAG:
        flg_ball_pattern = ZIGZAG;
        break;
    case DISAPPEAR:
        flg_ball_pattern = DISAPPEAR;
        break;
    case CURVE_R:
        flg_ball_pattern = CURVE_R;
        break;
    case CURVE_L:
        flg_ball_pattern = CURVE_L;
        break;
    case ACCELERATE:
        flg_ball_pattern = ACCELERATE;
        break;
    case PITI:
        ball_state = 1;
        break;
    case RESET:
        ball_state = 0;
        Reset = 1;
        break;
    case BALL:
        RecvIntData(&count.ball);
        //printf("ball: %d\n", count.ball);
        break;
    case STRIKE:
        RecvIntData(&count.strike);
        //printf("strike: %d\n", count.strike);
        break;
    case OUT:
        RecvIntData(&count.out);
        //printf("out: %d\n", count.out);
        break;
    case FIRST:
        RecvIntData(&runners_recv.first);
        //printf("first: %d\n", runners_recv.first);
        break;
    case SECOND:
        RecvIntData(&runners_recv.second);
        //printf("second: %d\n", runners_recv.second);
        break;
    case THIRD:
        RecvIntData(&runners_recv.third);
        //printf("third: %d\n", runners_recv.third);
        break;   
    }
    return endFlag;
}

/*****************************************************************
関数名	: SendBatter_Swing
機能	: サーバーにバットを振った情報(データ)を送る
引数	: int pos	    : クライアント番号
出力	: なし
*****************************************************************/
void SendBatter_swing(void)
{
    unsigned char data[MAX_DATA];
    int dataSize;

#ifndef NDEBUG
    printf("#####\n");
    printf("SendCircleCommand()\n");
    printf("Send Circle Command to");
#endif
    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, Batter_Swing_COMMAND, &dataSize);
    //データ送信
    SendData(data, dataSize);
}

//utetakadouka hantei
void Send_JUDGE(void){
    unsigned char data[MAX_DATA];
    int dataSize;

#ifndef NDEBUG
    printf("#####\n");
    printf("SendCircleCommand()\n");
    printf("Send Circle Command to");
#endif
    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, JUDGE, &dataSize);
    //データ送信
    SendData(data, dataSize);
}


/*****************************************************************
関数名	: SendBatter_Swing
機能	: サーバーにバットを振った情報(データ)を送る
引数	: int pos	    : クライアント番号
出力	: なし
*****************************************************************/
void SendBall_x(SDL_Point pos_ball)
{
    unsigned char data[MAX_DATA];
    int dataSize;

#ifndef NDEBUG
    printf("#####\n");
    printf("SendCircleCommand()\n");
    printf("Send Circle Command to");
#endif
    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, BALL_PRAM_X, &dataSize);
    /* Int数値のボールのx座標セット */
    SetIntData2DataBlock(data, pos_ball.x, &dataSize);
    //データ送信
    SendData(data, dataSize);
}

//投手がたまを投げたという合図を送る
void SendPiti(void){
    unsigned char data[MAX_DATA];
    int dataSize;

#ifndef NDEBUG
    printf("#####\n");
    printf("SendCircleCommand()\n");
    printf("Send Circle Command to");
#endif
    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, PITI, &dataSize);
    //データ送信
    SendData(data, dataSize);
}

//合図を送る
void SendBallType(int i){
    unsigned char data[MAX_DATA];
    int dataSize;

    dataSize = 0;
    if(i == STRAIGHT){
        /* コマンドのセット */
        SetCharData2DataBlock(data, STRAIGHT, &dataSize);
        //データ送信
        SendData(data, dataSize);
    }else if(i == ZIGZAG){
        /* コマンドのセット */
        SetCharData2DataBlock(data, ZIGZAG, &dataSize);
        //データ送信
        SendData(data, dataSize);
    }else if(i == DISAPPEAR){
        /* コマンドのセット */
        SetCharData2DataBlock(data, DISAPPEAR, &dataSize);
        //データ送信
        SendData(data, dataSize);
    }else if(i == CURVE_R){
        /* コマンドのセット */
        SetCharData2DataBlock(data, CURVE_R, &dataSize);
        //データ送信
        SendData(data, dataSize);
    }else if(i == CURVE_L){
        /* コマンドのセット */
        SetCharData2DataBlock(data, CURVE_L, &dataSize);
        //データ送信
        SendData(data, dataSize);
    }else if(i == ACCELERATE){
        /* コマンドのセット */
        SetCharData2DataBlock(data, ACCELERATE, &dataSize);
        //データ送信
        SendData(data, dataSize);
    }
}


/*****************************************************************
関数名	: SendBall_y
機能	: サーバーにバットを振った情報(データ)を送る
引数	: int pos	    : クライアント番号
出力	: なし
*****************************************************************/
void SendBall_y(SDL_Point pos_ball)
{
    unsigned char data[MAX_DATA];
    int dataSize;

#ifndef NDEBUG
    printf("#####\n");
    printf("SendCircleCommand()\n");
    printf("Send Circle Command to");
#endif
    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, BALL_PRAM_Y, &dataSize);
    /* Int数値のボールのx座標セット */
    SetIntData2DataBlock(data, pos_ball.y, &dataSize);
    //データ送信
    SendData(data, dataSize);
}


/*****************************************************************
関数名	: SendEndCommand
機能	: プログラムの終了を知らせるために，
          サーバーにデータを送る
引数	: なし
出力	: なし
*****************************************************************/
void SendEndCommand(void)
{
    unsigned char data[MAX_DATA];
    int dataSize;

#ifndef NDEBUG
    printf("#####\n");
    printf("SendEndCommand()\n");
#endif
    dataSize = 0;
    /* コマンドのセット */
    SetCharData2DataBlock(data, END_COMMAND, &dataSize);

    /* データの送信 */
    SendData(data, dataSize);
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
static void SetIntData2DataBlock(void *data, int intData, int *dataSize)
{
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
static void SetCharData2DataBlock(void *data, char charData, int *dataSize)
{
    /* 引き数チェック */
    assert(data != NULL);
    assert(0 <= (*dataSize));

    /* char 型のデータを送信用データの最後にコピーする */
    *(char *)(data + (*dataSize)) = charData;
    /* データサイズを増やす */
    (*dataSize) += sizeof(char);
}
