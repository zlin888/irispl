//
// Created by bytedance on 2020/3/23.
//

#ifndef TYPED_SCHEME_REPL_HPP
#define TYPED_SCHEME_REPL_HPP

#include <vector>
#include <string>
#include <exception>
#include "Runtime.hpp"

using namespace std;

class REPL {
public:
    string allCode;
    vector<string> inputBuffer;
    Runtime runtime;

    int indent_level;

    static void start();

    bool inputBufferIsFinished();

    int getIndentLevel();

    string bufferToString();

    bool inputBufferHasSideEffect();

    string addDisplay(string code);
};

bool REPL::inputBufferIsFinished() {
    int left_bracket_counter = 0;
    int right_bracket_counter = 0;
    for (auto s : this->inputBuffer) {
        for (auto c : s) {
            if (c == '(') {
                left_bracket_counter++;
            }
            if (c == ')') {
                right_bracket_counter++;
            }
        }
    }

    return left_bracket_counter == right_bracket_counter;
}

int REPL::getIndentLevel() {
    int left_bracket_counter = 0;
    int right_bracket_counter = 0;
    for (auto s : this->inputBuffer) {
        for (auto c : s) {
            if (c == '(') {
                left_bracket_counter++;
            }
            if (c == ')') {
                right_bracket_counter++;
            }
        }
    }

    return left_bracket_counter - right_bracket_counter;
}

string REPL::bufferToString() {
    string resultStr = "";
    for (auto s : this->inputBuffer) {
        resultStr += s + "\n";
    }
    return resultStr;
}

string REPL::addDisplay(string code) {
    string verifyCode;
    for (auto c : code) {
        if (c != ' ') {
            verifyCode.push_back(c);
        }
    }

    if (!verifyCode.starts_with("(display")) {
        return "(display " + code + ")";
    } else {
        return code;
    }

}


void REPL::start() {
    cout << "Iris REPL v1.0.0" << endl;
    cout << "> ";


    REPL repl;
    for (std::string line; std::getline(std::cin, line);) {
        repl.inputBuffer.push_back(line);

        if (repl.inputBufferIsFinished()) {

            try {
                string inputCode = repl.bufferToString();
                if(!repl.inputBufferHasSideEffect()) {
                    inputCode = repl.addDisplay(inputCode);

                }
                Module module = Module::loadModuleFromCode(repl.allCode + inputCode);

                Runtime runtime(OutputMode::BUFFERED);
                Process process0 = runtime.createProcess(module);
                runtime.addProcess(process0);
                runtime.schedule();

                //flush
                for (auto s : runtime.outputBuffer) {
                    cout << s << endl;
                }

                if (repl.inputBufferHasSideEffect()) {
                    repl.allCode += repl.bufferToString();
                }
            } catch (exception &e) {
                cout << e.what() << endl;
            }

            cout << "> ";
            repl.inputBuffer.clear();

        } else {
            int indentLevel = repl.getIndentLevel();
            for (int i = 0; i < indentLevel; i++) {
                cout << "...";
            }
            cout << " ";
        }


    }
}

bool REPL::inputBufferHasSideEffect() {
    for (auto s : this->inputBuffer) {
        // find a better way to put this
        if (std::regex_match(s, std::regex(".*define.*|.*set!.*|.*import.*|.*native.*"))) {
            return true;
        }
    }

    return false;
}


#endif //TYPED_SCHEME_REPL_HPP
