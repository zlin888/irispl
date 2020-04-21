//
// Created by bytedance on 2020/4/13.
//

#ifndef IRIS_AST_HPP
#define IRIS_AST_HPP

#include "Heap.hpp"
#include "SourceCodeMapper.hpp"

class AST {
public:
    AST() {};

    AST(const string &source, string &moduleName, string &path) : source(source), moduleName(moduleName) {
        this->sourceCodeMapper.addModule(moduleName, source, path);
    };

//    //Copy constructor, remember to change it when changing the fields list
//    AST(const AST &ast);


    Heap heap;
    string moduleName;
    string source;
    map<Handle, int> handleSourceIndexesMap;
    map<string, string> moduleAliasPathMap;
    map<string, string> natives; // Not supported yet
    map<string, string> varUniqueOriginNameMap;
    // Fort the definedVar Origin Name is unique, but for other variables,
    // origin name could be duplicated, and this is exactly why we need unique name
    map<string, string> definedVarOriginUniqueNameMap;
    vector<Handle> lambdaHandles;
    vector<Handle> tailcalls;
    SourceCodeMapper sourceCodeMapper;

    Handle getTopApplicationHandle();

    Handle getTopLambdaHandle();

    vector<Handle> getLambdaHandles();

    vector<Handle> getHandles();

    vector<HandleOrStr> getTopLambdaBodies();

    void mergeAST(AST anotherAST);

    shared_ptr<SchemeObject> get(Handle handle);

    bool isNativeCall(string nativeCall);

    void addLambdaHandle(Handle handle);

    void setHandleSourceIndexMapping(Handle handle, int index);

    Handle makeLambda(string prefix, Handle parentHandle);

    Handle makeApplication(string prefix, Handle parentHandle);

    Handle makeQuote(string prefix, Handle parentHandle);

    void deleteHandleRecursivly(HandleOrStr hos);

    void deleteHandle(HandleOrStr hos);

    Handle makeString(string prefix, Handle parentHandle);
};

Handle AST::makeLambda(string prefix, Handle parentHandle) {
    Handle lambdaHandle = this->heap.makeLambda(prefix, parentHandle);
    this->addLambdaHandle(lambdaHandle);
    this->setHandleSourceIndexMapping(lambdaHandle, this->sourceCodeMapper.getIndex(parentHandle));

    return lambdaHandle;
}

Handle AST::makeApplication(string prefix, Handle parentHandle) {
    Handle applicationHandle = this->heap.makeApplication(prefix, parentHandle);
    this->setHandleSourceIndexMapping(applicationHandle, this->sourceCodeMapper.getIndex(parentHandle));

    return applicationHandle;
}

Handle AST::makeQuote(string prefix, Handle parentHandle) {
    Handle quoteHandle = this->heap.makeQuote(prefix, parentHandle);
    this->setHandleSourceIndexMapping(quoteHandle, this->sourceCodeMapper.getIndex(parentHandle));

    return quoteHandle;
}

Handle AST::makeString(string prefix, Handle parentHandle) {
    Handle stringHandle = this->heap.makeString(prefix, parentHandle);
    this->setHandleSourceIndexMapping(stringHandle, this->sourceCodeMapper.getIndex(parentHandle));

    return stringHandle;
}

void AST::deleteHandleRecursivly(HandleOrStr hos) {
    try {
        if(typeOfStr(hos) == Type::HANDLE) {
            if(this->heap.hasHandle(hos)) {
                auto childrenHoses = SchemeObject::getChildrenHosesOrBodies(this->get(hos));
                for (auto hos : childrenHoses) {
                    this->deleteHandleRecursivly(hos);
                }
                // make sure hos still havn't deleted
                if(this->heap.hasHandle(hos)) {
                    this->heap.deleteHandle(hos);
                }
            }
        }
    } catch (exception &e) {
        //pass
    }
}

void AST::deleteHandle(HandleOrStr hos) {
    if(typeOfStr(hos) == Type::HANDLE) {
        if(this->heap.hasHandle(hos)) {
            this->heap.deleteHandle(hos);
        }
    }
}

void AST::setHandleSourceIndexMapping(Handle handle, int index) {
    this->sourceCodeMapper.setHandleSourceIndexMapping(handle, index, this->moduleName);
    this->handleSourceIndexesMap[handle] = index;
}


shared_ptr<SchemeObject> AST::get(Handle handle) {
    try {
        return this->heap.get(handle);
    } catch (exception &e){
        throw e;
    }
}

Handle AST::getTopApplicationHandle() {
    // ((lambda () " + code + "))
    // the first sList
    Handle topApplicationHandle;
    for (auto &[handle, schemeObjPtr] : this->heap.dataMap) {
        if (schemeObjPtr->schemeObjectType == SchemeObjectType::APPLICATION &&
            static_pointer_cast<ApplicationObject>(schemeObjPtr)->parentHandle == TOP_NODE_HANDLE) {
            topApplicationHandle = handle;
            break;
        }
    }
    if (topApplicationHandle.empty()) {
        throw runtime_error("topApplicationHandle not found");
    }
    return topApplicationHandle;
}


