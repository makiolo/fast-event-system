# Fast Event System [![Build Status](https://travis-ci.org/makiolo/fast-event-system.svg?branch=master)](https://travis-ci.org/makiolo/fast-event-system)

This fast event system allows calls between two interfaces decoupled (sync or async)

## Design guidelines in fast-event-system

* Can assume variadic templates supported by your compiler.
* Use perfect forwarding and new features from C++11/14 when possible.
* Prefer header only code, but it is not a must.

## Quality assurance

* Code tested in travis on gcc (4.7, 4.8, 4.9), clang (3.3, 3.4 and 3.6) and Visual Studio (2013).
* Test cases relationated with problems crossing boundaries of dynamic libraries.

## License

<a rel="license" href="http://creativecommons.org/licenses/by/4.0/"><img alt="Licencia de Creative Commons" style="border-width:0" src="https://i.creativecommons.org/l/by/4.0/88x31.png" /></a><br /><span xmlns:dct="http://purl.org/dc/terms/" href="http://purl.org/dc/dcmitype/Text" property="dct:title" rel="dct:type">fast-event-system</span> by <a xmlns:cc="http://creativecommons.org/ns#" href="https://github.com/makiolo/fast-event-system" property="cc:attributionName" rel="cc:attributionURL">Ricardo Marmolejo García</a> is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by/4.0/">Creative Commons Reconocimiento 4.0 Internacional License</a>.

## Contributing

The source for fast-event-system is held at [fast-event-system](https://github.com/makiolo/fast-event-system) github.com site.

To report an issue, use the [fast-event-system issue tracker](https://github.com/makiolo/fast-event-system/issues) at github.com.

## Using fast-event-system

### Compile fast-event-system
It's a header-only library. Only need an include.

### Compile tests
You will need cmake (and a compiler).

```
$ git clone https://github.com/makiolo/fast-event-system.git
$ mkdir build
$ cd build
$ cmake ..
$ make (in unix) or compile generated solution (in windows)
```

### Example fast-event-system:
```CPP
// TODO meanwhile see folder tests
```
