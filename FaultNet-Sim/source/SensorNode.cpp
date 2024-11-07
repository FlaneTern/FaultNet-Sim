#include "PCH.h"
#include "SensorNode.h"

namespace FaultNet_Sim
{

	std::string WorkingStateToString(const WorkingState& ws)
	{
		switch (ws)
		{
		case WorkingState::Transfer: 
			return "Transfer";
		case WorkingState::Collection: 
			return "Collection";
		case WorkingState::Recovery: 
			return "Recovery";
		}

		throw std::runtime_error("Unknown Working State in WorkingStateToString!");
		return "";
	}

	void SensorNode::Reset()
	{
		m_Parent = c_InvalidIndex;
		m_Level = -1;

		m_DeltaOpt = 0;

		m_CurrentData = 0;

		m_CollectionTime = 0;
		m_WastedTime = 0;

		m_EnergyConsumed = 0;
		m_EnergyWasted = 0;

		m_SentPacketTotalDelay = 0;
		m_SentPacketCount = 0;

		m_Color = -1;

		m_TotalDataSent = 0;

		m_Packets.clear();

		m_CurrentPacketIterator = -1;

		m_CurrentParent = -1;
		m_CurrentColor = -1;

		m_WelshPowellDegree = -1;

		m_FailureIterator = 0;
	}
}