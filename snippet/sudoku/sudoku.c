#include <stdio.h>

#if defined(__GNUC__)
#define bcount __builtin_popcount
#define boffs __builtin_ctz
#else
// 比特1的个数
static int bcount(int v)
{
    int count = 0;
    while (v)
    {
        count++;
        v &= v - 1;
    }
    return count;
}
// 第一个比特1的偏移
static int boffs(int v)
{
    int offs = 0;
    while (v)
    {
        if (v & 1) break;
        offs++;
        v >>= 1;
    }
    return offs;
}
#endif

// 十字阵列的i行j列，或包含的宫，清除掉 bit
static void sd_clsbit(int buf[9][9], int i, int j, int bit)
{
    int di = i / 3;
    int dj = j / 3;
    int ii,jj;
    for (ii = 0; ii < 9; ii++)
    {
        for (jj = 0; jj < 9; jj++)
        {
            // 忽略非候选单元
            if (buf[ii][jj] <= 0) continue;

            if (i == ii || j == jj || (di == ii / 3 && dj == jj / 3))
            {
                buf[ii][jj] &= ~(1 << bit);
            }
        }
    }
}

// 根据已知的数字，初步排除掉未知数的不可能值
static void sd_init(int buf[9][9])
{
    int i,j;
    for (i = 0; i < 9; i++)
    {
        for (j = 0; j < 9; j++)
        {
            if (buf[i][j] > 0) continue;
            int bit = -buf[i][j] - 1;
            // 约束该节点的纵、横、块
            sd_clsbit(buf, i, j, bit);
        }
    }
}

// 单一元素
static int sd_r0(int buf[9][9], char *his, int *hislen)
{
    int i,j;
    for (i = 0; i < 9; i++)
    {
        for (j = 0; j < 9; j++)
        {
            if (buf[i][j] <= 0) continue;
            // 唯一候选词
            if (bcount(buf[i][j]) == 1)
            {
                // 转化为确定数
                int bit = boffs(buf[i][j]);
                buf[i][j] = -(bit + 1);
                // 更新约束
                sd_clsbit(buf, i, j, bit);
                if (his && hislen)
                {
                    // 保存历史
                    his[*hislen] = i * 9 + j;
                    *hislen += 1;                    
                }
                return 1;
            }
        }
    }
    return 0;
}

// 数字逃逸
static int sd_r1(int buf[9][9], char *his, int *hislen)
{
    int i,j,ii,jj;
    for (i = 0; i < 9; i++)
    {
        for (j = 0; j < 9; j++)
        {
            int val = buf[i][j];
            if (val <= 0) continue;
            while (val)
            {
                // 循环中遍历每一位（每一种可能）
                int bit = boffs(val);
                int offs = 1 << bit;
                val &= ~offs;
                int md = 7; // 0b000_0000_0000_0111 底三位，表示三种匹配模式
                int di = i / 3;
                int dj = j / 3;
                for (ii = 0; ii < 9 && md; ii++)
                {
                    for (jj = 0; jj < 9 && md; jj++)
                    {
                        if (buf[ii][jj] <= 0) continue;
                        if (i == ii && j == jj) continue;

                        if ((md & 1) && di == ii / 3 && dj == jj / 3)
                        {
                            if (buf[ii][jj] & offs)
                            {
                                // 宫内不唯一
                                md -= 1;
                            }
                        }
                        if ((md & 2) && i == ii)
                        {
                            if (buf[ii][jj] & offs)
                            {
                                // 横不唯一
                                md -= 2;
                            }
                        }
                        if ((md & 4) && j == jj)
                        {
                            if (buf[ii][jj] & offs)
                            {
                                // 纵不唯一
                                md -= 4;
                            }
                        }
                    }
                }
                if (md == 0) continue;
                // 确定数字
                buf[i][j] = -(bit + 1);
                sd_clsbit(buf, i, j, bit);
                if (his && hislen)
                {
                    his[*hislen] = i * 9 + j;
                    *hislen += 1;                    
                }
                return 1;
            }
        }
    }
    return 0;
}

