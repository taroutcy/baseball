/*****************************************************************
ファイル名	: client_main.c
機能		: クライアントのメインルーチン
*****************************************************************/

#include "client_func.h"
#include "common.h"

int main(int argc, char *argv[]) {
    int num;
    char name[MAX_CLIENTS][MAX_NAME_SIZE];
    int endFlag = 1;
    char localHostName[] = "localhost";
    char *serverName;
    int clientID;

    /* 引き数チェック */
    if (argc == 1) {
        serverName = localHostName;
    } else if (argc == 2) {
        serverName = argv[1];
    } else {
        fprintf(stderr, "Usage: %s, Cannot find a Server Name.\n", argv[0]);
        return -1;
    }

    /* サーバーとの接続 */
    if (SetUpClient(serverName, &clientID, &num, name) == -1) {
        fprintf(stderr, "setup failed : SetUpClient\n");
        return -1;
    }
    /* ウインドウの初期化 */
    if (InitWindows(clientID, num, name) == -1) {
        fprintf(stderr, "setup failed : InitWindows\n");
        return -1;
    }

    /*// 球種選択(仮)
    if(clientID == 1)
    {
        while (1)
        {
            //　変更箇所
            printf("球種を選択してください. ストレート: 1, ジグザグ: 2, 消える魔球: 3, カーブ(右): 4, カーブ(左): 5, 加速球: 6\n");
            scanf("%d", &flg_ball_pattern);
            if (flg_ball_pattern)
            {
                flg_select_ball = 1;
                break;
            }
        }
    }*/

    /* タイマー設定 */
    my_timer_id_bat = SDL_AddTimer(40, draw_timer_bat, (void *)gMainRenderer);
    my_timer_id = SDL_AddTimer(40, draw_timer, (void *)gMainRenderer);

    /* メインイベントループ */
    while (endFlag) {
        WindowEvent(num, clientID);
        endFlag = SendRecvManager();
    };

    /* 終了処理 */
    DestroyWindow();
    CloseSoc();

    return 0;
}
