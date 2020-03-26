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

struct STATE {
    int h, w;
    int val[100][100];

    int ph[100], pw[100];

    double score;
    double comp_score, loss_score;

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
                    loss_score += abs(val[nh][nw]-bases[i].val[nh-ph[i]][nw-pw[i]]);
                }
            }
        }
        loss_score /= (12.5*T);
        return comp_score * P + loss_score * (1-P);
    }

    void random_init() {
        h = engine.rand()%(100-MINH) + MINH; w = engine.rand()%(100-MINW) + MINW;
        random_init(h, w);
    }

    void random_init (int _h, int _w) {
        h = _h; w = _w;
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                val[i][j] = engine.rand()%26;
            }
        }
        for (int i = 0; i < N; i++) {
            int score = inf;
            int h_pos = 0, w_pos = 0;
            for (int j = 0; j <= h-bases[i].h; j++) {
                for (int k = 0; k <= w-bases[i].w; k++) {
                    int nows = 0;
                    for (int h1 = j; h1 < j+bases[i].h; h1++) {
                        for (int w1 = k; w1 < k+bases[i].w; w1++) {
                            nows += abs(val[h1][w1]-bases[i].val[h1-j][w1-k]);
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
        }
        score = eval();

    }

    void output() {
        cout << h << endl;
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                cout << (char)('A'+val[i][j]);
            }
            cout << endl;
        }
        for (int i = 0; i < N; i++) {
            cout << ph[i] << " " << pw[i] << endl;
        }
    }
};

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
            //cerr << state.score << " ";
        }
        //cerr << endl;
    }
}

void solve() {
    init();

    grid_search();

    for (int i = 0; i < 1000; i++) {
        STATE state;
        state.random_init();
        if (state.score < ans.score) {
            ans.copy(state);
        }
    }
}

int main() {
    input();

    solve();

    ans.output();
    //cerr << ans.score << endl;
}
