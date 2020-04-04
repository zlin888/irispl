# typed-scheme
Yet another lisp-language. Typed-scheme in C++ VM

# User Manual

## Let
lexical binding: variable will be defined can have init as default value. the scope of these varibles is inside the let 
application
```
(let ((variable init)
      (variable init)
      (expression)))
```

## Function Arbitray Arguments Supported
general form:
```
(lambda (arg0 arg1 . agrs)
    (use-agrs-as-list))
```

## Apply
```
(apply function argument-list)
```
equivalent to:
```
(function arg0 arg1 ... argX)
```


# Develop Manual
## some bullets 
* Compile and VM push-store principle: always push inversely and store in order
