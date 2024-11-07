#pragma once
#include "FaultNet-Sim.h"

namespace FaultNet_Sim
{

	class ExampleProblem : public Problem
	{
	public:
		ExampleProblem(std::string description = "")
			: Problem(description) {}

		virtual void GenerateSNs() override;
		virtual void GenerateFailures() override;
	};


	class ExampleSimulator : public Simulator
	{
	public:
		ExampleSimulator(SimulatorParameters sp, std::string description);

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

		virtual bool IsDone(double currentTime) override;


		virtual void Simulate() override;

	};
}