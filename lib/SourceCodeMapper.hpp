//
// Created by bytedance on 2020/4/8.
//

#ifndef IRIS_SOURCECODEMAPPER_HPP
#define IRIS_SOURCECODEMAPPER_HPP

#include <map>
#include <string>
#include "Parser.hpp"


// merge source and handlesourcemap, we can use moduleName as key
// -> mapping
// handle -> moduleName : handleModuleNameMap
// moduleName -> source and handleSourceMap : moduleNameSourceMap, moduleNameHandleSourceMapMap
// build them when merging
class SourceCodeMapper {
public:
    map<Handle, string> handleModuleNameMap;
    map<string, map<Handle, int> > moduleNameHandleSourceIndexMapMap;
    map<string, string> moduleNameSourceMap;
    set<string> moduleNames;

    void addModule(string moduleName, string source);

    void setHandleSourceIndexPair(Handle handle, int index, string moduleName);

    pair<int, string> getHandleIndexAndSourceCode(Handle handle);
};

void SourceCodeMapper::addModule(string moduleName, string source) {
    if (!this->moduleNames.count(moduleName)) {
        this->moduleNames.insert(moduleName);

        map<Handle, int> handleSourceIndexesMap;
        this->moduleNameHandleSourceIndexMapMap[moduleName] = handleSourceIndexesMap;

        this->moduleNameSourceMap[moduleName] = source;
    }
}

void SourceCodeMapper::setHandleSourceIndexPair(Handle handle, int index, string moduleName) {
    if(this->moduleNames.count(moduleName)) {
        this->moduleNameHandleSourceIndexMapMap[moduleName][handle] = index;
        this->handleModuleNameMap[handle] = moduleName;
    } else {
        throw std::runtime_error("moduleName does not exist in moduleNames");
    }
}

pair<int, string> SourceCodeMapper::getHandleIndexAndSourceCode(Handle handle) {
    string moduleName = this->handleModuleNameMap[handle];
    int index = this->moduleNameHandleSourceIndexMapMap[moduleName][handle];
    string sourceCode = this->moduleNameSourceMap[moduleName];
    return pair<int, string>(index, moduleName);
}


#endif //IRIS_SOURCECODEMAPPER_HPP
