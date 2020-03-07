#include <iostream>
#include "lib/Runtime.hpp"
#include "lib/Process.hpp"
#include "lib/ModuleLoader.hpp"
#include "lib/Lexer.hpp"
#include "lib/Utils.hpp"
#include "lib/Parser.hpp"

int main() {
    Runtime runtime;

    // the executable file located in cmake-build-debug
    ModuleLoader moduleLoader("../docs/ILCode-test.txt");
    Module::loadModule("../docs/mytest.scm");



    Process process0 = runtime.createProcess(moduleLoader);

    runtime.addProcess(process0);
    runtime.schedule();
//    runtime.execute(process0);
    return 0;
}
