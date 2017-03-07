#include "common.h"
#include "millis.h"
extern void setup(void);
extern void loop(void);

int main(void) {
    Corbot::initialize_millis();
    setup();
    while (1) {
        loop();
    }
}
