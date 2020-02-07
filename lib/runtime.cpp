#include "runtime.hpp"
#include "process.hpp"

#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <iostream>

using namespace std;

int main() {
    Runtime runtime;
    Process process0 = runtime.createProcess();
    runtime.addProcess(process0);
    runtime.execute(process0);
    cout << "hi";
    return 0;
};