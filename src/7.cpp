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
        for (int i = 0; i < N-1; i++) {
            int j = engine.rand()%(N-i)+i;
            swap(inds[i], inds[j]);
        }
        for (int _i = 0; _i < N; _i++) {
            int i = inds[_i];
            int score = inf;
            int h_pos = 0, w_pos = 0;
            for (int j = 0; j <= h-bases[i].h; j++) {
                for (int k = 0; k <= w-bases[i].w; k++) {
                    int nows = 0;
                    for (int h1 = j; h1 < j+bases[i].h; h1++) {
                        for (int w1 = k; w1 < k+bases[i].w; w1++) {
                            if (que_si[h1][w1] == 0) continue;
                            if (que_si[h1][w1]&1) {
                                nows += abs(bases[i].val[h1-j][w1-k]-que[h1][w1][que_si[h1][w1]/2]);
                            } else {
                                nows += min(abs(bases[i].val[h1-j][w1-k]-que[h1][w1][que_si[h1][w1]/2]), abs(bases[i].val[h1-j][w1-k]-que[h1][w1][que_si[h1][w1]/2-1]));
                            }
                        }
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
                for (int k = 0; k < que_si[i][j]-1; k++) {
                    assert(que[i][j][k] <= que[i][j][k+1]);
                }
            }
        }
        score = eval();
    }

    void polish () {
        init_que();
        while (getTime(start_cycle) < 9.5) {
            int v = engine.rand()%N;
            int hp = engine.rand()%(h-bases[v].h+1);
            int wp = engine.rand()%(w-bases[v].w+1);

            int score = 0;

            for (int h1 = ph[v]; h1 < ph[v]+bases[v].h; h1++) {
                for (int w1 = pw[v]; w1 < pw[v]+bases[v].w; w1++) {
                    if (que_si[h1][w1] == 0) continue;
                    int nowv = bases[v].val[h1-ph[v]][w1-pw[v]];
                    if (que_si[h1][w1]&1) {
                        score += abs(nowv-que[h1][w1][que_si[h1][w1]/2]);
                    } else {
                        score += min(abs(nowv-que[h1][w1][que_si[h1][w1]/2]), abs(nowv-que[h1][w1][que_si[h1][w1]/2-1]));
                    }
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
                    if (que_si[h1][w1] == 0) continue;
                    if (que_si[h1][w1]&1) {
                        nows += abs(bases[v].val[h1-hp][w1-wp]-que[h1][w1][que_si[h1][w1]/2]);
                    } else {
                        nows += min(abs(bases[v].val[h1-hp][w1-wp]-que[h1][w1][que_si[h1][w1]/2]), abs(bases[v].val[h1-hp][w1-wp]-que[h1][w1][que_si[h1][w1]/2-1]));
                    }
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
    init();

    grid_search();

    int cnt = 0;

    while (getTime(start_cycle) < 4.5) {
        cnt++;
        STATE state;
        int nh = ans.h, nw = ans.w;
        if (ans.comp_score*P > ans.loss_score*(1-P)) {
            if (engine.rand()%2) nh--;
            else nw--;
        } else {
            if (engine.rand()%2) nh++;
            else nw++;
        }
        if (engine.rand()%2) swap(nh, nw);
        nh = max(MINH, min(100, nh));
        nw = max(MINW, min(100, nw));
        state.random_init(nh, nw);
        if (state.score < ans.score) {
            ans.copy(state);
        }
    }

#ifdef LOCAL
    cerr << cnt << endl;
    cerr << ans.score << endl;
#endif
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
