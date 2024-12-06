# FaultNet-Sim

FaultNet-Sim is a multithreaded C++ software designed to simulate the evolution of a WSN under failure-prone conditions. The default simulator uses the naive nearest neighbor topology construction algorithm and round-robin TDMA scheduling for data transfer. Furthermore, FaultNet-Sim is designed with user flexibility in mind: users can define their own custom simulators by deriving the base simulator class and redefining some (or all) of the virtual functions and through the use of user defined custom data interfaces.

FaultNet-Sim supports result data logging into sqlite .db files,  implemented in the multiple producers-single consumer scheme. The logged data can then be used to analyze the impacts of algorithms and parameters on the performance of the simulated WSN.


## Basic Usage of FaultNet-Sim

### Instantiating a problem case
A problem, wrapped in the Problem class, represents a WSN problem case consisting of a list of SN locations and failure timestamps for each SN. It has two primary functions: GenerateSNs() and GenerateSNFailures(). These functions are called automatically upon the creation of a problem case. Therefore, users do not need to call these functions manually. 
GenerateSNs() is a function that randomly generates locations for $k$ different SNs. By default, the xPos and yPos variables would be randomly generated with a uniform distribution between $-300$ and $300$. Meanwhile, GenerateSNFailures() generates random failure timestamps for each of the SN. By default, this function generates failure timestamps of all SNs for the first 90 days. The times between failures are exponentially distributed, with an average of 24 hours.
It is important to note that these functions can be customized to suit the users' needs, which will be detailed in [Modifying Problem Cases](#modifying-problem-cases) below.
To instantiate a problem case, users write the following code:
```cpp
Problem prblm =
Problem::CreateProblem<Problem>(std::string("DefaultProblem"));
```

### Setting up parameters
After instantiating the problem cases, users must set the parameter values. The simulator has various parameters affecting the WSN simulation, summarized in the data structure SimulatorParameter, which consists of:

- TotalSimulationTime: The total duration for running the simulation.
- TransferTime: The duration required for an SN to perform data transfer.
- RecoveryTime: The duration required for an SN to undergo recovery.
- EnergyRateSensing: The amount of energy an SN spends per unit of time for data sensing.
- EnergyRateTransfer: The amount of energy an SN spends per unit of time for data transfer.
- TransmissionRange: The transmission range of an SN.
- InterferenceRange: The interference range of an SN.

Users initializes a SimulatorParameter instance by writing:
```cpp
SimulatorParameter sp;
sp.TotalSimulationTime = someValue_1;
sp.TransferTime = someValue_2;
sp.RecoveryTime = someValue_3;
sp.EnergyRateSensing = someValue_4;
sp.EnergyRateTransfer = someValue_5;
sp.TransmissionRange = someValue_6;
sp.InterferenceRange = someValue_7;
```
or equivalently:
```cpp
SimulatorParameter sp =
{
    someValue_1,
    someValue_2,
    someValue_3,
    someValue_4,
    someValue_5,
    someValue_6,
    someValue_7,
};
```


