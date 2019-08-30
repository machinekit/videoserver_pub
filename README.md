# Videoserver Publisher for Machinekit

A simple OpenCV-based video stream publisher for Machinekit devices.

Requires:
* OpenCV 3
* Boost
* Protobuf
* ZeroMQ
* CMake

## Build and install
NOTE: Compiling the application on the BeagleBone Black requires Clang, since GCC get's stuck for some reason.

```bash
sudo apt install libopencv-dev libboost-program-options-dev libboost-log-dev protobuf-compiler clang
```

```bash
git clone https://github.com/machinekoder/videoserver_pub
cd videoserver_pub
mkdir build && cd build
cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
```
