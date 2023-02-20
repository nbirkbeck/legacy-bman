
#include "compat.h"
#include "globals.h"
#include <stdio.h>
#ifndef MYTIMER_H
#define MYTIMER_H

extern FILE* debug;

class MyTimer {
private:
  int startTime;

public:
  MyTimer(void) {}
  void start() { startTime = GetTickCount(); }
  void wait(DWORD timeToWait) {
    int temp;

    temp = timeToWait - (GetTickCount() - startTime);
    if (temp >= 2)
      Sleep(temp);
  }
};

#endif
