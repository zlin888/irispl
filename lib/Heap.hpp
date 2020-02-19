//
// Created by bytedance on 2020/2/18.
//

#ifndef TYPED_SCHEME_HEAP_HPP
#define TYPED_SCHEME_HEAP_HPP

#include "SchemeObject.hpp"

typedef string Handle;
const Handle TOP_NODE_HANDLE = "&TOP_NODE";

// SchemeObjects in the Heap are referred as node
class Heap {
public:
    map<Handle, std::shared_ptr<SchemeObject>> dataMap;
    int handleCounter = 0;

    bool hasHandle(Handle handle);

    shared_ptr<SchemeObject> get(Handle handle);

    void set(Handle handle, shared_ptr<SchemeObject> schemeObjectPtr);

    Handle allocateHandle(SchemeObjectType schemeObjectType);

    Handle allocateHandle(const string &prefix, SchemeObjectType schemeObjectType);

    Handle makeLambda(const string &prefix, Handle parentHandle);

    Handle makeApplication(const string &prefix, Handle parentHandle, const string &quoteType);

    Handle makeString(const string &prefix, string content);
};


bool Heap::hasHandle(Handle handle) {
    return this->dataMap.count(handle);
}

std::shared_ptr<SchemeObject> Heap::get(Handle handle) {
    if (this->hasHandle(handle)) {
        return this->dataMap[handle];
    } else {
        throw std::out_of_range("[ERROR] handle holds nothing -- Heap::get");
    }
}

void Heap::set(Handle handle, std::shared_ptr<SchemeObject> schemeObjectPtr) {
    this->dataMap[handle] = schemeObjectPtr;
}

Handle Heap::allocateHandle(SchemeObjectType schemeObjectType) {
    Handle handle = "&" + SchemeObjectTypeStrMap[schemeObjectType] + "_" + to_string(this->handleCounter);
    this->handleCounter++;
    this->dataMap.emplace(handle, nullptr);
    return handle;
}

Handle Heap::allocateHandle(const string &prefix, SchemeObjectType schemeObjectType) {
    Handle handle =
            "&" + prefix + "." + SchemeObjectTypeStrMap[schemeObjectType] + "_" + to_string(this->handleCounter);
    this->handleCounter++;
    this->dataMap.emplace(handle, nullptr);
    return handle;
}

Handle Heap::makeLambda(const string &prefix, Handle parentHandle) {
    // NOTE 每个节点把柄都带有模块全限定名，这样做的目的是：不必在AST融合过程中调整每个AST的把柄。下同。
    Handle handle = this->allocateHandle(prefix, SchemeObjectType::LAMBDA);
    this->set(handle, std::shared_ptr<LambdaObject>(new LambdaObject(parentHandle)));
    return handle;
}

Handle Heap::makeApplication(const string &prefix, Handle parentHandle, const string &quoteType) {
    string handle;
    if (quoteType == "QUOTE") {
        handle = this->allocateHandle(prefix, SchemeObjectType::QUOTE);
        this->set(handle, std::shared_ptr<QuoteObject>(new QuoteObject(parentHandle)));
    } else if (quoteType == "QUASIQUOTE") {
        handle = this->allocateHandle(prefix, SchemeObjectType::QUASIQUOTE);
        this->set(handle, std::shared_ptr<QuasiquoteObject>(new QuasiquoteObject(parentHandle)));
    } else if (quoteType == "UNQUOTE") {
        handle = this->allocateHandle(prefix, SchemeObjectType::UNQUOTE);
        this->set(handle, std::shared_ptr<UnquoteObject>(new UnquoteObject(parentHandle)));
    } else {
        handle = this->allocateHandle(prefix, SchemeObjectType::APPLICATION);
        this->set(handle, std::shared_ptr<ApplicationObject>(new ApplicationObject(parentHandle)));
    }
    return handle;
}

Handle Heap::makeString(const string &prefix, string content) {
    Handle handle = this->allocateHandle(prefix, SchemeObjectType::STRING);
    this->set(handle, std::shared_ptr<StringObject>(new StringObject(content)));
    return handle;
}

#endif //TYPED_SCHEME_HEAP_HPP
