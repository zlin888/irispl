//
// Created by Zhitao Lin on 2020/2/7.
//

#ifndef TYPED_SCHEME_MODULELOADER_HPP
#define TYPED_SCHEME_MODULELOADER_HPP

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <regex>
#include <boost/algorithm/string.hpp>
#include "Utils.hpp"

using namespace std;

class ModuleLoader {
public:
    string AVM_Version = "V0"; // AVM version that is used
    // public Components: Array<string>;    // 组成该模块的各个依赖模块名称的拓扑排序序列
    //        public AST: AST;
    vector<string> ILCode;

    explicit ModuleLoader(const string &path);
};

ModuleLoader::ModuleLoader(const string &path) {
    std::fstream fs(path);
    if (fs.is_open()) {
        std::string line;
        while (std::getline(fs, line)) {
            // using printf() in all tests for consistency
            this->ILCode.push_back(line);
        }
        fs.close();
    }
};

class Module {
public:
    string AVM_Version = "V0"; // AVM version that is used
    // public Components: Array<string>;    // 组成该模块的各个依赖模块名称的拓扑排序序列
    //    public AST: AST;
    vector<string> ILCode;

    explicit Module(string path);

private:
    void importModule(const string &path);

    string getModuleName(const string &path);
};


Module::Module(string path) {
    boost::trim(path);
    this->importModule(path);
}


void Module::importModule(const string &path) {
    string code;
    try {
        code = utils::readFileToOneString(path);
    } catch (exception &e) {
        throw "[ERROR] module " + path + "not found";
    }

    code = "((lambda () " + code + "))\n";

    string moduleName = this->getModuleName(path);

//    let currentAST = Analyse(Parse(code, moduleQualifiedName));
}

string Module::getModuleName(const string &path) {
//    vector<string> fields = utils::split(path, regex(R"([/\\\\])"));
    vector<string> fields;
    boost::split(fields, path, boost::is_any_of("\\/"));
    string moduleName;
    if (!fields.empty()) {
        moduleName = fields.back();
    }
    return moduleName;
//        moduleFileName.replace(/\.[^.]*$/gi, "");
}

#endif //TYPED_SCHEME_MODULELOADER_HPP