### Initializing the simulator
The simulation of the WSN on the problem cases is executed by the Simulator class. A simulator is a pipeline that executes a simulation with a configuration of simulation parameters. 
The default simulator uses the naive nearest-neighbor topology construction algorithm [[1]](#1) to connect the SNs and the round-robin TDMA scheduling for their data transfer [[2]](#2). 

A problem case can "own" several simulators. When the function Run() is called on the problem, simulations will be executed using all the simulators owned by the problem. This allows users to simulate the same problem case with different sets of simulation parameters. Initializing a simulator with a set of parameters can be done by writing:
```cpp
std::shared_ptr<Simulator> exampleSimulator = 
    Simulator::CreateSimulator<Simulator>(sp, "ExampleSimulator");
```

When initializing, a simulator receives an ID and stores the provided
parameters. Then, the simulator will be executed when Run() is called
by the problem that has the simulator. Therefore, users do not
need to call it manually. Run() invokes four functions described as
follows:

- TopologyConstruction(): This function constructs the topology of
  the WSN, with the greedy nearest-neighbor method as the default.
- GraphColoring(): Graph coloring of the topology is essential to
  determine the group of each SN for TDMA scheduling. This topology
  coloring is done using the Welsh-Powell algorithm
  [[3]](#3), considering the location of the SNs
  and their interference range.
- SetDataTransferIntervals(): This function sets the value of
  $\Delta$ for each SN.
  By default, $\Delta$ is set to $0$ to simulate the standard
  round-robin TDMA scheduling.
- Simulate(): The last function invoked when running the
  simulator. This function simulates the WSN operation based on the
  parameters, topology construction, graph coloring, and the
  previously set $\Delta$ values. The default simulation operation can
  be seen in the source code attachment Simulator.cpp. The simulator
  initializes a priority queue that stores data about the next state
  transition for each SN, including the timestamp and type of
  state. The transition with the smallest timestamp is processed
  first.

  
All SNs are initialized in the Data Sensing state at timestamp 0. All
these transitions are added to the priority queue. Then, a loop
simulates the changes in the WSN states at each SN state
transition. This loop runs until the termination condition is met. In
this loop, the following steps are taken:

- Determining the next state: If no failure occurs on the SN, the
  next state of that SN is determined by its current state, with
  transition rules designed based on [[4]](#4)-[[5]](#5):
  
  - Collection $\rightarrow$ Transfer.
  - Transfer $\rightarrow$ Collection.
  - Recovery $\rightarrow$ Collection.

  However, if an SN failure occurs before the current state's dwelling
  (sojourn) time is completed, the next state of that SN is
  Recovery. The next transition timestamp is determined based on the
  current transition timestamp and the dwelling time in each state,
  i.e., TransferTime in the Transfer state and RecoveryTime in the
  Recovery state. Specifically, for the Collection $\rightarrow$
  Transfer transition, the next transition timestamp is the closest
  data transfer timeslot for the SN's color group, plus the SN's
  $\Delta$. The next transition is then added to the priority queue.

- Handling state transitions:
  
  - Collection $\rightarrow$ Transfer: The SN's data collection
    time, energy consumption, and data packet size are updated.
  - Transfer $\rightarrow$ Collection: Data is forwarded from the
    SN to its parent, whether it be another SN or the base
    station. The data packets are considered lost if the parent SN
    is in the Recovery state because recovering SNs cannot receive
    transmitted data. Otherwise, the data is considered successfully
    transferred to the parent SN. Furthermore, energy consumption
    and wasted time are also calculated. Here, SNs are considered to
    have sufficiently large data storage and can store as much data as
    needed.
  - Recovery $\rightarrow$ Collection: the SN's packets are
    reset, indicating the recovery period is over.
  

- Checking termination conditions: The termination condition is
  checked. By default, the simulation stops when the simulation time
  is equal to or greater than TotalSimulationTime.


Similar to the Problem class, the functions of the simulator can be overridden as the users wish (see [Modifying Simulators](#modifying-simulators) below). 
### Multi-Simulation
FaultNet-Sim allows users to simulate multiple problem cases with multiple simulators in a multithreaded (parallel) fashion, which is handled by the engine automatically. Users can set the maximum number of simultaneous simulation threads by accessing the ``source/Global.h`` header file and modifying the following line:
```cpp
static constexpr int g_NumberOfThreads = 1;
```
Users can leverage this multithreading capability by simply instantiating multiple problem cases, which can be done by writing:
```cpp
Problem prblm1 =
Problem::CreateProblem<Problem>(std::string("DefaultProblem_1"));
Problem prblm2 =
Problem::CreateProblem<Problem>(std::string("DefaultProblem_2"));
Problem prblm3 =
Problem::CreateProblem<Problem>(std::string("DefaultProblem_3"));
...
```
Furthermore, if users wish to perform a simulation on many different parameter value combinations, users can use the SimulatorParameterGrid data structure instead of SimulatorParameter. This data structure can construct the Cartesian product of all the parameter values passed into it, which can be initialized by writing:
```cpp
SimulatorParameterGrid spg;
spg.TotalSimulationTime = 
    {someValue_1_1, someValue_1_2, someValue_1_3, ...};
spg.TransferTime = 
    {someValue_2_1, someValue_2_2, someValue_2_3, ...};
spg.RecoveryTime = 
    {someValue_3_1, someValue_3_2, someValue_3_3, ...};
spg.EnergyRateSensing = 
    {someValue_4_1, someValue_4_2, someValue_4_3, ...};
spg.EnergyRateTransfer = 
    {someValue_5_1, someValue_5_2, someValue_5_3, ...};
spg.TransmissionRange = 
    {someValue_6_1, someValue_6_2, someValue_6_3, ...};
spg.InterferenceRange = 
    {someValue_7_1, someValue_7_2, someValue_7_3, ...};
```
Meanwhile, initializing multiple simulators, each with an instance of
the product of the parameters set in a SimulatorParameterGrid can be
done by:
```cpp
std::vector<std::shared_ptr<Simulator>> simulators = 
    Simulator::CreateSimulator<Simulator>(spg, "Simulators");
```


### Function InterfaceMain()
To ensure the program works correctly, function
InterfaceMain() must implement the following steps:

- Instantiating problem cases: Initializing one or more problem
  instances by calling the Problem::CreateProblem() function.
- Setting simulator parameters: Initializing SimulatorParameter by
  providing the desired value for each parameter. If simulations are
  to be run with different values for each parameter, a
  SimulatorParameterGrid can be constructed, containing arrays of
  different values for each parameter.
- Initializing simulators: Initializing the simulator or array of
  simulators by calling Simulator::CreateSimulator() and passing the
  previously constructed SimulatorParameter or SimulatorParameterGrid
  as an argument.
- Adding simulator to the problem: Adding the desired simulators
  to the problem by calling AddSimulator() on the problem and
  providing the simulator as an argument.
- Call the function Run(): Calling Run() on the problem cases to
  run all the simulators that have been added to the problem.


Below is a complete example of a basic InterfaceMain() function:
```cpp
void interfaceMain()
{
    /////////////////////////////////
    // Instantiating Problem Cases //
    /////////////////////////////////

    std::vector<std::shared_ptr<Problem>> problems;

    I_ProblemData i_ProblemData =
    {
        0.1,
        2.3
    };

		
    problems.push_back(Problem::CreateProblem<Problem>
        (std::string("DefaultProblem_1")));
    problems.push_back(Problem::CreateProblem<Problem>
        (std::string("DefaultProblem_2")));
        
    for (auto& problem : problems)
    {
        problem->i_ProblemData = i_ProblemData;
        problem->Initialize();
    }

    ///////////////////////////
    // Setting Up Parameters //
    ///////////////////////////

    std::vector<double> TotalSimulationTimes =
    {
        3600 * 24,
    };

    std::vector<double> transmissionRanges =
    {
        50, 75, 100, 125, 150
    };

    std::vector<double> interferenceRanges =
    {
        100, 150, 200, 250, 300
    };

    std::vector<double> transferTimes =
    {
        60, 120, 180, 240, 300
    };

    std::vector<double> recoveryTimes =
    {
        60.0,
    };

    std::vector<double> energyRateTransfers =
    {
        8.0,
    };

    std::vector<double> EnergyRateSensings =
    {
        1.0,
    };

    SimulatorParameterGrid spg =
    {
        TotalSimulationTimes,
        transferTimes,
        recoveryTimes,
        EnergyRateSensings,
        energyRateTransfers,
        transmissionRanges,
        interferenceRanges
    };

    I_SimulatorData i_SimulatorData =
    {
        3.1,
        2.1
    };

    /////////////////////////////
    // Initializing Simulators //
    /////////////////////////////

    std::vector<std::shared_ptr<Simulator>> defaultSimulators = 
        Simulator::CreateSimulator<Simulator>
            (spg, "DefaultSimulator");;

    for (auto& simulator : exampleSimulators)
        simulator->i_SimulatorData = i_SimulatorData;

    /////////////////////////////////////////////////
    // Attaching Problems & Simulators and Running //
    /////////////////////////////////////////////////

    for (int i = 0; i < problems.size(); i++)
    {
        problems[i]->AddSimulator(defaultSimulators);
        problems[i]->Run();
    }
}
```

### Database Logging
FaultNet-Sim automatically logs simulation results to a SQLite database file located in ``Results/Main.db``. The data logging is performed by a single dedicated thread in a ``multiple-producer single-consumer`` manner. If users wish to modify the path of the output database file, it can be done by accessing the ``source/SQLiteDatabase.cpp`` source file and modifying the following line:
```cpp
std::shared_ptr<SQLiteDatabase> SQLiteDatabase::s_Database(
    new SQLiteDatabase("Results/Main.db"));
```

## Modifying FaultNet-Sim

### Modifying Problem Cases


Optionally, users can modify the problem case generation rules if needed. To do this, users can inherit the Problem class and override the GenerateSNs() and GenerateSNFailures() virtual functions. Below are examples of how to inherit the Problem class and override GenerateSNs() and GenerateSNFailures():

- Inheriting the Problem class: The Problem class can be modified
  by first inheriting it, which can be done by writing:
```cpp
class ExampleProblem : public Problem
{
public:
    ExampleProblem(std::string description = "")
        : Problem(description) {}

    virtual void GenerateSNs() override;
    virtual void GenerateFailures() override;
};
```

Note that it is not mandatory to override both of the virtual functions. If the users do not wish to override them, they simply remove the overriding lines. With this inheritance, users can now instantiate the modified Problem class (ExampleProblem) by writing:
```cpp
Problem examplePrblm =
Problem::CreateProblem<ExampleProblem>(std::string("ExampleProblem"));
```

- Overriding GenerateSN(): This function generates $k$ SNs and assigns a location to each SN. In this function, users must implement:
```cpp
void ExampleProblem::GenerateSNs()
{
    for (int SNCount = 0; SNCount < k; SNCount++)
    {
        SensorNode sn;
        sn.m_Position = { xPos, yPos };
    
        m_SensorNodes.push_back(sn);
    }
}
```

The method for determining $k$ (the number of SNs) and their corresponding locations (specified by the xPos and yPos variables) is flexible and can be customized by the user.

- Overriding GenerateSNFailure(): This function generates failure timestamps for each SN. In this function, users must implement:
```cpp
void ExampleProblem::GenerateSNFailure()
{
    for (int i = 0; i < m_SensorNodes.size(); i++)
    {
        while (!someTerminationCondition)
        {
            m_SensorNodes[i].m_FailureTimestamps.
                push_back(someRandomTimestamp);
        }
    }
}
```

The code above generates a random number of failures (along with their corresponding timestamps) for every SN. Users can customize the way these failures are randomized, i.e., by making some changes to the variables someTerminationCondition and someRandomTimestamp.


### Modifying Simulators

Similar to the Problem class, the Simulator class can be modified by
inheriting it and overriding the functions mentioned above. To do
this, users carry out the following steps

- Inheriting the Simulator class: Before any function overriding,
  users must inherit the Simulator class, for example by:
```cpp
class ExampleSimulator : public Simulator
{
public:
    ExampleSimulator(SimulatorParameters sp, std::string description)
        : Simulator(sp, description) {}

    std::shared_ptr<Simulator> Clone() const override
    {
        return std::make_shared<ExampleSimulator>(*this);
    }

    virtual std::string GetSimulatorType() override
    {
        return "ExampleSimulator";
    }

protected:
    virtual void ConstructTopology() override;
    virtual void ColorTopology() override;
    virtual void SetSNDeltas() override;
    virtual void Simulate() override;
    virtual bool IsDone(double currentTime) override;
};
``` 
After inheriting the Simulator class, users can implement the
overridden function by following the steps below.
    
- Overriding ConstructTopology(): If users wish to modify how
  the topology is constructed, it can be done by inheriting the
  Simulator class and overriding ConstructTopology(). When overriding
  this function, users must set the parent of each SN, for example
  by:
```cpp
void ExampleSimulator::ConstructTopology()
{
    for (int i = 0; i < m_SensorNodes.size(); i++)
    {
        m_SensorNodes[i].m_Parent = parentID;
    }
}
``` 
where parentID is user-defined.

- Overriding ColorTopology(): Similar to the previous function,
  users can override ColorTopology() to modify how the graph
  coloring works. Here, users can implement:
```cpp
void ExampleSimulator::ColorTopology()
{
    for (int i = 0; i < m_SensorNodes.size(); i++)
    {
        m_SensorNodes[j].m_Color = someColor;
    }
}
```
where someColor is user-defined and must follow the rule that SNs of
the same color cannot be in the interference range of each other.

- Overriding SetSNDeltas(): To modify how the simulator calculates
  the data transfer interval $\Delta$, users can override
  SetSNDeltas() and implement:
```cpp
void ExampleSimulator::SetSNDeltas()
{
    for (int i = 0; i < m_SensorNodes.size(); i++)
    {
        m_SensorNodes[i].m_DeltaOpt = someDelta;
    }
}
```
Variable someDelta is a double-precision floating-point number
representing the desired duration in the Collection state. The value
of $\Delta$ is determined by the user but cannot be negative.

- Overriding Simulate(): Modifying Simulate() is not encouraged as
  it is the heart of the simulator and is difficult to do. However, if
  users still wish to do so, it also can be done by inheriting
  the Simulator class and overriding Simulate(), for example by:
```cpp
void ExampleSimulator::Simulate()
{
    // User-defined simulation logic
}
```

- Overriding IsDone(): If users wish to determine a custom
  simulation termination condition, it can be done by overriding
  IsDone(), for example by:
```cpp
bool ExampleSimulator::IsDone()
{
    return someTerminationCondition;
}
```
The function must return a Boolean expression, which will terminate the
simulation when evaluated as true.

### Custom user data
To facilitate the use of custom user data, classes Problem, Simulator,
and SensorNode have data structure member variables: I\_ProblemData,
I\_SimulatorData, and I\_SensorNodeData. The contents of these data structures
can be freely modified by users in the DataInterface.h header
file. The use of these data structures is also flexible. An example
modification of these user data structures is as follows:
```cpp
struct I_ProblemData
{
    int a;
    double b;
};

struct I_SimulatorData
{
    int a;
    double b;
};

struct I_SensorNodeData
{
    int a;
    double b;
};
```

These custom data structures are public attributes of the respective classes,
which can be used in any of the overridden functions. These data can
be accessed and/or modified in the following manner:
```cpp
Problem someProblem;
Simulator someSimulator;
SensorNode someSensorNode;

// Accessing or retrieving
I_ProblemData someProblemData = someProblem.i_ProblemData;
I_SimulatorData someSimulatorData = someSimulator.i_SimulatorData;
I_SensorNodeData someSensorNodeData = someSensorNode.i_SensorNodeData;

// Modifying or setting
someProblem.i_ProblemData = someProblemData;
someSimulator.i_SimulatorData = someSimulatorData;
someSensorNode.i_SensorNodeData = someSensorNodeData;
```

## Setup

FaultNet-Sim supports both Windows and Linux operating systems. To set up FaultNet-Sim, first, clone the source code from the GitHub repository using the following command:
```sh
    git clone https://github.com/FlaneTern/FaultNet-Sim.git
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


# References
<a id="1">[1]</a> 
L. Karim, T. E. Salti, N. Nasser, Q. H. Mahmoud, The significant impact
of a set of topologies on wireless sensor networks, EURASIP Journal on
Wireless Communications and Networking 2012 (2012) 1–13.


<a id="2">[2]</a> 
G. Vakulya, G. Simon, Extended round-robin tdma scheduling scheme
for wireless sensor networks, in: 2013 IEEE International Instrumenta-
tion and Measurement Technology Conference (I2MTC), IEEE, 2013,
pp. 253–258.


<a id="3">[3]</a> 
D. J. A. Welsh, M. B. Powell, An upper bound for the chromatic number
of a graph and its application to timetabling problems, The Computer
Journal 10 (1) (1967) 85–86. [doi:10.1093/comjnl/10.1.85](https://doi.org/10.1093/comjnl/10.1.85).


<a id="4">[4]</a> 
M. A. Amrizal, L. Guillen, T. Suganuma, An analytical approach for
optimizing data transfer rate in a faulty wireless sensor network, in:
2019 IEEE 24th Pacific Rim International Symposium on Dependable
Computing (PRDC), 2019, pp. 122–1221. [doi:10.1109/PRDC47002.2019.00041](https://doi.org/10.1109/PRDC47002.2019.00041).


<a id="5">[5]</a> 
M. A. Amrizal, S. Y. Pradata, M. R. Sudha, L. Wang, R. Pulungan,
Investigating the impact of optimal data transfer intervals on failure-
prone wireless sensor networks, IEEE Sensors Letters (2024) 1–4
[doi:10.1109/LSENS.2024.3414294](https://doi.org/10.1109/LSENS.2024.3414294).
