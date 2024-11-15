# FaultNet-Sim

FaultNet-Sim is a multithreaded C++ software designed to simulate the evolution of a WSN under failure-prone conditions. The default simulator uses the naive nearest neighbor topology construction algorithm and round-robin TDMA scheduling for data transfer. Furthermore, FaultNet-Sim is designed with user flexibility in mind: users can define their own custom simulators by deriving the base simulator class and redefining some (or all) of the virtual functions and through the use of user defined custom data interfaces.

FaultNet-Sim supports result data logging into sqlite .db files,  implemented in the multiple producers-single consumer scheme. The logged data can then be used to analyze the impacts of algorithms and parameters on the performance of the simulated WSN.


## Setup

FaultNet-Sim supports both Windows and Linux operating systems. To set up FaultNet-Sim, first, clone the source code from the GitHub repository using the following command:
```sh
    git clone https://github.com/FlaneTern/FaultNet-Sim-Alpha.git
```
Then, follow the setup instructions below based on your operating system.

### Setup on Windows
- Execute the "BUILD\_WINDOWS.bat" program to generate the solution and project files for Visual Studio 2022.
- Open the solution file "FaultNet-Sim.sln" to load the solution in Visual Studio.

### Setup on Linux
- Execute the "BUILD\_LINUX.sh" script to generate the makefile using the following command:
```sh
    ./BUILD_LINUX.sh
```
- item Use your preferred code editor on Linux to open the FaultNet-Sim folder.

## Build and Run

### Build and Run on Windows
Pick the desired build configuration and click on "Start Debugging"

### Build and Run on Linux
To compile the program, run one of the two following commands based on the desired build configuration:
```sh
    make config=debug_x64
    make config=release_x64
```

Next, to execute the compiled program, run one of the two following commands based on the previously chosen build configuration:

```sh
    ./bin/Debug-x86_64/FaultNet-Sim/FaultNet-Sim
    ./bin/Release-x86_64/FaultNet-Sim/FaultNet-Sim
```
