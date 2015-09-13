FROM ubuntu:15.04

ENV REAL_CC clang-3.6
ENV EXTRA_DEF -std=c++14
ENV FL_CONF Release
ENV REAL_CXX clang++-3.6
ENV PACKAGE clang-3.6
ENV SUPPORT g++-4.9
ENV COVERAGE 0
ENV SANITIZER ""
ENV CC ${REAL_CC}
ENV CXX ${REAL_CXX}

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

RUN git clone https://github.com/makiolo/fast-event-system.git
RUN mkdir -p fast-event-system/$FL_CONF/shippable/testresults/
RUN cd fast-event-system/$FL_CONF && cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE=$FL_CONF -DEXTRA_DEF=$EXTRA_DEF -DCOVERAGE=$COVERAGE -DSANITIZER=$SANITIZER
RUN cd fast-event-system/$FL_CONF && make -j12
RUN cd fast-event-system/$FL_CONF && ctest .. -j12 --output-log run_tests.log --no-compress-output --output-on-failure --schedule-random -T Test --timeout 15 || true
RUN cd fast-event-system/$FL_CONF && find Testing/ -name "*.xml" | xargs xsltproc ../cmake/junit/CTest2JUnit.xsl > ../shippable/testresults/tests.xml

