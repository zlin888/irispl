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
#include "Compiler.hpp"

using namespace std;

class ModuleLoader {
public:
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
    AST ast;
    vector<Instruction> ILCode;
    map<string, AST> allASTs;
    vector<pair<string, string>> dependencies;
    vector<string> sortedModuleNames;

    Module() {};

    static Module loadModule(string path);

private:
    void importModule(const string &path);

    static string getModuleNameFromPath(const string &path);

    void topologicSort();

    string getFormattedCode(string path);

    void makeImportedNameUnique();

};


Module Module::loadModule(string path) {
    boost::trim(path);
    Module module;
    module.importModule(path); //Lexer, parser, analyser
    module.topologicSort();
    module.makeImportedNameUnique();

    Module mergeModule;
    mergeModule.ast = module.allASTs[Module::getModuleNameFromPath(path)];

    // -1 to skip the last module
    // the top module, the module that you run, is always the last module in the sorted list
    for (int i = 0; i < module.sortedModuleNames.size() - 1; i++) {
        string moduleName = module.sortedModuleNames[i];
        mergeModule.ast.mergeAST(module.allASTs[moduleName]);
    }

    mergeModule.ILCode = Compiler::compile(mergeModule.ast);

    return mergeModule;

}


void Module::importModule(const string &path) {
    string code = this->getFormattedCode(path);
    string moduleName = this->getModuleNameFromPath(path);

    AST currentAST;

    currentAST = Parser::parse(Lexer::tokenize(code), moduleName, code);
    currentAST = Analyser::analyse(currentAST);

    this->allASTs[moduleName] = currentAST;

    for (auto &[dependencyModuleAlias, dependencyModulePath] : currentAST.moduleAliasPathMap) {
        this->dependencies.push_back(make_pair(moduleName, getModuleNameFromPath(dependencyModulePath)));
        this->importModule(dependencyModulePath);
    }

    // topologic sort the imported module in dependencies, can put the result to this->sortedModuleName
}

void Module::makeImportedNameUnique() {
    // difference between this and scope analysis:
    // scope analysis only handles the non-imported variable
    // this only handles the imported variable

    // when using (import utils "path/to/utils"), we use utils to refer to the scm file in that path
    // therefore, we need to replace the alias name to its real name,
    // in order to merge all ASTs
    // example: utils.increase -> path.to.utils.increase
    for (auto &[astModuleName, currentAST] : allASTs) {

        for (auto &[handle, schemeObjPtr] : currentAST.heap.dataMap) {
            // only the children of lambda and application will be variable

            vector<HandleOrStr> hoses;
            if (schemeObjPtr->schemeObjectType == SchemeObjectType::LAMBDA) {
                auto lambdaObjPtr = static_pointer_cast<LambdaObject>(schemeObjPtr);
                hoses = lambdaObjPtr->bodies;

            } else if (schemeObjPtr->schemeObjectType == SchemeObjectType::APPLICATION) {
                auto applicationObjPtr = static_pointer_cast<ApplicationObject>(schemeObjPtr);
                hoses = applicationObjPtr->childrenHoses;
            }

            if (!hoses.empty() && hoses[0] != "import") {
                // (Application of import does not need to be change name
                for (HandleOrStr &hos : hoses) {
                    if (typeOfStr(hos) == Type::VARIABLE) {

                        // split the variable like 'sort' and 'Utils.sort'
                        vector<string> fields;
                        boost::split(fields, hos, boost::is_any_of("."));

                        if (fields.size() >= 2) {
                            // only change the name of variables that look like Utils.sort
                            string prefix = fields.front(); //Utils, which is the alias name of the module
                            string suffix = boost::join(vector<string>(fields.begin() + 1, fields.end()),
                                                        "."); // sort

                            if (currentAST.moduleAliasPathMap.count(prefix)) {
                                string moduleName = this->getModuleNameFromPath(
                                        currentAST.moduleAliasPathMap[prefix]);
                                if (this->allASTs[moduleName].definedVarOriginUniqueNameMap.count(suffix)) {
                                    // find the unique name of this imported variable
                                    string uniqueName = this->allASTs[moduleName].definedVarOriginUniqueNameMap[suffix];
                                    hos = uniqueName;
                                } else {
                                    // cannot find the unique name of this imported variable
                                    throw std::runtime_error(
                                            "[module loader] imported variable " + hos + " in module " + astModuleName +
                                            " is not defined in " + moduleName);
                                }
                            }
                        }
                        //only change the name of the variable
                    }
                }
            }
        }
    }
}

string Module::getModuleNameFromPath(const string &path) {
    // get the last field in the path like /path/to/file
    // example: ../docs/mytest.scm -> mytest.scm
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
