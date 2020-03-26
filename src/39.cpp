// C++11
#define _USE_MATH_DEFINES
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <cstdio>
#include <complex>
#include <numeric>
#include <string.h>
#include <random>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <cassert>
using namespace std;
const int inf = 1 << 30;

// #define LOCAL

const unsigned long long int cycle_per_sec = 2800000000;

unsigned long long int getCycle() {
    unsigned int low, high;
    __asm__ volatile ("rdtsc" : "=a" (low), "=d" (high));
    return ((unsigned long long int)low) | ((unsigned long long int)high << 32);
}

double getTime (unsigned long long int begin_cycle) {
    return (double)(getCycle() - begin_cycle) / cycle_per_sec;
}

class XorShift {
public:
    unsigned int x;
    unsigned int y;
    unsigned int z;
    unsigned int w;
    unsigned int t;
    
    XorShift(int tmp) {
        mt19937 rnd(tmp);
        x = rnd();
        y = rnd();
        z = rnd();
        w = rnd();
        t = 1;
    }
    
    int rand() {
        t = x ^ (x << 11);
        x = y;
        y = z;
        z = w;
        w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
        return w & 0x7fffffff;
    }
} engine(rand());

struct BASE {
    int h, w;
    int val[10][10];
};

double P;
int N, T;
int MINH, MINW;
BASE bases[100];
unsigned long long int start_cycle;
bool hash_table[101][101][8];

double board_val[101][101];

class STATE {
public:
    int h, w;
    int val[100][100];

    int ph[100], pw[100];

    double score;
    int loss_score, comp_score;

    static int que[100][100][100];
    static int que_si[100][100];
    static int prec[100][100][26];

    STATE () {
        score = inf;
    }

