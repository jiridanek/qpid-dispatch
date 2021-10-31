//
// Created by Vitorio on 30.10.2021.
//

#ifndef QPID_DISPATCH_ALIASES_H
#define QPID_DISPATCH_ALIASES_H

#include <windows.h>
static inline void sleep(int seconds) {
    Sleep(seconds * 1000);
}

#endif //QPID_DISPATCH_ALIASES_H
