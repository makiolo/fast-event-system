#!//bin/bash

# force recompile
# RUN conan install gtest/1.8.1@bincrafters/stable --build -s compiler=$COMPILER -s build_type=$MODE -s compiler.libcxx=$COMPILER_LIBCXX -s compiler.version=$COMPILER_VERSION
# RUN conan install boost/1.70.0@conan/stable --build -s compiler=$COMPILER -s build_type=$MODE -s compiler.libcxx=$COMPILER_LIBCXX -s compiler.version=$COMPILER_VERSION
# RUN conan create . npm-mas-mas/testing -s compiler=$COMPILER -s build_type=$MODE -s compiler.libcxx=$COMPILER_LIBCXX -s compiler.version=$COMPILER_VERSION -tf None
# RUN conan upload '*' -r npm-mas-mas --all -c

# compile only $PACKAGE
conan create . npm-mas-mas/testing -s compiler=$COMPILER -s build_type=$MODE -s compiler.libcxx=$COMPILER_LIBCXX -s compiler.version=$COMPILER_VERSION -tf None
conan upload $PACKAGE/*@npm-mas-mas/testing -r npm-mas-mas --all -c

