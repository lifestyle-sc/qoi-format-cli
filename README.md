# **QOI/PPM Image Converter**

This command-line utility provides the functionality to encode **PPM** (Portable Pixmap) images to the **QOI** (Quite OK Image) format and to decode QOI images back to PPM. This tool is built using C++ and provides a straightforward way to handle lossless image conversion for these two formats. It uses the specification provided by the official [specification here](https://qoiformat.org/qoi-specification.pdf)

-----

## **How to Build**

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

-----

## **How to Use**

The program is executed from the command line and takes two arguments: an operation and the input filename.

### **Encode Operation**

To convert a **PPM** file to a **QOI** file, use the `encode` operation below:

**Command**:

```sh
./build/debug/src/qoi.tsk encode <input_file> <output_file> -f ppm
```

To convert a **PNG** file to a **QOI** file, use the `encode` operation below:

**Command**:

```sh
./build/debug/src/qoi.tsk encode <input_file> <output_file> -f png
```

### **Decode Operation**

To convert a **QOI** file to a **PPM** file, use the `decode` operation below:

**Command**:

```sh
./build/debug/src/qoi.tsk decode <input_file> <output_file>
```

To convert a **QOI** file to a **PNG** file, use the the `decode` operation below:

**Command**:

```sh
./build/debug/src/qoi.tsk decode <input_file> <output_file> -f pmm
```

-----

## **Supported Formats**

* **PPM**: Only the **P6 (binary)** format with an 8-bit color depth (max pixel value of 255) is supported.
* **PNG**: Supports any kind of png file. It uses the stbi library to handle the loading of the file.
* **QOI**: The tool handles QOI files with both 3 (RGB) and 4 (RGBA) channels and any colorspace.

## **Limitations**

* The tool only works with PPM files in the P6 binary format.
* The maximum supported file size is **1 GB**.

-----
