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

int main() {
    string str;
    int cnt = 0;
    while (getline(cin, str)) {
        cnt++;
        if (cnt%2 != 1) continue;
        cout << str << endl;
    }
}