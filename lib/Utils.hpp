//
// Created by Zhitao Lin on 2020/2/7.
//

#ifndef TYPED_SCHEME_UTILS_HPP
#define TYPED_SCHEME_UTILS_HPP

#include <stdexcept>
#include <fstream>
#include <vector>

using namespace std;

namespace utils {

    inline std::string trim(const std::string &s){
        auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
        auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
        return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
    }

    string readFileToOneString(const string& path) {
        std::fstream fs(path);
        string resultStr;

        if(fs.is_open()){
            std::string line;
            while (std::getline(fs, line)) {
                resultStr += line + '\n';
            }
            fs.close();
        } else {
            throw std::runtime_error("file not file");
        }

        return resultStr;
    }

}

#endif //TYPED_SCHEME_UTILS_HPP
