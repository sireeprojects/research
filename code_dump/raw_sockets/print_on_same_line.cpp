#include <iostream>
#include <unistd.h>
using namespace std;

int main() {
    printf("\e[?25l"); // hide console mouse cursor
    printf("\e[?25h"); // unhide console mouse cursor
    for (int i=0; i<100; i++) {
        cout << "\rvalue of i: "  << i ;
        fflush(stdout);
        sleep(1);
    }
    return 0;
}

// #include <iostream>
// using namespace std;
// int main()
// {
//     cout << "Hello world!!!\r";
//     cout << "Good Morning!!!";
//     return 0;
// }
