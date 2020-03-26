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
#include <fstream>
using namespace std;
#define rep(i,n) for(int i = 0; i < n; i++)

int main(int argv, char** argc) {
    if (argv != 4) {
        cerr << "wrong parameters" << endl;
        return 0;
    }
    int n = atoi(argc[1]);
    vector<double> val1(n), val2(n);

    ifstream in1;
    string arg2(argc[2]);
    in1.open("output/"+arg2+".txt");
    if (!in1) {
        cerr << "failed to open file" << endl;
        return 0;
    }
    string _;
    rep(i,n) in1 >> _ >> _ >> val1[i];
    in1.clear();

    ifstream in2;
    string arg3(argc[3]);
    in2.open("output/"+arg3+".txt");
    if (!in2) {
        cerr << "failed to open file" << endl;
        return 0;
    }
    rep(i,n) in2 >> _ >> _ >> val2[i];
    in2.clear();

    double score[2] = {0.0, 0.0};
    rep(i,n) {
        if (val1[i] <= 0 && val2[i] <= 0) continue;
        if (val1[i] <= 0) {
            score[1] += 1.0;
            continue;
        }
        if (val2[i] <= 0) {
            score[0] += 1.0;
            continue;
        }
        if (val1[i] > val2[i]) {
            score[0] += val2[i] / val1[i];
            score[1] += 1.0;
        } else {
            score[0] += 1.0;
            score[1] += val1[i] / val2[i];
        }
        cerr << i << " " << val2[i]/val1[i] << endl;
    }
    cout << score[0]*100/(double)n << " " << score[1]*100/(double)n << endl;
}