//
// Created by bytedance on 2020/2/18.
//

#ifndef TYPED_SCHEME_HEAP_HPP
#define TYPED_SCHEME_HEAP_HPP

#include "IrisObject.hpp"

typedef string Handle;
const Handle TOP_NODE_HANDLE = "&TOP_NODE";

// SchemeObjects in the Heap are referred as node
class Heap {
public:
    map<Handle, std::shared_ptr<IrisObject>> dataMap;
    int handleCounter = 0;

    bool hasHandle(Handle handle);

    void deleteHandle(Handle handle);

    shared_ptr<IrisObject> get(Handle handle);

    void set(Handle handle, shared_ptr<IrisObject> schemeObjectPtr);

    Handle allocateHandle(IrisObjectType schemeObjectType);

    Handle allocateHandle(const string &prefix, IrisObjectType schemeObjectType);

    Handle makeLambda(const string &prefix, Handle parentHandle);

    Handle makeString(const string &prefix, string content);

    Handle makeApplication(const string &prefix, Handle parentHandle);

    Handle makeQuote(const string &prefix, Handle parentHandle);

    Handle makeUnquote(const string &prefix, Handle parentHandle);

    Handle makeQuasiquote(const string &prefix, Handle parentHandle);

    Handle makeList(const string &prefix, Handle parentHandle);
};


bool Heap::hasHandle(Handle handle) {
    return this->dataMap.count(handle);
}

void Heap::deleteHandle(Handle handle) {
    this->dataMap.erase(handle);
}

std::shared_ptr<IrisObject> Heap::get(Handle handle) {
    if (this->hasHandle(handle)) {
        return this->dataMap[handle];
    } else {
        throw std::runtime_error("[ERROR] handle holds nothing -- Heap::get");
    }
}

void Heap::set(Handle handle, std::shared_ptr<IrisObject> schemeObjectPtr) {
    this->dataMap[handle] = schemeObjectPtr;
}

Handle Heap::allocateHandle(IrisObjectType schemeObjectType) {
    Handle handle = "&" + IrisObjectTypeStrMap[schemeObjectType] + "_" + to_string(this->handleCounter);
    this->handleCounter++;
    this->dataMap.emplace(handle, nullptr);
    return handle;
}

Handle Heap::allocateHandle(const string &prefix, IrisObjectType schemeObjectType) {
    Handle handle =
            "&" + prefix + "." + IrisObjectTypeStrMap[schemeObjectType] + "_" + to_string(this->handleCounter);
    this->handleCounter++;
    this->dataMap.emplace(handle, nullptr);
    return handle;
}

Handle Heap::makeLambda(const string &prefix, Handle parentHandle) {
    Handle handle = this->allocateHandle(prefix, IrisObjectType::LAMBDA);
    this->set(handle, std::shared_ptr<LambdaObject>(new LambdaObject(parentHandle, handle)));
    return handle;
}

Handle Heap::makeApplication(const string &prefix, Handle parentHandle) {
    string handle = this->allocateHandle(prefix, IrisObjectType::APPLICATION);
    this->set(handle, std::shared_ptr<ApplicationObject>(new ApplicationObject(parentHandle, handle)));
    return handle;
}

Handle Heap::makeList(const string &prefix, Handle parentHandle) {
    string handle = this->allocateHandle(prefix, IrisObjectType::LIST);
    this->set(handle, std::shared_ptr<ListObject>(new ListObject(parentHandle, handle)));
    shared_ptr<ListObject> listObjPtr = static_pointer_cast<ListObject>(this->get(handle));
    listObjPtr->realListObjPtr = listObjPtr;
    return handle;
}

Handle Heap::makeQuote(const string &prefix, Handle parentHandle) {
    string handle = this->allocateHandle(prefix, IrisObjectType::QUOTE);
    this->set(handle, std::shared_ptr<QuoteObject>(new QuoteObject(parentHandle, handle)));
    return handle;
}

Handle Heap::makeUnquote(const string &prefix, Handle parentHandle) {
    string handle = this->allocateHandle(prefix, IrisObjectType::UNQUOTE);
    this->set(handle, std::shared_ptr<UnquoteObject>(new UnquoteObject(parentHandle, handle)));
    return handle;
}

Handle Heap::makeQuasiquote(const string &prefix, Handle parentHandle) {
    string handle = this->allocateHandle(prefix, IrisObjectType::QUASIQUOTE);
    this->set(handle, std::shared_ptr<QuasiquoteObject>(new QuasiquoteObject(parentHandle, handle)));
    return handle;
}


Handle Heap::makeString(const string &prefix, string content) {
    Handle handle = this->allocateHandle(prefix, IrisObjectType::STRING);
    this->set(handle, std::shared_ptr<StringObject>(new StringObject(content)));
    return handle;
}

#endif //TYPED_SCHEME_HEAP_HPP
