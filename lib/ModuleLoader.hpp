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
#include <map>
#include <boost/algorithm/string.hpp>
#include "Utils.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"
#include "Heap.hpp"
#include "Analyser.hpp"

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
    map<string, AST> allASTs;
    vector<pair<string, string>> dependencies;
    vector<string> sortedModuleNames;

    explicit Module(string path);

private:
    void importModule(const string &path);

    string getModuleName(const string &path);

    void topologicSort();

    string getFormattedCode(string path);
};


Module::Module(string path) {
    boost::trim(path);
    this->importModule(path); //Lexer, parser, analyser

    this->topologicSort();

    // when using (import utils "path/to/utils"), we use utils to refer to the scm file in that path
    // therefore, we need to replace the alias name to its real name,
    // in order to merge all ASTs
    for (auto &[moduleName, currentAST] : allASTs) {

        for (auto &[handle, schemeObjPtr] : currentAST.heap.dataMap) {
            // only the children of these two type will be variable
            if (schemeObjPtr->schemeObjectType == SchemeObjectType::LAMBDA) {
                auto lambdaObjPtr = static_pointer_cast<LambdaObject>(schemeObjPtr);

            } else if (schemeObjPtr->schemeObjectType == SchemeObjectType::APPLICATION) {
                auto applicationObjPtr = static_pointer_cast<ApplicationObject>(schemeObjPtr);

                if (!applicationObjPtr->childrenHoses.empty() && applicationObjPtr->childrenHoses[0] != "import") {
                    // (Application of import does not need to be change name
                    for (HandleOrStr &child : applicationObjPtr->childrenHoses) {
                        if (typeOfStr(child) == Type::VARIABLE) {

                            // split the variable like 'sort' and 'Utils.sort'
                            vector<string> fields;
                            boost::split(fields, child, boost::is_any_of("."));

                            if (fields.size() >= 2) {
                                // only change the name of variables that look like Utils.sort
                                string prefix = fields.front(); //Utils
                                string suffix = boost::join(vector<string>(fields.begin() + 1, fields.end()),
                                                            ""); // sort

                                if (currentAST.moduleAliasPathMap.count(prefix)) {
                                    string moduleName = this->getModuleName(currentAST.moduleAliasPathMap[prefix]);
//                                    this->allASTs[moduleName].heap
//                                    child = newName; // child is by reference
                                }
                            }
                            //only change the name of the variable
                        }
                    }
                }
            }
        }
    }

//    let mergedModule: Module = new Module();
//    let mainModuleQualifiedName = PathUtils.GetModuleQualifiedName(path);
//    mergedModule.AST = allASTs.get(mainModuleQualifiedName);
//    // 按照依赖关系图的拓扑排序进行融合
//    // NOTE 由于AST融合是将被融合（依赖）的部分放在前面，所以这里需要逆序进行
//    for(let i = sortedModuleNames.length - 1; i >= 0; i--) {
//        let moduleName = sortedModuleNames[i];
//        if(moduleName === mainModuleQualifiedName) continue;
//        mergedModule.AST.MergeAST(allASTs.get(moduleName), "top");
//    }
//    // 编译
//    mergedModule.ILCode = Compile(mergedModule.AST);

    // mergedModule.Components = sortedModuleNames;

//    return mergedModule;

}


void Module::importModule(const string &path) {
    string code = this->getFormattedCode(path);
    string moduleName = this->getModuleName(path);

    auto tokens = Lexer::lexer(code);
    Parser parser(tokens, moduleName);
    AST currentAST = parser.parse();
    Analyser::analyse(currentAST);

    this->allASTs[moduleName] = currentAST;

    for (auto &[dependencyModuleAlias, dependencyModulePath] : currentAST.moduleAliasPathMap) {
        this->dependencies.push_back(make_pair(moduleName, getModuleName(dependencyModulePath)));
        this->importModule(dependencyModulePath);
    }

    // topologic sort the imported module in dependencies, can put the result to this->sortedModuleName
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

void Module::topologicSort() {
    map<string, vector<string>> graph;
    map<string, int> indegreeMap;

    // construct graph
    for (auto &p : this->dependencies) {
        graph[p.second].push_back(p.first);
        indegreeMap[p.first] += 1;

        // init the fromNode indegree
        if (!indegreeMap.count(p.second)) {
            indegreeMap[p.second] = 0;
        }
    }

    while (indegreeMap.size()) {
        string currentNode;
        bool noZeroIndegreeFlag = true;

        // find the node with 0 indegree
        for (auto &[node, indegree] : indegreeMap) {
            if (indegree == 0) {
                currentNode = node;
                noZeroIndegreeFlag = false;
                break;
            }
        }

        if (noZeroIndegreeFlag) {
            throw runtime_error("[Error] cannot load modules properly, due to circular dependencies between modules");
        }

        // node1 -> node2*, all node2's indegree need to be decreased by 1
        for (string &toNode : graph[currentNode]) {
            indegreeMap[toNode] -= 1;
        }

        // delete the node with 0 indegree from indegreeMap
        indegreeMap.erase(currentNode);

        //add the currentNode to the sorted list
        this->sortedModuleNames.push_back(currentNode);
    }
}

string Module::getFormattedCode(string path) {
    string code;
    // read from file
    try {
        code = utils::readFileToOneString(path);
    } catch (exception &e) {
        throw "[ERROR] module " + path + "not found";
    }

    code = "((lambda () " + code + "))\n";

    return code;

}

#endif //TYPED_SCHEME_MODULELOADER_HPP
