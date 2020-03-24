#include <iostream>
#include "lib/Runtime.hpp"
#include "lib/Process.hpp"
#include "lib/ModuleLoader.hpp"
#include <cstdlib>
#include "lib/REPL.hpp"

using namespace std;

int main(int argc, const char *argv[]) {

    // run script
    if(argc == 2) {
        char actualpath[PATH_MAX+1];
        realpath(argv[1], actualpath);

        Runtime runtime;

        // the executable file located in cmake-build-debug
        Module module = Module::loadModule(actualpath);

        Process process0 = runtime.createProcess(module);

        runtime.addProcess(process0);
        runtime.schedule();
//    runtime.execute(process0);
        return 0;
    }

    // REPL
    else {
        REPL::start();
    }

}
