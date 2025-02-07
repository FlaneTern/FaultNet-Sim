#include "PCH.h"
#include "FaultNet-Sim.h"

#include "InterfaceExample.h"

void interfaceMain()
{
	std::vector<std::shared_ptr<FaultNet_Sim::Problem>> problems;

	I_ProblemData i_ProblemData =
	{
		2,
		2.3
	};

	I_SimulatorData i_SimulatorData =
	{
		3,
		2.1
	};
		
	problems.push_back(FaultNet_Sim::Problem::CreateProblem<FaultNet_Sim::Problem>(std::string("DefaultProblem")));
	problems.push_back(FaultNet_Sim::Problem::CreateProblem<FaultNet_Sim::ExampleProblem>(std::string("ExampleProblem")));
	for (auto& problem : problems)
	{
		problem->i_ProblemData = i_ProblemData;
		problem->Initialize();
	}

	std::vector<double> TotalSimulationTimes =
	{
		3600 * 24 * 90,
	};

	std::vector<double> transmissionRanges =
	{
		50, 75, 100, 125, 150
	};

	std::vector<double> interferenceRanges =
	{
		150, 225, 300, 375, 450
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
		17.4 * 2.0 * 0.001, 
	};

	std::vector<double> energyRateSensings =
	{
		17.4 * 2.0 * 0.001 * 0.125, 
	};

	FaultNet_Sim::SimulatorParameterGrid spg =
	{
		TotalSimulationTimes,
		transferTimes,
		recoveryTimes,
		energyRateSensings,
		energyRateTransfers,
		transmissionRanges,
		interferenceRanges
	};


	std::vector<std::shared_ptr<FaultNet_Sim::Simulator>> defaultSimulators = FaultNet_Sim::Simulator::CreateSimulator<FaultNet_Sim::Simulator>(spg, "DefaultSimulator");
	std::vector<std::shared_ptr<FaultNet_Sim::Simulator>> exampleSimulators = FaultNet_Sim::Simulator::CreateSimulator<FaultNet_Sim::ExampleSimulator>(spg, "ExampleSimulator");
	for (auto& simulator : exampleSimulators)
		simulator->i_SimulatorData = i_SimulatorData;

	for (int i = 0; i < problems.size(); i++)
	{
		problems[i]->AddSimulator(defaultSimulators);
		problems[i]->AddSimulator(exampleSimulators);

		problems[i]->Run();
	}

}
