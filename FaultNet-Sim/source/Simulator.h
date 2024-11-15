#pragma once
#include "Distribution.h"
#include "SensorNode.h"
#include "DataInterface.h"

namespace FaultNet_Sim
{

	struct Failure
	{
		int64_t SNID;
		double Timestamp;
	};

	struct SimulationInterval
	{
		WorkingState State;
		double StartTime;
		double EndTime;
	};			

	struct SimulatorResults
	{
		double ActualTotalDuration = 0;
		int64_t FinalFailureIndex = 0;
		double CWSNEfficiency = 0;
	};

	struct SimulatorParameters
	{
		double TotalSimulationTime = -1;
		double TransferTime = -1;
		double RecoveryTime = -1;

		double EnergyRateWorking;
		double EnergyRateTransfer;

		double TransmissionRange;
		double InterferenceRange;
	};

	struct SimulatorParameterGrid
	{
		std::vector<double> TotalSimulationTime;
		std::vector<double> TransferTime;
		std::vector<double> RecoveryTime;

		std::vector<double> EnergyRateWorking;
		std::vector<double> EnergyRateTransfer;

		std::vector<double> TransmissionRange;
		std::vector<double> InterferenceRange;
	};


	class Simulator
	{
	public:
		Simulator(SimulatorParameters sp, std::string description = "");
		Simulator(const Simulator& other);

		void Run(int64_t problemID, const std::vector<SensorNode>& SNs);

		virtual std::shared_ptr<Simulator> Clone() const
		{
			return std::make_shared<Simulator>(*this);
		}

		virtual std::string GetSimulatorType()
		{
			return "DefaultSimulator";
		}

		I_SimulatorData i_SimulatorData;
		I_ProblemData i_ProblemData;

		template<typename T>
		static inline std::shared_ptr<Simulator> CreateSimulator(SimulatorParameters sp, std::string description = "") { return T(sp, description); }

		template<typename T>
		static std::vector<std::shared_ptr<Simulator>> CreateSimulator(SimulatorParameterGrid spg, std::string description = "")
		{
			std::vector<std::shared_ptr<Simulator>> simulators;

			for (auto& recoveryTime : spg.RecoveryTime)
			{
				for (auto& transferTime : spg.TransferTime)
				{
					for (auto& TotalSimulationTime : spg.TotalSimulationTime)
					{
						for (auto& energyRateWorking : spg.EnergyRateWorking)
						{
							for (int energyRateTransfer : spg.EnergyRateTransfer)
							{
								for (auto& transmissionRange : spg.TransmissionRange)
								{
									for (auto& interferenceRange : spg.InterferenceRange)
									{
										FaultNet_Sim::SimulatorParameters sp =
										{
											TotalSimulationTime,
											transferTime,
											recoveryTime,
											energyRateWorking,
											energyRateTransfer,
											transmissionRange,
											interferenceRange
										};

										simulators.push_back(std::make_shared<T>(sp, description));
									}
								}
							}
						}
					}
				}
			}

			return simulators;
		}

		inline int64_t GetSimulatorID() { return m_SimulatorID; }
		inline int64_t GetProblemID() { return m_ProblemID; }
		inline std::string GetDescription() { return m_Description; }
		inline SimulatorParameters GetSimulatorParameters() { return m_SimulatorParameters; }
		inline SimulatorResults GetSimulatorResults() { return m_SimulatorResults; }
		inline double GetTransferredTotalDuration() { return m_TransferredTotalDuration; }

	protected:
		Simulator() = delete;

		virtual void Simulate();

		virtual void ConstructTopology();
		virtual void ColorTopology();
		virtual void SetSNDeltas();

		virtual bool IsDone(double currentTime);

		void ConstructTopologyPost();
		void ColorTopologyPost();
		void SetSNDeltasPost();

		int64_t m_SimulatorID;
		int64_t m_ProblemID;
		std::string m_Description;
		double m_TransferredTotalDuration;

		std::vector<SensorNode> m_SensorNodes;

		SimulatorParameters m_SimulatorParameters;

		SimulatorResults m_SimulatorResults;

		static int64_t GenerateID();

	private:

		void Log();

		void Deinitialize();

	};


}