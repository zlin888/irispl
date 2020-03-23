//
// Created by bytedance on 2020/3/23.
//

#ifndef TYPED_SCHEME_REPL_HPP
#define TYPED_SCHEME_REPL_HPP

#include <vector>
#include <string>
#include "Runtime.hpp"

using namespace std;

class REPL {
public:
    vector<string> allCode;
    vector<string> inputBuffer;
    Runtime runtime;



};

void REPL::start() {
    cout << "Iris REPL v1.0.0" << endl;
    cout << "> ";

}


#endif //TYPED_SCHEME_REPL_HPP
