#include <iostream>
#include <numeric>
#include <vector>

#include <chrono>
#include <cstring>

#include "driver_config.hpp"

using Ticker = void (*)(float, float*);

void automatic_tick(float t, float* screen);

void pure_simd_tick(float t, float* screen);

void intrinsic_tick(float t, float* screen);

void drive(const char* name, Ticker ticker, unsigned epochs);

using namespace std;

int main(int argc, char** argv)
{
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <iteration-times>" << endl;
        exit(-1);
    }

    unsigned itimes = atoi(argv[1]);

    drive("automatic", &automatic_tick, itimes);

    drive("pure_simd", &pure_simd_tick, itimes);

    drive("intrinsic", &intrinsic_tick, itimes);
}

void drive(const char* name, Ticker ticker, unsigned itimes)
{
    using namespace chrono;

    vector<float> screen(SCRWIDTH * SCRHEIGHT, 0.0);

    float scale = 0.0;

    for (unsigned i = 0; i < itimes; ++i) {
        auto start = steady_clock::now();

        scale += i / 10.0;
        ticker(scale, screen.data());

        auto done = steady_clock::now();
        unsigned millis = duration_cast<milliseconds>(done - start).count();
        cout << name << " cost " << millis << "ms" << endl;
    }

    cout << name << " sum = " << accumulate(screen.begin(), screen.end(), 0.0) << endl;
}
