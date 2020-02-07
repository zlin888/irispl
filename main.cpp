#include <iostream>
#include "lib/runtime.hpp"
#include "lib/process.hpp"

int main() {
    Runtime runtime;
    Process process0 = runtime.createProcess();
    runtime.addProcess(process0);
    runtime.execute(process0);
    return 0;
}
