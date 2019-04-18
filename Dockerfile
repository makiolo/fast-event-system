ARG IMAGE
FROM $IMAGE

ARG MODE
ARG COMPILER
ARG COMPILER_VERSION
ARG COMPILER_LIBCXX
ARG CC
ARG CXX
ARG PACKAGE
ARG CONAN_TOKEN

WORKDIR /code
COPY . /code/

RUN echo aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa CONAN_TOKEN=${CONAN_TOKEN}

# system depends
RUN pip install -r requirements.txt
RUN curl -sL https://deb.nodesource.com/setup_10.x | sudo -E bash -
RUN sudo apt-get install -y nodejs

RUN conan remote add bincrafters "https://api.bintray.com/conan/bincrafters/public-conan"
RUN conan remote add npm-mas-mas "https://api.bintray.com/conan/npm-mas-mas/testing"
RUN conan user -p $CONAN_TOKEN -r npm-mas-mas makiolo

# RUN conan install . -r npm-mas-mas -s compiler=$COMPILER -s build_type=$MODE -s compiler.libcxx=$COMPILER_LIBCXX -s compiler.version=$COMPILER_VERSION
# RUN conan install gtest/1.8.1@bincrafters/stable --build -s compiler=$COMPILER -s build_type=$MODE -s compiler.libcxx=$COMPILER_LIBCXX -s compiler.version=$COMPILER_VERSION
# RUN conan install boost/1.70.0@conan/stable --build -s compiler=$COMPILER -s build_type=$MODE -s compiler.libcxx=$COMPILER_LIBCXX -s compiler.version=$COMPILER_VERSION
RUN conan create . npm-mas-mas/testing -s compiler=$COMPILER -s build_type=$MODE -s compiler.libcxx=$COMPILER_LIBCXX -s compiler.version=$COMPILER_VERSION -tf None
RUN conan upload $PACKAGE/*@npm-mas-mas/testing -r npm-mas-mas --all -c

