//
// Created by Zhitao Lin on 2020/2/7.
//

#ifndef TYPED_SCHEME_MODULELOADER_HPP
#define TYPED_SCHEME_MODULELOADER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <iostream>

using namespace std;

class ModuleLoader {
    public:
        string AVM_Version= "V0"; // AVM version that is used
            // public Components: Array<string>;    // 组成该模块的各个依赖模块名称的拓扑排序序列
    //        public AST: AST;
            vector<string> ILCode;

        explicit ModuleLoader(const string &path);
};

ModuleLoader::ModuleLoader(const string& path){
    std::fstream fs(path);
    if(fs.is_open()){
        std::string line;
        while (std::getline(fs, line)) {
            // using printf() in all tests for consistency
            this->ILCode.push_back(line);
        }
        fs.close();
    }
}

#endif //TYPED_SCHEME_MODULELOADER_HPP
