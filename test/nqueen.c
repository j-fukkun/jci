//#include <stdio.h>
//#include <time.h>

/* チェスの盤の縦横サイズ */
//#define N 8

/* クイーンがあることを示す定数 */
//#define QUEEN 1

/* クイーンがないことを示す定数 */
//#define EMPTY 0

//#define OK 1
//#define NG 0

//void printQueen(int a);
//int check(int a, int b, int c, int d, int e);
//int checkQueen(int a);
//void nQueen(int a, int b, int c);
//void initQueen(int a);

/* チェスの盤 */
int board[8][8];

/* 見つけた解の数 */
int resolve = 0;

/**
 * チェス盤の表示
 * n：１方向のマスの数
 */
void printQueen(int n)
{
  int i;
  int j;

    printf("%d個目の解\n", resolve);

    /* 全マスを表示 */
    for (j = 0; j < n; j++)
    {
        for (i = 0; i < n; i++)
        {
            if (board[j][i] == 1)
            {
                /* (i, j)マスにクイーンがある場合 */
                printf("Q");
            }
            else
            {
                /* (i, j)マスにクイーンが無い場合 */
                printf("*");
            }
        }
        printf("\n");
    }
    printf("\n\n");
    return;
}

/**
 * 指定された方向に他のクイーンが無いかどうかを判断
 * n：１方向のマスの数
 * i：クイーンの位置の列番号
 * j：クイーンの位置の行番号
 * di：列番号の増加量
 * dj：行番号の増加量
 */
int check(int n, int i, int j, int di, int dj)
{
    int k;
    int ii;
    int jj;

    for (k = 1; k < n; k++)
    {
        /* (di, dj)方向にkマスを進める */
        ii = i + di * k;
        jj = j + dj * k;
        if (ii >= n || ii < 0 || jj >= n || jj < 0)
        {
            /* 盤外までチェックしたらその方向にクイーンはない */
            break;
        }

        if (board[j + dj * k][i + di * k] == 1)
        {
            /* マス上にクイーンがあればNGを返却 */
            return 0;
        }
    }

    /* その方向にクイーンが無いのでOKを返却 */
    return 1;
}

/**
 * Nクイーン問題を満たしているかどうかを判断
 * n：１方向のマスの数
 */
int checkQueen(int n)
{
  int i;
  int j;

    /* クイーンがあるマスを探索 */
    for (j = 0; j < n; j++)
    {
        for (i = 0; i < n; i++)
        {
            if (board[j][i] == 1)
            {
                /* クイーンのあるマスから縦横斜め方向にクイーンがあるかどうかをチェック */

                /* 左方向をチェック */
                if (!check(n, i, j, -1, 0))
                {
                    return 0;
                }
                /* 右方向をチェック */
                if (!check(n, i, j, 1, 0))
                {
                    return 0;
                }
                /* 下方向をチェック */
                if (!check(n, i, j, 0, -1))
                {
                    return 0;
                }
                /* 上方向をチェック */
                if (!check(n, i, j, 0, 1))
                {
                    return 0;
                }
                /* 左下方向をチェック */
                if (!check(n, i, j, -1, -1))
                {
                    return 0;
                }
                /* 左上方向をチェック */
                if (!check(n, i, j, -1, 1))
                {
                    return 0;
                }
                /* 右下方向をチェック */
                if (!check(n, i, j, 1, -1))
                {
                    return 0;
                }
                /* 右上方向をチェック */
                if (!check(n, i, j, 1, 1))
                {
                    return 0;
                }
            }
        }
    }

    return 1;
}

/**
 * クイーンの設置
 * n：１方向のマスの数
 * q：配置済みのクイーンの数
 * k：配置開始
 */
void nQueen(int n, int q, int k)
{
  int i;
  int j;
  int l;

    if (q == n)
    {
        /* n個クイーン設置ずみの場合 */

        if (checkQueen(n))
        {
            /* Nクイーン問題を満たしている場合 */

            /* 解の数をインクリメント */
            resolve++;

            /* チェス盤を表示 */
            printQueen(n);
        }
        return;
    }

    /* 配置位置の開始点から順にクイーンを配置していく */
    for (l = k; l < n * n; l++)
    {
        /* 配置位置より行番号と列番号を算出 */
        j = l / n;
        i = l % n;

        /* (i, j)マスにクイーンを置く */
        board[j][i] = 1;

        if (checkQueen(n))
        {
            /* クイーンを置いてもまだNクイーン問題を満たしている場合 */

            /* 次のクイーンを置く */
            nQueen(n, q + 1, l + 1);
        }
        /* (i, j)マスからクイーンを取り除く */
        board[j][i] = 0;
    }
    return;
}

/**
 * チェス盤の初期化
 * n：１方向のマスの数
 */
void initQueen(int n)
{
  int i;
  int j;
    for (j = 0; j < n; j++)
    {
        for (i = 0; i < n; i++)
        {
            /* マスを空にする */
            board[j][i] = 0;
        }
    }
    return;
}

int main(void)
{
  //clock_t start, end;

    /* チェス盤の初期化 */
    initQueen(8);

    //start = clock();

    /* Nクイーン問題を解く */
    nQueen(8, 0, 0);

    //end = clock();

    /* 処理時間を表示 */
    //printf("time = %f[s]", (double)(end - start) / CLOCKS_PER_SEC);
    printf("done\n");
    return 0;
}