Handle AST::getTopLambdaHandle() {
    // ((lambda () " + code + "))
    // return the first lambda, which is the top lambda
    auto topAppObjPtr = static_pointer_cast<ApplicationObject>(this->heap.get(this->getTopApplicationHandle()));
    return topAppObjPtr->childrenHoses[0];
}

vector<HandleOrStr> AST::getTopLambdaBodies() {
    // return by value !!!!!!!!!!!!!!
    Handle topLambdaHandle = this->getTopLambdaHandle();
    auto topLambdaObjptr = static_pointer_cast<LambdaObject>(this->heap.get(topLambdaHandle));
    return topLambdaObjptr->bodies;
}


// merge source and handlesourcemap, we can use moduleName as key
// -> mapping
// handle -> moduleName : handleModuleNameMap
// moduleName -> source and handleSourceMap : moduleNameSourceMap, moduleNameHandleSourceMapMap
// build them when merging
void AST::mergeAST(AST anotherAST) {

//    this->source = anotherAST.source + "\n" + this->source;
    this->sourceCodeMapper.merge(anotherAST.sourceCodeMapper);



    // merge the top_node_lambda's body
    // the top_node_lambda's body (lambda () (*))
    // we only need to lambda's body because the top lambda not has any argument
    Handle thisTopLambdaHandle = this->getTopLambdaHandle();
    Handle thisTopApplicationHandle = this->getTopApplicationHandle();
    vector<HandleOrStr> thisTopLambdaBodies = this->getTopLambdaBodies(); // return by value !!!!

    Handle anotherTopLambdaHandle = anotherAST.getTopLambdaHandle();
    Handle anotherTopApplicationHandle = anotherAST.getTopApplicationHandle();
    vector<HandleOrStr> anotherTopLambdaBodies = anotherAST.getTopLambdaBodies();

    // merge heap
    for (auto &[handle, schemeObjPtr] : anotherAST.heap.dataMap) {
        this->heap.set(handle, schemeObjPtr);
    }

    // append the thisTopLambdaBodies to the otherTopLambdaBodies
    anotherTopLambdaBodies.insert(anotherTopLambdaBodies.end(), thisTopLambdaBodies.begin(), thisTopLambdaBodies.end());

    auto thisTopLambdaObjPtr = static_pointer_cast<LambdaObject>(this->heap.get(thisTopLambdaHandle));
    thisTopLambdaObjPtr->setBodies(anotherTopLambdaBodies);

    // Now we have, this ((lambda () (this bodies + anthoer bodies))
    // Then we need to change other's bodies' handles' parentHandle to thisLambda
    for (auto &handle : anotherTopLambdaBodies) {
        // only handle exists in the bodies
        this->heap.get(handle)->parentHandle = thisTopLambdaHandle;
    }

    // we add them to the heap of thisAST
    this->heap.deleteHandle(anotherTopLambdaHandle);
    this->heap.deleteHandle(anotherTopApplicationHandle);

    // the heap have been merge, but not the other fields of ast
    // merge each field one by one
    for (auto[source, index] : this->handleSourceIndexesMap) {
        this->handleSourceIndexesMap[source] = index + anotherAST.source.size() + 1;
    }

    for (auto[source, index] : anotherAST.handleSourceIndexesMap) {
        this->handleSourceIndexesMap[source] = index;
    }

    for (auto lambdaHandle : anotherAST.getLambdaHandles()) {
        if (lambdaHandle != anotherTopLambdaHandle) {
            this->lambdaHandles.push_back(lambdaHandle);
        }
    }

    for (auto handle : anotherAST.tailcalls) {
        if (handle != anotherTopLambdaHandle) {
            this->tailcalls.push_back(handle);
        }
    }

    for (auto[unique, origin]: anotherAST.varUniqueOriginNameMap) {
        this->varUniqueOriginNameMap[unique] = origin;
    }

    for (auto[origin, unique] : anotherAST.definedVarOriginUniqueNameMap) {
        this->definedVarOriginUniqueNameMap[origin] = unique;
    }

    for (auto[alias, path] : anotherAST.moduleAliasPathMap) {
        this->moduleAliasPathMap[alias] = path;
    }

    for (auto[s1, s2] : anotherAST.natives) {
        this->natives[s1] = s2;
    }


}

vector<Handle> AST::getLambdaHandles() {
    vector<Handle> lambdaHandles;
    for (auto &[handle, schemeObjPtr] : this->heap.dataMap) {
        if (schemeObjPtr->schemeObjectType == SchemeObjectType::LAMBDA) {
            lambdaHandles.push_back(handle);
        }
    }
    this->lambdaHandles = lambdaHandles;
    return lambdaHandles;
}

vector<Handle> AST::getHandles() {
    vector<Handle> result;
    for (auto &[handle, _] : this->heap.dataMap) {
        result.push_back(handle);
    }
    return result;
}

bool AST::isNativeCall(string nativeCall) {
    // split the variable like 'native' and 'native.sort'
    vector<string> fields;
    boost::split(fields, nativeCall, boost::is_any_of("."));

    if(!fields.empty()) {
        return this->natives.count(fields[0]);
    } else {
        return false;
    }

}

void AST::addLambdaHandle(Handle handle) {
    this->lambdaHandles.push_back(handle);
}


#endif //IRIS_AST_HPP
