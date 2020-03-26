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
    if (argv != 3) {
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
    in2.open("output/best.txt");
    if (!in2) {
        cerr << "failed to open file" << endl;
        return 0;
    }
    rep(i,n) in2 >> _ >> _ >> val2[i];
    in2.clear();

    ofstream ou1;
    ou1.open("output/best.txt");

    rep(i,n) {
        ou1 << "Score = " << min(val1[i], val2[i]) << endl;
    }
}