// 循环闭环
static int sd_r2(int buf[9][9])
{
    int i,j,ii,jj,iii,jjj;
    for (i = 0; i < 9; i++)
    {
        for (j = 0; j < 9; j++)
        {
            int val = buf[i][j];
            if (val <= 0) continue;
            int bitcount = bcount(val);
            // 行内
            int count = bitcount;
            for (ii = 0; ii < 9; ii++)
            {
                if (buf[ii][j] == val)
                {
                    if (--count == 0)
                    {
                        // 长度n数组重复至少n次
                        int empty = 1;
                        for (iii = 0; iii < 9; iii++)
                        {
                            if (buf[iii][j] > 0 && buf[iii][j] != val && (buf[iii][j] & val))
                            {
                                buf[iii][j] &= ~val;
                                empty = 0;
                            }
                        }
                        if (empty)
                        {
                            break;
                        }
                        else
                        {
                            return 1;
                        }
                    }
                }
            }
            // 列内
            count = bitcount;
            for (jj = 0; jj < 9; jj++)
            {
                if (buf[i][jj] == val)
                {
                    if (--count == 0)
                    {
                        // 长度n数组重复至少n次
                        int empty = 1;
                        for (jjj = 0; jjj < 9; jjj++)
                        {
                            if (buf[i][jjj] > 0 && buf[i][jjj] != val && (buf[i][jjj] & val))
                            {
                                buf[i][jjj] &= ~val;
                                empty = 0;
                            }
                        }
                        if (empty)
                        {
                            break;
                        }
                        else
                        {
                            return 1;
                        }
                    }
                }
            }
            // 宫内
            int di = i / 3;
            int dj = j / 3;
            int ei = di * 3 + 3;
            int ej = dj * 3 + 3;
            count = bitcount;
            for (ii = ei - 3; ii < ei; ii++)
            {
                for (jj = ej - 3; jj < ej; jj++)
                {
                    if (ii == i && jj == j) continue;
                    if (buf[ii][jj] == val)
                    {
                        if (--count == 0)
                        {
                            // 长度n数组重复至少n次
                            int empty = 1;
                            for (iii = di * 3; iii < ei; iii++)
                            {
                                for (jjj = dj * 3; jjj < ej; jjj++)
                                {
                                    if (buf[iii][jjj] > 0 && buf[iii][jjj] != val && (buf[iii][jjj] & val))
                                    {
                                        buf[iii][jjj] &= ~val;
                                        empty = 0;
                                    }
                                }
                            }
                            if (empty)
                            {
                                ii = ei;
                                break;
                            }
                            else
                            {
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

// 连珠
static int sd_r3(int buf[9][9])
{
    int i,j,z,ii,jj,o;
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            for (z = 0; z < 3; z++)
            {
                int y = i * 3;
                int x = j * 3;
                // 纵向宫内唯一，列可消除
                int val = 0;
                if (buf[y+0][x+(z+0)%3] > 0) val |=  buf[y+0][x+(z+0)%3];
                if (buf[y+1][x+(z+0)%3] > 0) val |=  buf[y+1][x+(z+0)%3];
                if (buf[y+2][x+(z+0)%3] > 0) val |=  buf[y+2][x+(z+0)%3];
                if (buf[y+0][x+(z+1)%3] > 0) val &= ~buf[y+0][x+(z+1)%3];
                if (buf[y+1][x+(z+1)%3] > 0) val &= ~buf[y+1][x+(z+1)%3];
                if (buf[y+2][x+(z+1)%3] > 0) val &= ~buf[y+2][x+(z+1)%3];
                if (buf[y+0][x+(z+2)%3] > 0) val &= ~buf[y+0][x+(z+2)%3];
                if (buf[y+1][x+(z+2)%3] > 0) val &= ~buf[y+1][x+(z+2)%3];
                if (buf[y+2][x+(z+2)%3] > 0) val &= ~buf[y+2][x+(z+2)%3];
                if (val)
                {
                    int chg = 0;
                    for (ii = 0; ii < 9; ii++)
                    {
                        if (ii / 3 == i) continue;
                        if (buf[ii][x+z] <= 0 || !(buf[ii][x+z] & val)) continue;
                        buf[ii][x+z] &= ~val;
                        chg = 1;
                    }
                    if (chg) return 1;
                }
                // 横向宫内唯一，行可消除
                val = 0;
                if (buf[y+(z+0)%3][x+0] > 0) val |=  buf[y+(z+0)%3][x+0];
                if (buf[y+(z+0)%3][x+1] > 0) val |=  buf[y+(z+0)%3][x+1];
                if (buf[y+(z+0)%3][x+2] > 0) val |=  buf[y+(z+0)%3][x+2];
                if (buf[y+(z+1)%3][x+0] > 0) val &= ~buf[y+(z+1)%3][x+0];
                if (buf[y+(z+1)%3][x+1] > 0) val &= ~buf[y+(z+1)%3][x+1];
                if (buf[y+(z+1)%3][x+2] > 0) val &= ~buf[y+(z+1)%3][x+2];
                if (buf[y+(z+2)%3][x+0] > 0) val &= ~buf[y+(z+2)%3][x+0];
                if (buf[y+(z+2)%3][x+1] > 0) val &= ~buf[y+(z+2)%3][x+1];
                if (buf[y+(z+2)%3][x+2] > 0) val &= ~buf[y+(z+2)%3][x+2];
                if (val)
                {
                    int chg = 0;
                    for (jj = 0; jj < 9; jj++)
                    {
                        if (jj / 3 == j) continue;
                        if (buf[y+z][jj] <= 0 || !(buf[y+z][jj] & val)) continue;
                        buf[y+z][jj] &= ~val;
                        chg = 1;
                    }
                    if (chg) return 1;
                }
                // 纵向列内唯一，宫可消除
                val = 0;
                int t = x + z % 3;
                for (o = 0; o < 9; o++)
                {
                    if (o < 3)
                    {
                        if (buf[y+o][t] > 0) val |=  buf[y+o][t];
                    }
                    else
                    {
                        if (buf[(y+o)%9][t] > 0) val &= ~buf[(y+o)%9][t];
                    }
                }
                if (val)
                {
                    int chg = 0;
                    for (ii = y; ii < y + 3; ii++)
                    {
                        for (jj = x; jj < x + 3; jj++)
                        {
                            if (jj == t) continue;
                            if (buf[ii][jj] <= 0 || !(buf[ii][jj] & val)) continue;
                            buf[ii][jj] &= ~val;
                            chg = 1;
                        }
                    }
                    if (chg) return 1;
                }

                val = 0;
                t = y + z % 3;
                for (o = 0; o < 9; o++)
                {
                    if (o < 3)
                    {
                        if (buf[t][x+o] > 0) val |=  buf[t][x+o];
                    }
                    else
                    {
                        if (buf[t][(x+o)%9] > 0) val &= ~buf[t][(x+o)%9];
                    }
                }
                if (val)
                {
                    int chg = 0;
                    for (ii = y; ii < y + 3; ii++)
                    {
                        for (jj = x; jj < x + 3; jj++)
                        {
                            if (ii == t) continue;
                            if (buf[ii][jj] <= 0 || !(buf[ii][jj] & val)) continue;
                            buf[ii][jj] &= ~val;
                            chg = 1;
                        }
                    }
                    if (chg) return 1;
                }
            }
        }
    }
    return 0;
}

// 隐循环闭环
static int sd_r4(int buf[9][9])
{
    int i,j,ii,jj,c,b,t;
    int bd[9];
    for (i = 0; i < 9; i++)
    {
        for (j = 0; j < 9; j++)
        {
            int val = buf[i][j];
            if (val <= 0) continue;
            int bitcount = bcount(val);
            for (c = 0, t = val; c < bitcount; c++)
            {
                // 稀疏矩阵
                bd[c] = 1 << boffs(t);
                t &= t - 1;
            }
            for (b = 2; b < bitcount; b++)
            {
                // 与 db 生成尽可能所有组合
                int mask = 1 << bitcount;
                for (;;)
                {
                    // 抹掉需要进位的比特
                    mask &= ~((1 << boffs(~mask)) - 1);
                    if (!mask) break;
                    mask = (1 << (boffs(mask) - 1)) | (mask & (mask - 1));
                    // 尾巴补齐比特位数
                    while (bcount(mask) < b)
                    {
                        mask |= 1 << (boffs(mask) - 1);
                    }
                    // 本次组合的匹配数值
                    int mval = 0;
                    for (t = mask; t; t &= t - 1)
                    {
                        mval |= bd[boffs(t)];
                    }
                    // 行
                    int mc = 0;
                    for (ii = 0; ii < 9; ii++)
                    {
                        int t = buf[ii][j];
                        if (t <= 0) continue;
                        t &= mval;
                        if (t == mval) mc++; // 包含
                        else if (t) // 不全包含
                        {
                            mc = 0;
                            break;
                        }
                    }
                    if (mc == b)
                    {
                        // n位子集出现n次
                        int chg = 0;
                        for (ii = 0; ii < 9; ii++)
                        {
                            if (buf[ii][j] <= 0) continue;
                            if ((buf[ii][j] & mval) == mval && buf[ii][j] != mval)
                            {
                                // 只能是组合内n位子集之一
                                buf[ii][j] = mval;
                                chg = 1;
                            }
                        }
                        if (chg) return 1;
                    }
                    // 列
                    mc = 0;
                    for (jj = 0; jj < 9; jj++)
                    {
                        int t = buf[i][jj];
                        if (t <= 0) continue;
                        t &= mval;
                        if (t == mval) mc++;
                        else if (t)
                        {
                            mc = 0;
                            break;
                        }
                    }
                    if (mc == b)
                    {
                        int chg = 0;
                        for (jj = 0; jj < 9; jj++)
                        {
                            if (buf[i][jj] <= 0) continue;
                            if ((buf[i][jj] & mval) == mval && buf[i][jj] != mval)
                            {
                                buf[i][jj] = mval;
                                chg = 1;
                            }
                        }
                        if (chg) return 1;
                    }
                    // 宫
                    mc = 0;
                    int di = i / 3;
                    int dj = j / 3;
                    int ei = di * 3 + 3;
                    int ej = dj * 3 + 3;
                    for (ii = ei - 3; ii < ei; ii++)
                    {
                        for (jj = ej - 3; jj < ej; jj++)
                        {
                            int t = buf[ii][jj];
                            if (t <= 0) continue;
                            t &= mval;
                            if (t == mval) mc++;
                            else if (t)
                            {
                                mc = 0;
                                ii = ei;
                                break;
                            }
                        }
                    }
                    if (mc == b)
                    {
                        int chg = 0;
                        for (ii = ei - 3; ii < ei; ii++)
                        {
                            for (jj = ej - 3; jj < ej; jj++)
                            {
                                if (buf[ii][jj] <= 0) continue;
                                if ((buf[ii][jj] & mval) == mval && buf[ii][jj] != mval)
                                {
                                    buf[ii][jj] = mval;
                                    chg = 1;
                                }
                            }
                        }
                        if (chg) return 1;
                    }
                }
            }
        }
    }
    return 0;
}

// 数组n
static int sd_r5(int buf[9][9])
{
    int i,j,b;
    // 从数组 3 到 8 尝试
    for (b = 3; b < 9; b++)
    {
        int mask = 1 << 9;
        for (;;)
        {
            // 抹掉需要进位的比特
            mask &= ~((1 << boffs(~mask)) - 1);
            if (!mask) break;
            // 生成当前组合
            mask = (1 << (boffs(mask) - 1)) | (mask & (mask - 1));
            while (bcount(mask) < b)
            {
                mask |= 1 << (boffs(mask) - 1);
            }
            // 行
            for (i = 0; i < 9; i++)
            {
                int mval = 0;
                int t = mask;
                while (t)
                {
                    int tv = buf[i][boffs(t)];
                    if (tv <= 0)
                    {
                        // 组合内包含确定的数，该组合失效
                        mval = 0;
                        break;
                    }
                    // 拼接
                    mval |= tv;
                    if (bcount(mval) > b)
                    {
                        mval = 0;
                        break;
                    }
                    t &= t - 1;
                }
                if (mval == 0 || bcount(mval) != b) continue;

                t = 0x1ff & ~mask;
                int chg = 0;
                while (t)
                {
                    j = boffs(t);
                    int tv = buf[i][j];
                    t &= t - 1;
                    if (tv <= 0) continue;
                    tv &= mval;
                    if (tv == 0) continue;
                    // 剃掉非组合内的值
                    buf[i][j] &= ~tv;
                    chg = 1;
                }
                if (chg) return 1;
            }
            // 列
            for (j = 0; j < 9; j++)
            {
                int mval = 0;
                int t = mask;
                while (t)
                {
                    int tv = buf[boffs(t)][j];
                    if (tv <= 0)
                    {
                        mval = 0;
                        break;
                    }
                    mval |= tv;
                    if (bcount(mval) > b)
                    {
                        mval = 0;
                        break;
                    }
                    t &= t - 1;
                }
                if (mval == 0 || bcount(mval) != b) continue;

                t = 0x1ff & ~mask;
                int chg = 0;
                while (t)
                {
                    i = boffs(t);
                    int tv = buf[i][j];
                    t &= t - 1;
                    if (tv <= 0) continue;
                    tv &= mval;
                    if (tv == 0) continue;
                    buf[i][j] &= ~tv;
                    chg = 1;
                }
                if (chg) return 1;
            }
            // 宫
            int oi,oj;
            for (oi = 0; oi < 3; oi++)
            {
                for (oj = 0; oj < 3; oj++)
                {
                    int mval = 0;
                    int t = mask;
                    while (t)
                    {
                        int bit = boffs(t);
                        i = oi * 3 + bit / 3;
                        j = oj * 3 + bit % 3;
                        int tv = buf[i][j];
                        if (tv <= 0)
                        {
                            mval = 0;
                            break;
                        }
                        mval |= tv;
                        if (bcount(mval) > b)
                        {
                            mval = 0;
                            break;
                        }
                        t &= t - 1;
                    }
                    if (mval == 0 || bcount(mval) != b) continue;

                    t = 0x1ff & ~mask;
                    int chg = 0;
                    while (t)
                    {
                        int bit = boffs(t);
                        i = oi * 3 + bit / 3;
                        j = oj * 3 + bit % 3;
                        int tv = buf[i][j];
                        t &= t - 1;
                        if (tv <= 0) continue;
                        tv &= mval;
                        if (tv == 0) continue;
                        buf[i][j] &= ~tv;
                        chg = 1;
                    }
                    if (chg) return 1;
                }
            }
        }
    }

    return 0;
}

static int sd_decode(int buf[9][9], char *his, int *hislen)
{
    if (sd_r0(buf, his, hislen)) return 1;

    if (sd_r1(buf, his, hislen)) return 1;

    if (sd_r2(buf)) return 1;

    if (sd_r3(buf)) return 1;

    if (sd_r4(buf)) return 1;

    if (sd_r5(buf)) return 1;

    // todo 更多规则



    // 校验
    int i,j;
    for (i = 0; i < 9; i++)
    {
        for (j = 0; j < 9; j++)
        {
            if (buf[i][j] == 0) return -1;
        }
    }
    return 0;
}

// -1 无解    0 成功    1 规则不够，暂时无法下一步解析
int sudoku(const char in[9][9], int out[9][9], char *his, int *hislen)
{
    int i,j;
    int buf[9][9];
    for (i = 0; i < 9; i++)
    {
        for (j = 0; j < 9; j++)
        {
            // 0x1ff <=> 0b000_0001_1111_1111
            buf[i][j] = in[i][j]? -in[i][j]: 0x1ff;
        }
    }
    // 初始化
    sd_init(buf);

    int ret = 0;
    if (hislen) *hislen = 0;
    while ((ret = sd_decode(buf, his, hislen)) > 0);

    for (i = 0; i < 9; i++)
    {
        for (j = 0; j < 9; j++)
        {
            out[i][j] = -buf[i][j];
            if (buf[i][j] > 0 && ret == 0) ret = 1;
        }
    }
    return ret;
}











// 打印候选
static void printOther(int v)
{
    printf("[");
    while (v)
    {
        printf("%d", boffs(v) + 1);
        v &= v - 1;
        if (v) printf("|");
    }
    printf("] ");
}

// 打印历史
static void printHis(const char *his, int len)
{
    int i;
    for (i = 0; i < len; i++)
    {
        printf("[%d,%d] ", his[i]/9 + 1, his[i]%9 + 1); 
    }
    printf("\n"); 
}

int main()
{
    //*
    const char in[9][9] = {
        {0, 4, 6, 9, 0, 3, 0, 0, 0},
        {0, 0, 3, 0, 5, 0, 0, 6, 0},
        {9, 0, 0, 0, 0, 2, 0, 0, 3},
        {0, 0, 5, 0, 0, 6, 0, 0, 0},
        {8, 0, 0, 0, 0, 0, 0, 1, 0},
        {0, 1, 0, 7, 8, 0, 2, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 5, 0},
        {0, 8, 1, 3, 0, 0, 0, 0, 7},
        {0, 0, 0, 8, 0, 0, 1, 0, 4}
    };
    // */
    /*
    const char in[9][9] = {
        {0,0,0,  0,0,0,  0,0,0},
        {0,0,0,  0,0,0,  0,0,0},
        {0,0,0,  0,0,0,  0,0,0},

        {0,0,0,  0,0,0,  0,0,0},
        {0,0,0,  0,0,0,  0,0,0},
        {0,0,0,  0,0,0,  0,0,0},

        {0,0,0,  0,0,0,  0,0,0},
        {0,0,0,  0,0,0,  0,0,0},
        {0,0,0,  0,0,0,  0,0,0}
    };
    // */
    /*
    const char in[9][9] = {
        {4,7,0,  3,0,1,  2,0,5},
        {0,5,1,  0,2,4,  0,0,3},
        {2,3,9,  6,5,7,  4,1,8},

        {0,2,5,  7,4,3,  0,0,0},
        {0,0,0,  5,0,0,  3,4,0},
        {0,4,3,  1,0,0,  0,5,0},

        {0,0,0,  2,1,5,  6,3,4},
        {3,0,4,  0,7,0,  5,2,0},
        {5,0,2,  4,3,0,  0,0,0}
    };
    // */
    /*
    const char in[9][9] = {
        {0,0,0,  3,0,0,  1,0,0},
        {5,0,0,  4,0,0,  0,9,0},
        {0,0,0,  0,2,8,  6,0,0},

        {0,9,0,  0,0,0,  0,0,1},
        {0,0,8,  0,0,7,  0,0,2},
        {0,1,0,  0,4,0,  8,0,0},

        {0,0,4,  0,8,5,  0,0,0},
        {3,0,0,  1,0,0,  0,4,0},
        {0,0,2,  6,0,0,  0,0,0}
    };
    // */
    /*
    const char in[9][9] = {
        {0,3,0,  0,0,0,  8,0,1},
        {7,0,0,  0,3,0,  0,0,0},
        {6,0,0,  0,0,0,  3,0,7},

        {3,0,0,  0,6,0,  0,7,0},
        {0,0,0,  2,0,0,  4,0,3},
        {0,0,0,  3,0,0,  0,0,0},

        {0,1,2,  8,0,3,  0,0,0},
        {9,8,3,  4,0,0,  5,0,0},
        {0,0,0,  0,0,0,  0,3,8}
    };
    // */
    /*
    const char in[9][9] = {
        {0,0,0,  0,0,0,  7,5,0},
        {4,0,0,  3,0,0,  0,0,0},
        {0,0,0,  0,4,0,  0,0,0},

        {0,0,0,  0,5,1,  0,7,0},
        {2,0,0,  0,0,0,  8,0,0},
        {0,0,0,  0,0,0,  0,0,0},

        {0,0,1,  8,0,0,  3,0,0},
        {0,6,0,  2,0,0,  0,0,0},
        {0,5,7,  0,0,0,  0,0,0}
    };
    // */
    int out[9][9];
    char his[81];
    int hislen;
    int ret = sudoku(in, out, his, &hislen);
    printHis(his, hislen);

    if (ret < 0) printf("Decoding failed\n");
    else if (ret > 0) printf("To be continued\n");

    int i,j;
    for (i = 0; i < 9; i++)
    {
        if (i % 3 == 0) printf("\n");
        for (j = 0; j < 9; j++)
        {
            if (j % 3 == 0) printf("    ");
            if (out[i][j] > 0) printf("%d ", out[i][j]);
            else if (out[i][j] == 0) printf("x ");
            else printOther(-out[i][j]);
        }
        printf("\n");
    }
    
    return 0;
}

