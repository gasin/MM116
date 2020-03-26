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

class STATE {
public:
    int h, w;
    int val[100][100];

    int ph[100], pw[100];

    double score;
    double comp_score, loss_score;

    static int que[100][100][100];
    static int que_si[100][100];

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
        comp_score = h*w/(double)T;
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
        loss_score /= (12.5*T);
        return comp_score * P + loss_score * (1-P);
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

    void fill_init (int _h, int _w) {
        h = _h; w = _w;
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                val[i][j] = -1;
                que_si[i][j] = 0;
            }
        }
        int inds[100], used[100];
        for (int i = 0; i < N; i++) {
            inds[i] = i;
            used[i] = false;
        }
        for (int i = 0; i < N; i++) {
            if (used[i] || bases[i].h < h-1) continue;
            for (int j = 0; j < w; j++) {
                if (que_si[0][j] == 0) {
                    used[i] = true;
                    ph[i] = 0; pw[i] = j;
                    for (int h1 = ph[i]; h1 < ph[i]+bases[i].h; h1++) {
                        for (int w1 = pw[i]; w1 < pw[i]+bases[i].w; w1++) {
                            que_si[h1][w1]++;
                            que[h1][w1][0] = bases[i].val[h1-ph[i]][w1-pw[i]];
                        }
                    }
                }
            }
        }
        for (int i = 0; i < N; i++) {
            if (used[i] || bases[i].w < w-1) continue;
            for (int j = 0; j < h; j++) {
                if (que_si[j][0] == 0) {
                    used[i] = true;
                    ph[i] = j; pw[i] = 0;
                    for (int h1 = ph[i]; h1 < ph[i]+bases[i].h; h1++) {
                        for (int w1 = pw[i]; w1 < pw[i]+bases[i].w; w1++) {
                            que_si[h1][w1]++;
                            que[h1][w1][0] = bases[i].val[h1-ph[i]][w1-pw[i]];
                        }
                    }
                }
            }
        }
        sort(inds, inds+N, [](const int& l, const int& r) {
            return bases[l].h*bases[l].w > bases[r].h*bases[r].w;
        });
        if (engine.rand()%2) {
            if (engine.rand()%2) {
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
        for (int _i = 0; _i < N; _i++) {
            int i = inds[_i];
            if (used[i]) continue;
            int score = inf;
            int h_pos = 0, w_pos = 0;
            
            if (dir_rand) {
                for (int j = 0; j <= h-bases[i].h; j++) {
                    for (int k = 0; k <= w-bases[i].w; k++) {
                        int nows = 0;
                        if (score == 0) break;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            for (int w1 = k; w1 < k+bases[i].w; w1++) {
                                int qs = que_si[h1][w1];
                                int bv = bases[i].val[h1-j][w1-k];
                                if (qs && bv != que[h1][w1][0]) {
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
                for (int j = 0; j <= h-bases[i].h; j++) {
                    for (int k = 0; k <= w-bases[i].w; k++) {
                        int nows = 0;
                        if (score == 0) break;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            for (int w1 = k; w1 < k+bases[i].w; w1++) {
                                int qs = que_si[h1][w1];
                                int bv = bases[i].val[h1-j][w1-k];
                                if (qs == 0) continue;
                                if (qs&1)                      { nows += abs(bv-que[h1][w1][qs/2]); } else
                                if (bv >= que[h1][w1][qs/2])   { nows += bv-que[h1][w1][qs/2];      } else
                                if (bv <= que[h1][w1][qs/2-1]) { nows += que[h1][w1][qs/2-1]-bv;    }
                            }
                            if (nows >= score) break;
                        }
                        if (nows < score) {
                            score = nows;
                            h_pos = j;
                            w_pos = k;
                        }
                    }
                }
            } else {
                for (int k = 0; k <= w-bases[i].w; k++) {
                    for (int j = 0; j <= h-bases[i].h; j++) {
                        int nows = 0;
                        if (score == 0) break;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            for (int w1 = k; w1 < k+bases[i].w; w1++) {
                                int qs = que_si[h1][w1];
                                int bv = bases[i].val[h1-j][w1-k];
                                if (qs && bv != que[h1][w1][0]) {
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
                for (int k = 0; k <= w-bases[i].w; k++) {
                    for (int j = 0; j <= h-bases[i].h; j++) {
                        int nows = 0;
                        if (score == 0) break;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            for (int w1 = k; w1 < k+bases[i].w; w1++) {
                                int qs = que_si[h1][w1];
                                int bv = bases[i].val[h1-j][w1-k];
                                if (qs == 0) continue;
                                if (qs&1)                      { nows += abs(bv-que[h1][w1][qs/2]); } else
                                if (bv >= que[h1][w1][qs/2])   { nows += bv-que[h1][w1][qs/2];      } else
                                if (bv <= que[h1][w1][qs/2-1]) { nows += que[h1][w1][qs/2-1]-bv;    }
                            }
                            if (nows >= score) break;
                        }
                        if (nows < score) {
                            score = nows;
                            h_pos = j;
                            w_pos = k;
                        }
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
                }
            }
        }
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                if (que_si[i][j] == 0) continue;
                val[i][j] = que[i][j][que_si[i][j]/2];
#ifdef LOCAL
                for (int k = 0; k < que_si[i][j]-1; k++) {
                    assert(que[i][j][k] <= que[i][j][k+1]);
                }
#endif
            }
        }
    }

    void random_init() {
        h = engine.rand()%(100-MINH) + MINH; w = engine.rand()%(100-MINW) + MINW;
        random_init(h, w);
    }

    void random_init (int _h, int _w) {
        h = _h; w = _w;
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
                                int qs = que_si[h1][w1];
                                int bv = bases[i].val[h1-j][w1-k];
                                if (qs && bv != que[h1][w1][0]) {
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
                for (int j = 0; j <= h-bases[i].h; j++) {
                    for (int k = 0; k <= w-bases[i].w; k++) {
                        int nows = 0;
                        if (score == 0) break;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            for (int w1 = k; w1 < k+bases[i].w; w1++) {
                                int qs = que_si[h1][w1];
                                int bv = bases[i].val[h1-j][w1-k];
                                if (qs == 0) continue;
                                if (qs&1)                      { nows += abs(bv-que[h1][w1][qs/2]); } else
                                if (bv >= que[h1][w1][qs/2])   { nows += bv-que[h1][w1][qs/2];      } else
                                if (bv <= que[h1][w1][qs/2-1]) { nows += que[h1][w1][qs/2-1]-bv;    }
                            }
                            if (nows >= score) break;
                        }
                        if (nows < score) {
                            score = nows;
                            h_pos = j;
                            w_pos = k;
                        }
                    }
                }
            } else {
                for (int k = 0; k <= w-bases[i].w; k++) {
                    for (int j = 0; j <= h-bases[i].h; j++) {
                        int nows = 0;
                        if (score == 0) break;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            for (int w1 = k; w1 < k+bases[i].w; w1++) {
                                int qs = que_si[h1][w1];
                                int bv = bases[i].val[h1-j][w1-k];
                                if (qs && bv != que[h1][w1][0]) {
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
                for (int k = 0; k <= w-bases[i].w; k++) {
                    for (int j = 0; j <= h-bases[i].h; j++) {
                        int nows = 0;
                        if (score == 0) break;
                        for (int h1 = j; h1 < j+bases[i].h; h1++) {
                            for (int w1 = k; w1 < k+bases[i].w; w1++) {
                                int qs = que_si[h1][w1];
                                int bv = bases[i].val[h1-j][w1-k];
                                if (qs == 0) continue;
                                if (qs&1)                      { nows += abs(bv-que[h1][w1][qs/2]); } else
                                if (bv >= que[h1][w1][qs/2])   { nows += bv-que[h1][w1][qs/2];      } else
                                if (bv <= que[h1][w1][qs/2-1]) { nows += que[h1][w1][qs/2-1]-bv;    }
                            }
                            if (nows >= score) break;
                        }
                        if (nows < score) {
                            score = nows;
                            h_pos = j;
                            w_pos = k;
                        }
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
                }
            }
        }
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                if (que_si[i][j] == 0) continue;
                val[i][j] = que[i][j][que_si[i][j]/2];
#ifdef LOCAL
                for (int k = 0; k < que_si[i][j]-1; k++) {
                    assert(que[i][j][k] <= que[i][j][k+1]);
                }
#endif
            }
        }
        score = eval();
    }

    void polish () {
        double end_time = 9.8;
        init_que();
        int loop = 0;
        while (getTime(start_cycle) < end_time) {
            loop++;
            int v = engine.rand()%N;
            int hp = engine.rand()%(h-bases[v].h+1);
            int wp = engine.rand()%(w-bases[v].w+1);

            int score = 0;

            for (int h1 = ph[v]; h1 < ph[v]+bases[v].h; h1++) {
                for (int w1 = pw[v]; w1 < pw[v]+bases[v].w; w1++) {
                    int qs = que_si[h1][w1];
                    if (qs == 0) continue;
                    int nowv = bases[v].val[h1-ph[v]][w1-pw[v]];
                    if (qs&1)                        { score += abs(nowv-que[h1][w1][qs/2]); } else
                    if (nowv >= que[h1][w1][qs/2])   { score += nowv-que[h1][w1][qs/2-1];    } else
                    if (nowv <= que[h1][w1][qs/2-1]) { score += que[h1][w1][qs/2]-nowv;      }

                    int pos = 0;
                    for ( ; ; pos++) {
#ifdef LOCAL
                        assert(pos < que_si[h1][w1]);
#endif
                        if (que[h1][w1][pos] == nowv) {
                            break;
                        }
                    }
                    for ( ; pos < que_si[h1][w1]; pos++) {
                        que[h1][w1][pos] = que[h1][w1][pos+1];
                    }
                    que_si[h1][w1]--;
                }
            }

            int nows = 0;

            for (int h1 = hp; h1 < hp+bases[v].h; h1++) {
                for (int w1 = wp; w1 < wp+bases[v].w; w1++) {
                    int qs = que_si[h1][w1];
                    int bv = bases[v].val[h1-hp][w1-wp];
                    if (que_si[h1][w1] == 0) continue;
                    if (que_si[h1][w1]&1)          { nows += abs(bv-que[h1][w1][qs/2]); } else
                    if (bv >= que[h1][w1][qs/2])   { nows += bv-que[h1][w1][qs/2];      } else
                    if (bv <= que[h1][w1][qs/2-1]) { nows += que[h1][w1][qs/2-1]-bv;    }
                }
            }

            if (score > nows) {
                ph[v] = hp; pw[v] = wp;
            }
            for (int h1 = ph[v]; h1 < ph[v]+bases[v].h; h1++) {
                for (int w1 = pw[v]; w1 < pw[v]+bases[v].w; w1++) {
                    int pos = que_si[h1][w1];
                    while (pos) {
                        if (que[h1][w1][pos-1] <= bases[v].val[h1-ph[v]][w1-pw[v]]) break;
                        que[h1][w1][pos] = que[h1][w1][pos-1];
                        pos--;
                    }
                    que_si[h1][w1]++;
                    que[h1][w1][pos] = bases[v].val[h1-ph[v]][w1-pw[v]];
                }
            }
        }
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                if (que_si[i][j] == 0) continue;
                val[i][j] = que[i][j][que_si[i][j]/2];
            }
        }
#ifdef LOCAL
        cerr << "polish_loop: " << loop << endl;
#endif
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
        for (int _i = ratio; _i < N; _i++) {
            int i = inds[_i];
            int score = inf;
            int h_pos = 0, w_pos = 0;
            for (int j = 0; j <= h-bases[i].h; j++) {
                for (int k = 0; k <= w-bases[i].w; k++) {
                    int nows = 0;
                    for (int h1 = j; h1 < j+bases[i].h; h1++) {
                        for (int w1 = k; w1 < k+bases[i].w; w1++) {
                            int qs = que_si[h1][w1];
                            int bv = bases[i].val[h1-j][w1-k];
                            if (qs == 0) continue;
                            if (qs&1)                      { nows += abs(bv-que[h1][w1][qs/2]); } else
                            if (bv >= que[h1][w1][qs/2])   { nows += bv-que[h1][w1][qs/2];      } else
                            if (bv <= que[h1][w1][qs/2-1]) { nows += que[h1][w1][qs/2-1]-bv;    }
                        }
                        if (nows >= score) break;
                    }
                    if (nows < score) {
                        score = nows;
                        h_pos = j;
                        w_pos = k;
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
                }
            }
        }
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                if (que_si[i][j] == 0) continue;
                val[i][j] = que[i][j][que_si[i][j]/2];
#ifdef LOCAL
                for (int k = 0; k < que_si[i][j]-1; k++) {
                    assert(que[i][j][k] <= que[i][j][k+1]);
                }
#endif
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
}

void grid_search() {
    for (int i = MINH; i < 100; i += 10) {
        for (int j = MINW; j < 100; j += 10) {
            if (P*i*j/(double)T >= ans.score) continue;
            STATE state;
            state.random_init(i, j);
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
    engine.rand();
    init();

    grid_search();

    int cnt = 0;

    while (getTime(start_cycle) < 4.5) {
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
        cnt++;

        nh = max(MINH, min(100, nh));
        nw = max(MINW, min(100, nw));

        if (P*nh*nw/(double)T >= ans.score) continue;
        state.random_init(nh, nw);
        if (state.score < ans.score) {
            ans.copy(state);
        }
    }

#ifdef LOCAL
    cerr << ans.score << endl;
#endif
    //if (ans.h <= 10 && ans.w <= 10) {
    //cerr << ans.loss_score << " " << ans.comp_score << endl;
    if (ans.loss_score == 0) {
        // cerr << "hoge" << endl;
        int loop2 = 0;
        while (getTime(start_cycle) < 9.5) {
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
            //if (engine.rand()%2) state.fill_init(nh, nw);
            //else state.random_init(nh, nw);
            state.random_init(nh, nw);
            if (state.score < ans.score) {
                ans.copy(state);
            }
        }
        // cerr << loop2 << endl;
    } else {
        int loop2 = 0;
        while (getTime(start_cycle) < 9.5) {
            loop2++;
            STATE state;
            state.copy(ans);

            state.random_exchange(4.5, 9.5);

            if (state.score < ans.score) {
                ans.copy(state);
            }
            if (ans.loss_score == 0) break;
        }
        // cerr << "huga" << endl;
        while (getTime(start_cycle) < 9.5) {
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
            //if (engine.rand()%2) state.fill_init(nh, nw);
            //else state.random_init(nh, nw);
            if (state.score < ans.score) {
                ans.copy(state);
            }
        }
        // cerr << loop2 << endl;
    }

    ans.polish();
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
