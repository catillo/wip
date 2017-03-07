#pragma once
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>

extern unsigned long millis();

namespace Corbot {
  class SerialType {
  public:
    static void begin(int) {};
    static void print(const char*) {};
    static void print(unsigned long) {};
  };
}
#define delay(a) sleep(a/1000)
extern Corbot::SerialType Serial;
