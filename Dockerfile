FROM ubuntu:15.04

RUN apt-get update -qq
RUN apt-get install -qq software-properties-common
RUN add-apt-repository --yes "deb http://llvm.org/apt/precise/ llvm-toolchain-precise-3.6 main"
RUN add-apt-repository --yes ppa:ubuntu-toolchain-r/test
RUN add-apt-repository --yes ppa:h-rayflood/llvm
RUN add-apt-repository --yes ppa:andykimpe/cmake

RUN apt-get install -qq wget
RUN wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | apt-key add -
RUN apt-get install -qq cmake
RUN apt-get install -qq clang-3.6
RUN apt-get install -qq g++-4.9
RUN apt-get install -qq xsltproc
RUN apt-get install -qq llvm-3.6-tools
RUN apt-get install -qq git
RUN apt-get install -qq kbtin
RUN apt-get install -qq libtool
RUN apt-get install -qq autoconf

