#include "PCH.h"
#include "DatabaseData.h"



namespace FaultNet_Sim
{

	Data Data::ConvertData(Problem& problem)
	{
		Data data;
		data.m_DataType = DataType::ProblemData;
		data.m_Data = std::vector<std::byte>(sizeof(ProblemData));

		std::string description = problem.GetDescription();

		ProblemData problemData;
		problemData.ProblemID = problem.GetProblemID();
		memset(problemData.Description, 0, sizeof(problemData.Description));
		memcpy(problemData.Description, description.data(), strlen(description.data()));

		memcpy(&data.m_Data[0], &problemData, sizeof(ProblemData));
		return data;
	}

	Data Data::ConvertData(Simulator& simulator)
	{
		Data data;
		data.m_DataType = DataType::SimulatorData;
		data.m_Data = std::vector<std::byte>(sizeof(SimulatorData));

		SimulatorParameters sp = simulator.GetSimulatorParameters();
		std::string description = simulator.GetDescription();
		std::string simulatorType = simulator.GetSimulatorType();

		SimulatorData simulatorData;
		simulatorData.SimulatorID = simulator.GetSimulatorID();
		simulatorData.ProblemID = simulator.GetProblemID();
		memset(simulatorData.Description, 0, sizeof(simulatorData.Description));
		memcpy(simulatorData.Description, description.data(), strlen(description.data()));
		memset(simulatorData.SimulatorType, 0, sizeof(simulatorData.SimulatorType));
		memcpy(simulatorData.SimulatorType, simulatorType.data(), strlen(simulatorType.data()));
		simulatorData.TotalSimulationTime = sp.TotalSimulationTime;
		simulatorData.TransferTime = sp.TransferTime;
		simulatorData.RecoveryTime = sp.RecoveryTime;
		simulatorData.EnergyRateSensing = sp.EnergyRateSensing;
		simulatorData.EnergyRateTransfer = sp.EnergyRateTransfer;
		simulatorData.TransmissionRange = sp.TransmissionRange;
		simulatorData.InterferenceRange = sp.InterferenceRange;
		simulatorData.TransferredTotalDuration = simulator.GetTransferredTotalDuration();

		memcpy(&data.m_Data[0], &simulatorData, sizeof(SimulatorData));
		return data;
	}

	Data Data::ConvertData(SensorNode& sn, int64_t simulatorID, int64_t problemID)
	{
		Data data;
		data.m_DataType = DataType::SensorNodeData;
		data.m_Data = std::vector<std::byte>(sizeof(SensorNodeData));
		double failureMean = 0.0;
		for (int j = 0; j < sn.m_FailureTimestamps.size(); j++)
		{
			if (j == 0)
				failureMean += sn.m_FailureTimestamps[j];
			else
				failureMean += sn.m_FailureTimestamps[j] - sn.m_FailureTimestamps[j - 1];
		}
		failureMean /= sn.m_FailureTimestamps.size();

		SensorNodeData snData;
		snData.SensorNodeID = sn.m_ID;
		snData.SimulatorID = simulatorID;
		snData.ProblemID = problemID;
		snData.PositionX = sn.m_Position.X;
		snData.PositionY = sn.m_Position.Y;
		snData.Parent = sn.m_Parent;
		snData.Level = sn.m_Level;
		snData.DeltaOpt = sn.m_DeltaOpt;
		snData.CollectionTime = sn.m_CollectionTime;
		snData.WastedTime = sn.m_WastedTime;
		snData.TotalDataSent = sn.m_TotalDataSent;
		snData.EnergyConsumed = sn.m_EnergyConsumed;
		snData.EnergyWasted = sn.m_EnergyWasted;
		snData.SentPacketTotalDelay = sn.m_SentPacketTotalDelay;
		snData.SentPacketCount = sn.m_SentPacketCount;
		snData.Color = sn.m_Color;
		snData.FailureMean = failureMean;
		snData.ChildCount = sn.m_ChildCount;
		snData.DescendantCount = sn.m_DescendantCount;

		memcpy(&data.m_Data[0], &snData, sizeof(SensorNodeData));

		return data;
	}
}