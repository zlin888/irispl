# typed-scheme
Yet another lisp-language. Typed-scheme in C++ VM

# User Manual

## Let
general form:
```
(let ((variable init)
      (variable init)
      (expression)))
```
lexical binding: variable will be defined can have init as default value. the scope of these varibles is inside the let 
application

## Apply
general form:
```
(apply function argument-list)
```
equivalent to:
```
(function argument0 argument1 ......)
```

