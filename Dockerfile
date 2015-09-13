FROM ubuntu:15.04

ENV REAL_CC clang-3.6
ENV EXTRA_DEF -std=c++14
ENV BUILD_MODE Release
ENV REAL_CXX clang++-3.6
ENV PACKAGE clang-3.6
ENV SUPPORT g++-4.9
ENV COVERAGE 0
ENV SANITIZER ""
ENV CC ${REAL_CC}
ENV CXX ${REAL_CXX}
ENV REPOSITORY_USER makiolo
ENV REPOSITORY fast-event-system

RUN apt-get update -qq
RUN apt-get install -qq software-properties-common
RUN add-apt-repository --yes "deb http://llvm.org/apt/precise/ llvm-toolchain-precise-3.6 main"
RUN add-apt-repository --yes ppa:ubuntu-toolchain-r/test
RUN add-apt-repository --yes ppa:h-rayflood/llvm
RUN add-apt-repository --yes ppa:andykimpe/cmake

RUN apt-get install -qq wget
RUN wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | apt-key add -
RUN apt-get install -qq cmake
RUN apt-get install -qq $SUPPORT
RUN apt-get install -qq $PACKAGE
RUN apt-get install -qq xsltproc
RUN apt-get install -qq llvm-3.6-tools
RUN apt-get install -qq git

RUN git clone https://github.com/$REPOSITORY_USER/$REPOSITORY.git
RUN mkdir -p $REPOSITORY/$BUILD_MODE/
RUN mkdir -p $REPOSITORY/shippable/testresults/
RUN cd $REPOSITORY/$BUILD_MODE && cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_MODE -DEXTRA_DEF=$EXTRA_DEF -DCOVERAGE=$COVERAGE -DSANITIZER=$SANITIZER
RUN cd $REPOSITORY/$BUILD_MODE && make -j12
RUN cd $REPOSITORY/$BUILD_MODE && ctest .. -j12 --output-log run_tests.log --no-compress-output --output-on-failure --schedule-random -T Test --timeout 15 || true
RUN find $REPOSITORY/$BUILD_MODE/Testing/ -name "*.xml" | xargs xsltproc $REPOSITORY/cmake/junit/CTest2JUnit.xsl > $REPOSITORY/shippable/testresults/tests.xml

