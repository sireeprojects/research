#include <iostream>
#include <thread>
#include <string>
#include <cstdint>
#include <chrono>
#include <sstream>
#include <iomanip>

using namespace std;
using namespace chrono;

class cdn_enet_avip_timer {
public:
    cdn_enet_avip_timer() = default;
    void start();
    void stop();
    double get_delta();
    string delta_in_string(int precision=2);
private:
    time_point<high_resolution_clock> begin;
    time_point<high_resolution_clock> end;
};

void cdn_enet_avip_timer::start() {
   begin = high_resolution_clock::now();
}

void cdn_enet_avip_timer::stop() {
    end = high_resolution_clock::now();
}

double cdn_enet_avip_timer::get_delta() {
    double delta = duration<double>(end-begin).count();
    return delta;
}

string cdn_enet_avip_timer::delta_in_string(int precision) {
    stringstream ss;
    ss << fixed << setprecision(precision) << get_delta() << " sec";
    return ss.str();
}

/*

int main() {
   cdn_enet_avip_timer stopwatch;
   stopwatch.start();
   this_thread::sleep_for(chrono::milliseconds(1500)); // wait 1.5 sec OR do something
   stopwatch.stop();
   cout << stopwatch.delta_in_string() << endl; // shud print 1.5 sec
}



*/
