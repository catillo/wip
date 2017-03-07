#include "common.h"

extern "C" { 
    void setup(void);
    void loop(void);
}

int main(void) {
    setup();
    while (1) {
        loop();
    }
}
