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
    map<string, string> moduleNamePathMap;
    set<string> moduleNames;

    void addModule(string moduleName, string source, string path);

    void setHandleSourceIndexMapping(Handle handle, int index, string moduleName);

    int getIndex(Handle handle);
    string getSourceCode(Handle handle);

    void merge(SourceCodeMapper anotherSourceCodeMapper);

    string getModuleName(Handle handle);

    string getPath(Handle handle);
};

void SourceCodeMapper::addModule(string moduleName, string source, string path) {
    if (!this->moduleNames.count(moduleName)) {
        this->moduleNames.insert(moduleName);

        map<Handle, int> handleSourceIndexesMap;
        this->moduleNameHandleSourceIndexMapMap[moduleName] = handleSourceIndexesMap;

        this->moduleNameSourceMap[moduleName] = source;
        this->moduleNamePathMap[moduleName] = path;
    }
}

void SourceCodeMapper::setHandleSourceIndexMapping(Handle handle, int index, string moduleName) {
    if(this->moduleNames.count(moduleName)) {
        this->moduleNameHandleSourceIndexMapMap[moduleName][handle] = index;
        this->handleModuleNameMap[handle] = moduleName;
    } else {
        throw std::runtime_error("moduleName does not exist in moduleNames");
    }
}

int SourceCodeMapper::getIndex(Handle handle) {
    string moduleName = this->handleModuleNameMap[handle];
    int index = this->moduleNameHandleSourceIndexMapMap[moduleName][handle];
    return index;
}

string SourceCodeMapper::getSourceCode(Handle handle) {
    string moduleName = this->handleModuleNameMap[handle];
    string sourceCode = this->moduleNameSourceMap[moduleName];
    return sourceCode;
}

string SourceCodeMapper::getModuleName(Handle handle) {
    string moduleName = this->handleModuleNameMap[handle];
    return moduleName;
}

string SourceCodeMapper::getPath(Handle handle) {
    string moduleName = this->handleModuleNameMap[handle];
    string path = this->moduleNamePathMap[moduleName];
    return path;
}

void SourceCodeMapper::merge(SourceCodeMapper anotherSourceCodeMapper) {

    // merge moduleName sets
    for (auto moduleName : anotherSourceCodeMapper.moduleNames) {
        this->moduleNames.insert(moduleName);
    }

    // merge handleModuleNameMap
    for (auto [handle, moduleName] : anotherSourceCodeMapper.handleModuleNameMap) {
        this->handleModuleNameMap[handle] = moduleName;
    }

    // merge moduleNameHandleSourceIndexMapMap
    for (auto [moduleName, handleSourceIndexMap] : anotherSourceCodeMapper.moduleNameHandleSourceIndexMapMap) {
        this->moduleNameHandleSourceIndexMapMap[moduleName] = handleSourceIndexMap;
    }

    // merge moduleNameSourceMap
    for (auto [moduleName, source] : anotherSourceCodeMapper.moduleNameSourceMap) {
        this->moduleNameSourceMap[moduleName] = source;
    }

    // merge modulePathMap
    for (auto [moduleName, path] : anotherSourceCodeMapper.moduleNameSourceMap) {
        this->moduleNamePathMap[moduleName] = path;
    }
}


#endif //IRIS_SOURCECODEMAPPER_HPP
