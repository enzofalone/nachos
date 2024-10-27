#include "syscall.h"

int main() {
    int s = Kill(-123);
    Exit(s);
}