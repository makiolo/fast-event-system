# Fast Event System
[![npm version](https://badge.fury.io/js/fast-event-system.svg)](https://badge.fury.io/js/fast-event-system) [![state dependencies](https://david-dm.org/makiolo/fast-event-system.svg)](https://david-dm.org/makiolo/fast-event-system) [![state dev dependencies](https://david-dm.org/makiolo/fast-event-system/dev-status.svg)](https://david-dm.org/makiolo/fast-event-system?type=dev)

Coverage: [![codecov](https://codecov.io/gh/makiolo/fast-event-system/branch/master/graph/badge.svg)](https://codecov.io/gh/makiolo/fast-event-system)

gcc 4.9 / clang 3.6: [![Build Status](https://travis-ci.org/makiolo/fast-event-system.svg?branch=master)](https://travis-ci.org/makiolo/fast-event-system)

MSVC 2015: [![Build status](https://ci.appveyor.com/api/projects/status/oatw9c59rlbwa98t?svg=true)](https://ci.appveyor.com/project/makiolo/fast-event-system)

# quick-start
```bash
$ git clone --recursive git@github.com:makiolo/fast-event-system.git fes
$ cd fes
$ npm install
$ npm test
```

# Very simple to use
Fast event system is a library for resolve observer pattern in a functional way. Is a library very easy to use, only have 3 objects: *sync*, *async_fast* and *async_delay*.
## sync
I will explain sync object with minimal examples:
```cpp
// instanciate
fes::sync<bool> key_A;
```
Now, you can connect functors, lambdas ...:
```cpp
key_A.connect([](bool press)
	{
		if(press)
			std::cout << "pressed key A" << std::endl;
		else
			std::cout << "released key A" << std::endl;
	});
```
And finally, something notify using operator() and all functors connected will receive this.
```cpp
key_A(true);  // notify to subscribers
```
All objetcs in fes, use variadic templates, new interfaces can be created in compile time:
```cpp
fes::sync<std::string, int, std::string> civilian;
civilian.connect([](const std::string& name, int age, const std::string& country)
	{
		std::cout << "new civilian registered" << std::endl;
	});
```
## async_fast
Works equal than sync but data is saved in buffer.
```cpp
key_A(true);  // saved in queue
key_A(true);  // saved in queue
key_A(true);  // saved in queue
```
Now, we have three messages waiting in queue. For dispatching, type:
```cpp
key_A.update();  // notify to subscribers
```
## async_delay
Works equal than async_fast but data can send delayed.
The time delayed is specified in first parameter of operator() with help of fes::deltatime():
```cpp
key_A(fes::deltatime(2000), true);  // saved in queue (with your delayed time)
```
We can use .update() for dispatching, now have a parameter for config timeout:
```cpp
// 2000 ms after, message is notify to subscribers
key_A.update(fes::deltatime(5000));
```
Previous line, wait 5000 ms or finish when one message is dispatched. For default, timeout is 16ms.
For dispatching a fixed time(dispathing multiples messages), you can use .fortime():
```cpp
key_A.fortime(fes::deltatime(5000));
```
### Documentation under construction