    void copy(STATE& st) {
        h = st.h; w = st.w;
        score = st.score;
        comp_score = st.comp_score; loss_score = st.loss_score;
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                val[i][j] = st.val[i][j];
            }
        }
        for (int i = 0; i < N; i++) {
            ph[i] = st.ph[i];
            pw[i] = st.pw[i];
        }
    }

    double eval() {
        comp_score = h*w;
        loss_score = 0.0;
        for (int i = 0; i < N; i++) {
            for (int nh = ph[i]; nh < ph[i]+bases[i].h; nh++) {
                for (int nw = pw[i]; nw < pw[i]+bases[i].w; nw++) {
#ifdef LOCAL
                    assert(val[nh][nw] != -1);
#endif
                    loss_score += abs(val[nh][nw]-bases[i].val[nh-ph[i]][nw-pw[i]]);
                }
            }
        }
        return P * comp_score/(double)T + (1-P) * loss_score  / (12.5*T);
    }

    void init_que () {
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                que_si[i][j] = 0;
            }
        }
        for (int i = 0; i < N; i++) {
            for (int h1 = ph[i]; h1 < ph[i]+bases[i].h; h1++) {
                for (int w1 = pw[i]; w1 < pw[i]+bases[i].w; w1++) {
                    int pos = que_si[h1][w1];
                    while (pos) {
                        if (que[h1][w1][pos-1] <= bases[i].val[h1-ph[i]][w1-pw[i]]) break;
                        que[h1][w1][pos] = que[h1][w1][pos-1];
                        pos--;
                    }
                    que_si[h1][w1]++;
                    que[h1][w1][pos] = bases[i].val[h1-ph[i]][w1-pw[i]];
                }
            }

        }
    }

    void random_init (int _h, int _w) {
        h = _h; w = _w;
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                val[i][j] = -1;
                que_si[i][j] = 0;
                for (int k = 0; k < 26; k++) {
                    prec[i][j][k] = 0;
                }
            }
        }
        int inds[100];
        for (int i = 0; i < N; i++) {
            inds[i] = i;
        }
        sort(inds, inds+N, [](const int& l, const int& r) {
            return bases[l].h*bases[l].w > bases[r].h*bases[r].w;
        });
        int ind_r1 = engine.rand()%2, ind_r2 = engine.rand()%2;
        if (ind_r1) {
            if (ind_r2) {
                sort(inds, inds+N, [](const int& l, const int& r) {
                    return bases[l].h > bases[r].h || (bases[l].h ==bases[r].h && bases[l].w > bases[r].w);
                });
            } else {
                sort(inds, inds+N, [](const int& l, const int& r) {
                    return bases[l].w > bases[r].w || (bases[l].w ==bases[r].w && bases[l].h > bases[r].h); 
                });
            }
        }
        int dir_rand = engine.rand()%2;
        if (hash_table[h][w][ind_r1*4+ind_r2*2+dir_rand]) {
            for (int i = 0; i < N-1; i++) {
                if (engine.rand()%2) continue;
                int j = engine.rand()%(N-i)+i;
                swap(inds[i], inds[j]);
            }
        }
        hash_table[h][w][ind_r1*4+ind_r2*2+dir_rand] = true;
        for (int _i = 0; _i < N; _i++) {
            int i = inds[_i];
            int score = inf;
            int h_pos = 0, w_pos = 0;
            
            if (dir_rand) {
                for (int j = 0; j <= h-bases[i].h; j++) {
                    for (int k = 0; k <= w-bases[i].w; k++) {
                        int nows = 0;
                        if (score == 0) break;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            for (int w1 = k; w1 < k+bases[i].w; w1++) {
                                int bv = bases[i].val[h1-j][w1-k];
                                if (prec[h1][w1][bv]) {
                                    nows = 1;
                                    break;
                                }
                            }
                            if (nows) break;
                        }
                        if (nows == 0) {
                            score = nows;
                            h_pos = j;
                            w_pos = k;
                        }
                    }
                }
                if (bases[i].w == 2) {
                    for (int j = 0; j <= h-bases[i].h; j++) for (int k = 0; k <= w-bases[i].w; k++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 3) {
                    for (int j = 0; j <= h-bases[i].h; j++) for (int k = 0; k <= w-bases[i].w; k++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 4) {
                    for (int j = 0; j <= h-bases[i].h; j++) for (int k = 0; k <= w-bases[i].w; k++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 5) {
                    for (int j = 0; j <= h-bases[i].h; j++) for (int k = 0; k <= w-bases[i].w; k++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 6) {
                    for (int j = 0; j <= h-bases[i].h; j++) for (int k = 0; k <= w-bases[i].w; k++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 7) {
                    for (int j = 0; j <= h-bases[i].h; j++) for (int k = 0; k <= w-bases[i].w; k++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            nows += prec[h1][k+6][bases[i].val[h1-j][6]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 8) {
                    for (int j = 0; j <= h-bases[i].h; j++) for (int k = 0; k <= w-bases[i].w; k++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            nows += prec[h1][k+6][bases[i].val[h1-j][6]];
                            nows += prec[h1][k+7][bases[i].val[h1-j][7]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 9) {
                    for (int j = 0; j <= h-bases[i].h; j++) for (int k = 0; k <= w-bases[i].w; k++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            nows += prec[h1][k+6][bases[i].val[h1-j][6]];
                            nows += prec[h1][k+7][bases[i].val[h1-j][7]];
                            nows += prec[h1][k+8][bases[i].val[h1-j][8]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else {
                    for (int j = 0; j <= h-bases[i].h; j++) for (int k = 0; k <= w-bases[i].w; k++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            nows += prec[h1][k+6][bases[i].val[h1-j][6]];
                            nows += prec[h1][k+7][bases[i].val[h1-j][7]];
                            nows += prec[h1][k+8][bases[i].val[h1-j][8]];
                            nows += prec[h1][k+9][bases[i].val[h1-j][9]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                }
            } else {
                for (int k = 0; k <= w-bases[i].w; k++) {
                    for (int j = 0; j <= h-bases[i].h; j++) {
                        int nows = 0;
                        if (score == 0) break;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            for (int w1 = k; w1 < k+bases[i].w; w1++) {
                                int bv = bases[i].val[h1-j][w1-k];
                                if (prec[h1][w1][bv]) {
                                    nows = 1;
                                    break;
                                }
                            }
                            if (nows) break;
                        }
                        if (nows == 0) {
                            score = nows;
                            h_pos = j;
                            w_pos = k;
                        }
                    }
                }
                if (bases[i].w == 2) {
                    for (int k = 0; k <= w-bases[i].w; k++) for (int j = 0; j <= h-bases[i].h; j++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 3) {
                    for (int k = 0; k <= w-bases[i].w; k++) for (int j = 0; j <= h-bases[i].h; j++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 4) {
                    for (int k = 0; k <= w-bases[i].w; k++) for (int j = 0; j <= h-bases[i].h; j++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 5) {
                    for (int k = 0; k <= w-bases[i].w; k++) for (int j = 0; j <= h-bases[i].h; j++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 6) {
                    for (int k = 0; k <= w-bases[i].w; k++) for (int j = 0; j <= h-bases[i].h; j++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 7) {
                    for (int k = 0; k <= w-bases[i].w; k++) for (int j = 0; j <= h-bases[i].h; j++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            nows += prec[h1][k+6][bases[i].val[h1-j][6]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 8) {
                    for (int k = 0; k <= w-bases[i].w; k++) for (int j = 0; j <= h-bases[i].h; j++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            nows += prec[h1][k+6][bases[i].val[h1-j][6]];
                            nows += prec[h1][k+7][bases[i].val[h1-j][7]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 9) {
                    for (int k = 0; k <= w-bases[i].w; k++) for (int j = 0; j <= h-bases[i].h; j++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            nows += prec[h1][k+6][bases[i].val[h1-j][6]];
                            nows += prec[h1][k+7][bases[i].val[h1-j][7]];
                            nows += prec[h1][k+8][bases[i].val[h1-j][8]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else {
                    for (int k = 0; k <= w-bases[i].w; k++) for (int j = 0; j <= h-bases[i].h; j++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            nows += prec[h1][k+6][bases[i].val[h1-j][6]];
                            nows += prec[h1][k+7][bases[i].val[h1-j][7]];
                            nows += prec[h1][k+8][bases[i].val[h1-j][8]];
                            nows += prec[h1][k+9][bases[i].val[h1-j][9]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                }
            }
            ph[i] = h_pos; pw[i] = w_pos;
            for (int h1 = h_pos; h1 < h_pos+bases[i].h; h1++) {
                for (int w1 = w_pos; w1 < w_pos+bases[i].w; w1++) {
                    int pos = que_si[h1][w1];
                    int bv = bases[i].val[h1-h_pos][w1-w_pos];
                    while (pos) {
                        if (que[h1][w1][pos-1] <= bv) break;
                        que[h1][w1][pos] = que[h1][w1][pos-1];
                        pos--;
                    }
                    que_si[h1][w1]++;
                    que[h1][w1][pos] = bases[i].val[h1-h_pos][w1-w_pos];

                    val[h1][w1] = que[h1][w1][que_si[h1][w1]/2];
                    int val2 = que[h1][w1][(que_si[h1][w1]-1)/2];
                    for (int k = 0; k < 26; k++) {
                        if (k <= val2) prec[h1][w1][k] = val2-k;
                        else if (k >= val[h1][w1]) prec[h1][w1][k] = k-val[h1][w1];
                        else prec[h1][w1][k] = 0;
                    }
                }
            }
        }
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                if (que_si[i][j] == 0) continue;
                val[i][j] = que[i][j][que_si[i][j]/2];
            }
        }
        score = eval();
    }

    void random_exchange (double start_time, double end_time) {
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                val[i][j] = -1;
                que_si[i][j] = 0;
            }
        }
        int inds[100];
        for (int i = 0; i < N; i++) {
            inds[i] = i;
        }
        for (int i = 0; i < N-1; i++) {
            int j = engine.rand()%(N-i)+i;
            swap(inds[i], inds[j]);
        }
        int ratio = min(N-3, (int)(N*(getTime(start_cycle)-start_time)/(end_time-start_time)));
        ratio = max(ratio, 0);
        // int ratio = N-5;
        for (int _i = 0; _i < ratio; _i++) {
            int i = inds[_i];
            int h_pos = ph[i], w_pos = pw[i];
            for (int h1 = h_pos; h1 < h_pos+bases[i].h; h1++) {
                for (int w1 = w_pos; w1 < w_pos+bases[i].w; w1++) {
                    int pos = que_si[h1][w1];
                    while (pos) {
                        if (que[h1][w1][pos-1] <= bases[i].val[h1-h_pos][w1-w_pos]) break;
                        que[h1][w1][pos] = que[h1][w1][pos-1];
                        pos--;
                    }
                    que_si[h1][w1]++;
                    que[h1][w1][pos] = bases[i].val[h1-h_pos][w1-w_pos];
                }
            }
        }
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                if (que_si[i][j] == 0) {
                    for (int k = 0; k < 26; k++) prec[i][j][k] = 0;
                    continue;
                }
                val[i][j] = que[i][j][que_si[i][j]/2];
                int val2 = que[i][j][(que_si[i][j]-1)/2];
                for (int k = 0; k < 26; k++) {
                    if (k <= val2) prec[i][j][k] = val2-k;
                    else if (val[i][j] <= k) prec[i][j][k] = k-val[i][j];
                    else prec[i][j][k] = 0;
                }
            }
        }
        //cerr << "o";
        int dir_rand = engine.rand()%2;
        for (int _i = ratio; _i < N; _i++) {
            int i = inds[_i];
            int score = inf;
            int h_pos = 0, w_pos = 0;
            if (dir_rand) {
                if (bases[i].w == 2) {
                    for (int j = 0; j <= h-bases[i].h; j++) for (int k = 0; k <= w-bases[i].w; k++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 3) {
                    for (int j = 0; j <= h-bases[i].h; j++) for (int k = 0; k <= w-bases[i].w; k++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 4) {
                    for (int j = 0; j <= h-bases[i].h; j++) for (int k = 0; k <= w-bases[i].w; k++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 5) {
                    for (int j = 0; j <= h-bases[i].h; j++) for (int k = 0; k <= w-bases[i].w; k++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 6) {
                    for (int j = 0; j <= h-bases[i].h; j++) for (int k = 0; k <= w-bases[i].w; k++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 7) {
                    for (int j = 0; j <= h-bases[i].h; j++) for (int k = 0; k <= w-bases[i].w; k++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            nows += prec[h1][k+6][bases[i].val[h1-j][6]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 8) {
                    for (int j = 0; j <= h-bases[i].h; j++) for (int k = 0; k <= w-bases[i].w; k++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            nows += prec[h1][k+6][bases[i].val[h1-j][6]];
                            nows += prec[h1][k+7][bases[i].val[h1-j][7]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 9) {
                    for (int j = 0; j <= h-bases[i].h; j++) for (int k = 0; k <= w-bases[i].w; k++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            nows += prec[h1][k+6][bases[i].val[h1-j][6]];
                            nows += prec[h1][k+7][bases[i].val[h1-j][7]];
                            nows += prec[h1][k+8][bases[i].val[h1-j][8]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else {
                    for (int j = 0; j <= h-bases[i].h; j++) for (int k = 0; k <= w-bases[i].w; k++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            nows += prec[h1][k+6][bases[i].val[h1-j][6]];
                            nows += prec[h1][k+7][bases[i].val[h1-j][7]];
                            nows += prec[h1][k+8][bases[i].val[h1-j][8]];
                            nows += prec[h1][k+9][bases[i].val[h1-j][9]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                }
            } else {
                if (bases[i].w == 2) {
                    for (int k = 0; k <= w-bases[i].w; k++) for (int j = 0; j <= h-bases[i].h; j++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 3) {
                    for (int k = 0; k <= w-bases[i].w; k++) for (int j = 0; j <= h-bases[i].h; j++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 4) {
                    for (int k = 0; k <= w-bases[i].w; k++) for (int j = 0; j <= h-bases[i].h; j++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 5) {
                    for (int k = 0; k <= w-bases[i].w; k++) for (int j = 0; j <= h-bases[i].h; j++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 6) {
                    for (int k = 0; k <= w-bases[i].w; k++) for (int j = 0; j <= h-bases[i].h; j++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 7) {
                    for (int k = 0; k <= w-bases[i].w; k++) for (int j = 0; j <= h-bases[i].h; j++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            nows += prec[h1][k+6][bases[i].val[h1-j][6]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 8) {
                    for (int k = 0; k <= w-bases[i].w; k++) for (int j = 0; j <= h-bases[i].h; j++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            nows += prec[h1][k+6][bases[i].val[h1-j][6]];
                            nows += prec[h1][k+7][bases[i].val[h1-j][7]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else if (bases[i].w == 9) {
                    for (int k = 0; k <= w-bases[i].w; k++) for (int j = 0; j <= h-bases[i].h; j++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            nows += prec[h1][k+6][bases[i].val[h1-j][6]];
                            nows += prec[h1][k+7][bases[i].val[h1-j][7]];
                            nows += prec[h1][k+8][bases[i].val[h1-j][8]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                } else {
                    for (int k = 0; k <= w-bases[i].w; k++) for (int j = 0; j <= h-bases[i].h; j++) {
                        if (score == 0) break;
                        int nows = 0;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            nows += prec[h1][k][bases[i].val[h1-j][0]];
                            nows += prec[h1][k+1][bases[i].val[h1-j][1]];
                            nows += prec[h1][k+2][bases[i].val[h1-j][2]];
                            nows += prec[h1][k+3][bases[i].val[h1-j][3]];
                            nows += prec[h1][k+4][bases[i].val[h1-j][4]];
                            nows += prec[h1][k+5][bases[i].val[h1-j][5]];
                            nows += prec[h1][k+6][bases[i].val[h1-j][6]];
                            nows += prec[h1][k+7][bases[i].val[h1-j][7]];
                            nows += prec[h1][k+8][bases[i].val[h1-j][8]];
                            nows += prec[h1][k+9][bases[i].val[h1-j][9]];
                            if (nows >= score) break;
                        }
                        if (nows < score) { score = nows; h_pos = j; w_pos = k; }
                    }
                }
            } 
            ph[i] = h_pos; pw[i] = w_pos;
            for (int h1 = h_pos; h1 < h_pos+bases[i].h; h1++) {
                for (int w1 = w_pos; w1 < w_pos+bases[i].w; w1++) {
                    int pos = que_si[h1][w1];
                    while (pos) {
                        if (que[h1][w1][pos-1] <= bases[i].val[h1-h_pos][w1-w_pos]) break;
                        que[h1][w1][pos] = que[h1][w1][pos-1];
                        pos--;
                    }
                    que_si[h1][w1]++;
                    que[h1][w1][pos] = bases[i].val[h1-h_pos][w1-w_pos];
                    val[h1][w1] = que[h1][w1][que_si[h1][w1]/2];
                    int val2 = que[h1][w1][(que_si[h1][w1]-1)/2];
                    for (int k = 0; k < 26; k++) {
                        if (k <= val2) prec[h1][w1][k] = val2-k;
                        else if (k >= val[h1][w1]) prec[h1][w1][k] = k-val[h1][w1];
                        else prec[h1][w1][k] = 0;
                    }
                }
            }
        }
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                if (que_si[i][j] == 0) continue;
                val[i][j] = que[i][j][que_si[i][j]/2];
            }
        }
        score = eval();
    }

    void output() {
        cout << h << endl;
        //cerr << h << " " << w << endl;
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                if (val[i][j] == -1) {
                    cout << 'A';
                } else {
                    cout << (char)('A'+val[i][j]);
                }
            }
            cout << endl;
        }
        for (int i = 0; i < N; i++) {
            cout << ph[i] << " " << pw[i] << endl;
        }
    }
};

int STATE::que[100][100][100];
int STATE::que_si[100][100];
int STATE::prec[100][100][26];

STATE ans;

void input() {
    cin >> P >> N;
    for (int i = 0; i < N; i++) {
        cin >> bases[i].h;
        string str;
        for (int y = 0; y < bases[i].h; y++) {
            cin >> str;
            bases[i].w = str.size();
            for (int x = 0; x < bases[i].w; x++) {
                bases[i].val[y][x] = str[x]-'A';
            }
        }
    }
}

void init() {
    MINH = 0; MINW = 0;
    T = 0;
    for (int i = 0; i < N; i++) {
        MINH = max(MINH, bases[i].h);
        MINW = max(MINW, bases[i].w);
        T += bases[i].h*bases[i].w;
    }
    for (int i = 0; i <= 100; i++) {
        for (int j = 0; j <= 100; j++) {
            board_val[i][j] = inf;
        }
    }
}

void grid_search() {
    for (int i = MINH; i < 100; i += 10) {
        for (int j = MINW; j < 100; j += 10) {
            if (P*i*j/(double)T >= ans.score) continue;
            STATE state;
            state.random_init(i, j);
            board_val[i][j] = state.score;
            if (state.score < ans.score) {
                ans.copy(state);
            }
#ifdef LOCAL
            cerr << state.score << " ";
#endif
        }
#ifdef LOCAL
        cerr << endl;
#endif
    }
}

void solve() {
    init();

    grid_search();

    int cnt = 0;

    while (getTime(start_cycle) < 2.5) {
        STATE state;
        int nh = ans.h+engine.rand()%11-5;
        int nw = ans.w+engine.rand()%11-5;
        if (ans.loss_score == 0 && ans.w*ans.h <= nh*nw) continue;
        if (engine.rand()%2) swap(nh, nw);

        if (engine.rand()%2) {
            int S = ans.w*ans.h;
            // h * MINW / T * P < ans.score
            int maxh = ans.score*T/(P*MINW)+1;
            nh = MINH + engine.rand()%(maxh-MINH);
            nw = (S-1) / nh + engine.rand()%11 - 5;
        }

        nh = max(MINH, min(100, nh));
        nw = max(MINW, min(100, nw));

        if (P*nh*nw/(double)T >= ans.score) continue;
        cnt++;
        state.random_init(nh, nw);

        board_val[nh][nw] = min(board_val[nh][nw], state.score);

        if (state.score < ans.score) {
            ans.copy(state);
        }
    }
    vector<int> binds(101*101);
    for (int i = 0; i < 101*101; i++) binds[i] = i;
    sort(binds.begin(), binds.end(), [](const int& l, const int& r) {
        return board_val[l/101][l%101] < board_val[r/101][r%101];
    });
    /*
    for (int i = 0; i < 5; i++) {
        cerr << binds[i]/101 << " " << binds[i]%101 << " " << board_val[binds[i]/101][binds[i]%101] << endl;
    }
    cerr << endl;
    */
    for (int i = 0; i < 5; i++) {
        int nh = binds[i]/101, nw = binds[i]%101;
        STATE state;
        state.random_init(nh, nw);

        if (state.loss_score == 0) continue;

        while (getTime(start_cycle) < 2.5+(double)(i+1)*0.4) {

            STATE tmp; tmp.copy(state);

            tmp.random_exchange(2.5+(double)i*0.4, 2.5+(double)(i+1)*0.4);

            if (tmp.score < state.score) {
                state.copy(tmp);
            }
        }
        board_val[nh][nw] = min(board_val[nh][nw], state.score);

        if (state.score < ans.score) {
            ans.copy(state);
        }
    }
    /*
    for (int i = 0; i < 5; i++) {
        cerr << binds[i]/101 << " " << binds[i]%101 << " " << board_val[binds[i]/101][binds[i]%101] << endl;
    }
    */

#ifdef LOCAL
    cerr << ans.score << endl;
#endif
    //if (ans.h <= 10 && ans.w <= 10) {
    //cerr << ans.loss_score << " " << ans.comp_score << endl;
    if (ans.loss_score == 0) {
        // cerr << "hoge" << endl;
        int loop2 = 0;
        while (getTime(start_cycle) < 9.8) {
            loop2++;
            STATE state;
            int S = ans.w*ans.h;
            int nh = MINH + engine.rand()%(100-MINH);
            //int nw = ans.w+engine.rand()%11-5;
            int nw = (S-1) / nh;
            if (ans.loss_score != 0 && engine.rand()%2) {
                nw++;
            }
            if (nw < MINW || nw >= 100) continue;
            if (P*nh*nw/(double)T >= ans.score) continue;
            state.random_init(nh, nw);
            if (state.score < ans.score) {
                ans.copy(state);
            }
        }
        // cerr << loop2 << endl;
    } else {
        int loop2 = 0;
        double st_time = getTime(start_cycle);
        while (getTime(start_cycle) < 9.8) {
            loop2++;
            STATE state;
            state.copy(ans);

            state.random_exchange(st_time, 9.8);

            if (state.score < ans.score) {
                ans.copy(state);
            }
            if (ans.loss_score == 0) break;
        }
        //cerr << loop2 << endl;
        // cerr << "huga" << endl;
        while (getTime(start_cycle) < 9.8) {
            STATE state;
            int S = ans.w*ans.h;
            int nh = MINH + engine.rand()%(100-MINH);
            //int nw = ans.w+engine.rand()%11-5;
            int nw = (S-1) / nh;
            if (ans.loss_score != 0 && engine.rand()%2) {
                nw++;
            }
            if (nw < MINW || nw >= 100) continue;
            if (P*nh*nw/(double)T >= ans.score) continue;
            loop2++;
            state.random_init(nh, nw);
            if (state.score < ans.score) {
                ans.copy(state);
            }
        }
        // cerr << loop2 << endl;
    }

#ifdef LOCAL
    cerr << ans.score << endl;
#endif
}

int main() {
    start_cycle = getCycle();

    input();

    solve();

    ans.output();
#ifdef LOCAL
    cerr << ans.comp_score * P << " " << ans.loss_score * (1-P) << endl;
#endif
    //cerr << ans.score << endl;
}
