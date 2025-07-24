# Example CPP Project

* This serves as a template for creating cpp projects

## Inside The Project

* Root `CMakeLists.txt`: This setups up the project. it includes specifing the minimum `cmake` version, the name of the project and the `C++` version.
* `build.sh`: This is an helper script for managing the `cmake` build system. It allows us to easily build and run tests. When you first clone the project, enable executable permission on the file by running the command below;

    ```sh
    chmod +x ./build.sh
    ```

  * You can build your project in debug mode by specifing the `--debug` flag as below;

    ```sh
    ./build.sh --debug
    ```

  * You can build your project in release mode by specifing the `--release` flag as below;

    ```sh
    ./build.sh --release
    ```

  * You can specify action you want to perform. Actions include `clean` which cleans your build directory, `build` which builds your project and `ctest` which runs the tests. An example below;

    ```sh
    ./build.sh --action=clean
    ```

  * You can also chain commands together as below;

    ```sh
    ./build.sh --action=clean,build,ctest
    ```

  * **NB**: By default the build mode is `debug` and action is `build`, this means if you specify no options as below, it would build your project in debug mode.

    ```sh
    ./build.sh
    ```

* `src/CMakeLists.txt`: This setups the libraries, the executable, googletest and the tests. Few things to note, the source files are built as a static library and linked against the main file and the test files, also each tests are built as a separate executable which means you can run each test individually.
* `example.m.cpp`: This is the main file, it serves as the entry point to the project. Ensure you keep the naming convention of this file as `*.m.cpp` as this is used by the build system to identify the main file.
* `ex.h`: This is an example header file.
* `ex.cpp`: This is an example source file. These are built as a `static` library
* `example.t.cpp`: This is an example test file. It uses `googletest`. Ensure you keep the naming convention of this kind of files as `*.t.cpp` as this used by the build system to identify that this is a test file.

## TODO

* Include a code formatter to ensure each file is properly formatted.
