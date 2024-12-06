#include "PCH.h"
#include "InterfaceExample.h"


namespace FaultNet_Sim
{

	static constexpr double s_EnergyTransitionWorkingToTransfer = 0.0;
	static constexpr double s_EnergyTransitionTransferToWorking = 0.0;


	void ExampleProblem::GenerateSNs()
	{

		static constexpr uint64_t TotalSNCount = 100;

		Distribution dist(DistributionType::Uniform, 300.0 / 2.0,
			std::sqrt(300.0 * 300.0 / 12.0));


		for (int SNCount = 0; SNCount < TotalSNCount; SNCount++)
		{
			double xPos = dist.GenerateRandomNumber() * (s_RNG() % 2 ? -1 : 1);
			double yPos = dist.GenerateRandomNumber() * (s_RNG() % 2 ? -1 : 1);

			SensorNode sn;
			sn.m_ID = (int64_t)SNCount;
			sn.m_Position = { xPos, yPos };

			m_SensorNodes.push_back(sn);
		}
	}

	void ExampleProblem::GenerateFailures()
	{
		static constexpr double failGenerationDuration = 3600 * 24 * 90;

		Distribution uniformDist(DistributionType::Exponential, 3600 * 24, 3600 * 24);

		for (int i = 0; i < m_SensorNodes.size(); i++)
		{
			double currentTime = 0;
			double timeToNextFailure;
			while (currentTime < failGenerationDuration)
			{
				timeToNextFailure = uniformDist.GenerateRandomNumber();
				currentTime += timeToNextFailure;
				m_SensorNodes[i].m_FailureTimestamps.push_back(currentTime);
			}
		}
	}


	ExampleSimulator::ExampleSimulator(SimulatorParameters sp, std::string description)
		: Simulator(sp, description)
	{

	}

	void ExampleSimulator::ConstructTopology()
	{

		for (int i = 0; i < m_SensorNodes.size(); i++)
		{
			if (std::sqrt(m_SensorNodes[i].m_Position.X * m_SensorNodes[i].m_Position.X +
				m_SensorNodes[i].m_Position.Y * m_SensorNodes[i].m_Position.Y) < m_SimulatorParameters.TransmissionRange)
			{
				m_SensorNodes[i].m_Parent = SensorNode::c_BaseStationIndex;
				m_SensorNodes[i].m_Level = 0;
			}
		}

		int currentLevel = 1;
		bool done = false;
		bool assigned = true;


		while (!done && assigned)
		{
			done = true;
			assigned = false;

			for (int i = 0; i < m_SensorNodes.size(); i++)
			{
				if (m_SensorNodes[i].m_Parent != SensorNode::c_InvalidIndex)
					continue;



				struct IDDistance
				{
					int64_t SNID;
					double Distance;
				};

				std::vector<IDDistance> temp;

				for (int j = 0; j < m_SensorNodes.size(); j++)
				{
					if (m_SensorNodes[j].m_Level != currentLevel - 1)
						continue;

					double distance = SensorNode::Distance(m_SensorNodes[i], m_SensorNodes[j]);

					if (distance < m_SimulatorParameters.TransmissionRange)
						temp.push_back({ (int64_t)j, distance });

				}

				if (temp.empty())
				{
					done = false;
					continue;
				}
				assigned = true;


				int closestIndex = -1;
				double closestDistance = std::numeric_limits<double>::max();
				for (int j = 0; j < temp.size(); j++)
				{
					if (closestDistance > temp[j].Distance)
					{
						closestIndex = temp[j].SNID;
						closestDistance = temp[j].Distance;
					}
				}

				m_SensorNodes[i].m_Parent = closestIndex;
				m_SensorNodes[i].m_Level = currentLevel;
			}


			currentLevel++;
		}


		for (int i = 0; i < m_SensorNodes.size(); i++)
		{
			if (m_SensorNodes[i].m_Parent == SensorNode::c_InvalidIndex)
			{
				m_SensorNodes[i].m_Parent = SensorNode::c_NoParentIndex;
			}
		}

		for (int i = 0; i < m_SensorNodes.size(); i++)
		{
			m_SensorNodes[i].m_ChildCount = 0;
			m_SensorNodes[i].m_DescendantCount = 0;
		}

		for (int i = 0; i < m_SensorNodes.size(); i++)
		{
			int64_t currentParent = m_SensorNodes[i].m_Parent;
			if (currentParent != SensorNode::c_BaseStationIndex && currentParent != SensorNode::c_InvalidIndex && currentParent != SensorNode::c_NoParentIndex)
				m_SensorNodes[m_SensorNodes[i].m_Parent].m_ChildCount++;
			while (currentParent != SensorNode::c_BaseStationIndex && currentParent != SensorNode::c_InvalidIndex && currentParent != SensorNode::c_NoParentIndex)
			{
				m_SensorNodes[currentParent].m_DescendantCount++;
				currentParent = m_SensorNodes[currentParent].m_Parent;
			}
		}

	}

	void ExampleSimulator::ColorTopology()
	{
		std::vector<SensorNode> tempSN = m_SensorNodes;

		for (int i = 0; i < tempSN.size(); i++)
		{
			if (tempSN[i].m_Parent == SensorNode::c_NoParentIndex)
			{
				tempSN.erase(tempSN.begin() + i);
				i--;
			}
		}

		for (int i = 0; i < tempSN.size(); i++)
		{
			for (int j = 0; j < tempSN.size(); j++)
			{
				if (SensorNode::Distance(tempSN[i], tempSN[j]) <= m_SimulatorParameters.InterferenceRange)
				{
					if (tempSN[i].m_WelshPowellDegree == -1)
						tempSN[i].m_WelshPowellDegree = 0;
					tempSN[i].m_WelshPowellDegree++;
				}
			}
		}

		std::sort(tempSN.begin(), tempSN.end(), [&](SensorNode sn1, SensorNode sn2) {
			return sn1.m_WelshPowellDegree > sn2.m_WelshPowellDegree;
			});

		bool exists = true;
		for (int i = 0; exists; i++)
		{
			exists = false;
			for (int j = 0; j < tempSN.size(); j++)
			{
				if (tempSN[j].m_Color != -1)
					continue;
				bool con = false;
				for (int k = 0; k < tempSN.size(); k++)
					if (tempSN[k].m_Color == i && SensorNode::Distance(tempSN[j], tempSN[k]) <= m_SimulatorParameters.InterferenceRange) {
						con = true;
						break;
					}
				if (!con) {
					tempSN[j].m_Color = i;
					exists = true;
				}
			}
		}

		for (int i = 0; i < tempSN.size(); i++)
			m_SensorNodes[tempSN[i].m_ID].m_Color = tempSN[i].m_Color;
	}

	void ExampleSimulator::SetSNDeltas()
	{
		Distribution uniformDist(DistributionType::Uniform, 1000.0, 1.0);

		for (int i = 0; i < m_SensorNodes.size(); i++)
			m_SensorNodes[i].m_DeltaOpt = uniformDist.GenerateRandomNumber();
	}

	void ExampleSimulator::Simulate()
	{
		SimulatorResults& sr = m_SimulatorResults;

		struct WorkingStateTimestamp
		{
			int64_t SNID;
			WorkingState State;
			double Timestamp;
		};

		std::unordered_map<int64_t, WorkingStateTimestamp> previousEvents;
		auto pqCompare = [](WorkingStateTimestamp left, WorkingStateTimestamp right)
		{
			if (left.Timestamp > right.Timestamp)
				return true;
			else if (left.Timestamp < right.Timestamp)
				return false;

			return left.SNID < right.SNID;
		};

		std::priority_queue<WorkingStateTimestamp, std::vector<WorkingStateTimestamp>, decltype(pqCompare)> eventQueue(pqCompare);
		for (int i = 0; i < m_SensorNodes.size(); i++)
		{
			if (m_SensorNodes[i].m_CurrentParent == SensorNode::c_NoParentIndex)
				continue;
			eventQueue.push({ (int64_t)i, WorkingState::Collection, 0.0 });
			previousEvents[(int64_t)i] = { (int64_t)i, WorkingState::Collection, 0.0 };
		}


		int colorCount = -1;
		for (int i = 0; i < m_SensorNodes.size(); i++)
			colorCount = std::max(colorCount, (int)m_SensorNodes[i].m_Color);
		colorCount++;

		double transferredTotalDuration = 0;
		double currentTime = 0.0;
		int failureCount = 0;

		int superSlotIterator = 0;

		bool isDone = false;

		while (!isDone && !eventQueue.empty())
		{
			auto currentEvent = eventQueue.top();
			currentTime = currentEvent.Timestamp;
			auto& currentState = currentEvent.State;
			auto& currentSN = currentEvent.SNID;
			eventQueue.pop();


			superSlotIterator = currentTime / (m_SimulatorParameters.TransferTime * colorCount);


			{
				double nextTime = currentTime;
				WorkingState nextState;
				if (currentState == WorkingState::Collection)
				{


					double optimalTime = currentTime + m_SensorNodes[currentSN].m_DeltaOpt;
					int k = ceil((((currentTime + m_SensorNodes[currentSN].m_DeltaOpt) / m_SimulatorParameters.TransferTime) - m_SensorNodes[currentSN].m_CurrentColor) / colorCount);
					nextTime = (k * colorCount + m_SensorNodes[currentSN].m_CurrentColor) * m_SimulatorParameters.TransferTime;

					if (std::abs(nextTime - optimalTime) > std::abs((nextTime - m_SimulatorParameters.TransferTime * colorCount) - optimalTime))
						nextTime -= m_SimulatorParameters.TransferTime * colorCount;
					while (nextTime < currentTime)
						nextTime += m_SimulatorParameters.TransferTime * colorCount;

					nextState = WorkingState::Transfer;

				}
				else if (currentState == WorkingState::Transfer)
				{
					nextTime += m_SimulatorParameters.TransferTime;
					nextState = WorkingState::Collection;
				}
				else if (currentState == WorkingState::Recovery)
				{
					nextTime += m_SimulatorParameters.RecoveryTime;
					nextState = WorkingState::Collection;
				}

				if (m_SensorNodes[currentSN].m_FailureIterator >= m_SensorNodes[currentSN].m_FailureTimestamps.size())
					std::cout << "Ran out of failures!\n";
				if (m_SensorNodes[currentSN].m_FailureIterator < m_SensorNodes[currentSN].m_FailureTimestamps.size() &&
					nextTime >= m_SensorNodes[currentSN].m_FailureTimestamps[m_SensorNodes[currentSN].m_FailureIterator])
				{
					eventQueue.push({ currentSN, WorkingState::Recovery, m_SensorNodes[currentSN].m_FailureTimestamps[m_SensorNodes[currentSN].m_FailureIterator] });
					m_SensorNodes[currentSN].m_FailureIterator++;
				}
				else
				{
					eventQueue.push({ currentSN, nextState, nextTime });
				}
			}

			if (previousEvents[currentSN].State == WorkingState::Collection)
			{
				if (currentState == WorkingState::Collection)
				{
					m_SensorNodes[currentSN].m_Packets.push_back({ currentSN, currentTime });
					m_SensorNodes[currentSN].m_CurrentPacketIterator = m_SensorNodes[currentSN].m_Packets.size() - 1;
				}
				else if (currentState == WorkingState::Transfer)
				{
					m_SensorNodes[currentSN].m_CollectionTime += currentTime - previousEvents[currentSN].Timestamp;
					m_SensorNodes[currentSN].m_CurrentData += currentTime - previousEvents[currentSN].Timestamp;
					m_SensorNodes[currentSN].m_EnergyConsumed += (currentTime - previousEvents[currentSN].Timestamp) * m_SimulatorParameters.EnergyRateSensing + s_EnergyTransitionWorkingToTransfer;
					m_SensorNodes[currentSN].m_Packets[m_SensorNodes[currentSN].m_CurrentPacketIterator].Size += currentTime - m_SensorNodes[currentSN].m_Packets[m_SensorNodes[currentSN].m_CurrentPacketIterator].InitialTimestamp;
				}
				else if (currentState == WorkingState::Recovery)
				{
					m_SensorNodes[currentSN].m_EnergyWasted += (currentTime - previousEvents[currentSN].Timestamp) * m_SimulatorParameters.EnergyRateSensing;
					{
						for (int i = 0; i < m_SensorNodes[currentSN].m_Packets.size(); i++)
						{
							int energyCurrentSN = m_SensorNodes[currentSN].m_Packets[i].InitialSNID;
							while (energyCurrentSN != currentSN)
							{
								double distance = SensorNode::Distance(m_SensorNodes[energyCurrentSN], m_SensorNodes[m_SensorNodes[energyCurrentSN].m_CurrentParent]);
								m_SensorNodes[energyCurrentSN].m_EnergyWasted += m_SensorNodes[currentSN].m_Packets[i].Size * m_SimulatorParameters.EnergyRateSensing + s_EnergyTransitionWorkingToTransfer;
								m_SensorNodes[energyCurrentSN].m_EnergyWasted += distance * distance * m_SimulatorParameters.TransferTime * m_SimulatorParameters.EnergyRateTransfer + s_EnergyTransitionTransferToWorking;

								energyCurrentSN = m_SensorNodes[energyCurrentSN].m_CurrentParent;
							}
						}
					}

					m_SensorNodes[currentSN].m_WastedTime += currentTime - previousEvents[currentSN].Timestamp;
					failureCount++;
					m_SensorNodes[currentSN].m_EnergyConsumed += (currentTime - previousEvents[currentSN].Timestamp) * m_SimulatorParameters.EnergyRateSensing;
					m_SensorNodes[currentSN].m_Packets.clear();
					m_SensorNodes[currentSN].m_CurrentPacketIterator = -1;
					m_SensorNodes[currentSN].m_CurrentData = 0;
				}
			}
			else if (previousEvents[currentSN].State == WorkingState::Transfer)
			{
				if (currentState == WorkingState::Collection)
				{
					if (m_SensorNodes[currentSN].m_CurrentParent != SensorNode::c_NoParentIndex)
					{

						if (m_SensorNodes[currentSN].m_CurrentParent != SensorNode::c_BaseStationIndex)
						{
							if (previousEvents[m_SensorNodes[currentSN].m_CurrentParent].State != WorkingState::Recovery)
							{
								m_SensorNodes[m_SensorNodes[currentSN].m_CurrentParent].m_CurrentData += m_SensorNodes[currentSN].m_CurrentData;
								for (int i = 0; i < m_SensorNodes[currentSN].m_Packets.size(); i++)
									m_SensorNodes[m_SensorNodes[currentSN].m_CurrentParent].m_Packets.push_back(m_SensorNodes[currentSN].m_Packets[i]);
							}
						}
						else
						{
							transferredTotalDuration += m_SensorNodes[currentSN].m_CurrentData;
							for (int i = 0; i < m_SensorNodes[currentSN].m_Packets.size(); i++)
							{
								m_SensorNodes[m_SensorNodes[currentSN].m_Packets[i].InitialSNID].m_SentPacketTotalDelay += currentTime - m_SensorNodes[currentSN].m_Packets[i].InitialTimestamp;
								m_SensorNodes[m_SensorNodes[currentSN].m_Packets[i].InitialSNID].m_SentPacketCount++;

								m_SensorNodes[m_SensorNodes[currentSN].m_Packets[i].InitialSNID].m_TotalDataSent += m_SensorNodes[currentSN].m_Packets[i].Size;
							}
						}

						double distance = 0.0;
						double posx = m_SensorNodes[currentSN].m_Position.X;
						double posy = m_SensorNodes[currentSN].m_Position.X;
						if (m_SensorNodes[currentSN].m_CurrentParent == SensorNode::c_BaseStationIndex)
							distance = std::sqrt(posx * posx + posy * posy);
						else
							distance = SensorNode::Distance(m_SensorNodes[currentSN], m_SensorNodes[m_SensorNodes[currentSN].m_CurrentParent]);



						if (m_SensorNodes[currentSN].m_CurrentParent == SensorNode::c_BaseStationIndex || previousEvents[m_SensorNodes[currentSN].m_CurrentParent].State != WorkingState::Recovery)
						{
							m_SensorNodes[currentSN].m_Packets.clear();
							m_SensorNodes[currentSN].m_CurrentData = 0;
						}
						m_SensorNodes[currentSN].m_EnergyConsumed += distance * distance * (currentTime - previousEvents[currentSN].Timestamp) * m_SimulatorParameters.EnergyRateTransfer + s_EnergyTransitionTransferToWorking;

						m_SensorNodes[currentSN].m_WastedTime += m_SimulatorParameters.TransferTime;
						m_SensorNodes[currentSN].m_Packets.push_back({ currentSN, currentTime });
						m_SensorNodes[currentSN].m_CurrentPacketIterator = m_SensorNodes[currentSN].m_Packets.size() - 1;
					}
				}
				else if (currentState == WorkingState::Transfer) {}
				else if (currentState == WorkingState::Recovery)
				{
					m_SensorNodes[currentSN].m_CurrentData = 0;
					m_SensorNodes[currentSN].m_WastedTime += currentTime - previousEvents[currentSN].Timestamp;
					failureCount++;

					{
						for (int i = 0; i < m_SensorNodes[currentSN].m_Packets.size(); i++)
						{
							int energyCurrentSN = m_SensorNodes[currentSN].m_Packets[i].InitialSNID;
							while (energyCurrentSN != currentSN)
							{
								double distance = SensorNode::Distance(m_SensorNodes[energyCurrentSN], m_SensorNodes[m_SensorNodes[energyCurrentSN].m_CurrentParent]);
								m_SensorNodes[energyCurrentSN].m_EnergyWasted += m_SensorNodes[currentSN].m_Packets[i].Size * m_SimulatorParameters.EnergyRateSensing + s_EnergyTransitionWorkingToTransfer;
								m_SensorNodes[energyCurrentSN].m_EnergyWasted += distance * distance * m_SimulatorParameters.TransferTime * m_SimulatorParameters.EnergyRateTransfer + s_EnergyTransitionTransferToWorking;

								energyCurrentSN = m_SensorNodes[energyCurrentSN].m_CurrentParent;
							}
						}
					}

					double distance = 0.0;
					double posx = m_SensorNodes[currentSN].m_Position.X;
					double posy = m_SensorNodes[currentSN].m_Position.X;
					if (m_SensorNodes[currentSN].m_CurrentParent == SensorNode::c_BaseStationIndex)
						distance = std::sqrt(posx * posx + posy * posy);
					else
						distance = SensorNode::Distance(m_SensorNodes[currentSN], m_SensorNodes[m_SensorNodes[currentSN].m_CurrentParent]);

					m_SensorNodes[currentSN].m_EnergyConsumed += distance * distance * (currentTime - previousEvents[currentSN].Timestamp) * m_SimulatorParameters.EnergyRateTransfer + s_EnergyTransitionTransferToWorking;
					m_SensorNodes[currentSN].m_EnergyWasted += distance * distance * (currentTime - previousEvents[currentSN].Timestamp) * m_SimulatorParameters.EnergyRateTransfer + s_EnergyTransitionTransferToWorking;

					m_SensorNodes[currentSN].m_Packets.clear();
					m_SensorNodes[currentSN].m_CurrentPacketIterator = -1;

				}
			}
			else if (previousEvents[currentSN].State == WorkingState::Recovery)
			{

				if (currentState == WorkingState::Collection)
				{
					m_SensorNodes[currentSN].m_WastedTime += m_SimulatorParameters.RecoveryTime;
					m_SensorNodes[currentSN].m_Packets.push_back({ currentSN, currentTime });
					m_SensorNodes[currentSN].m_CurrentPacketIterator = m_SensorNodes[currentSN].m_Packets.size() - 1;

				}
				else if (currentState == WorkingState::Transfer) {}
				else if (currentState == WorkingState::Recovery)
				{
					m_SensorNodes[currentSN].m_WastedTime += currentTime - previousEvents[currentSN].Timestamp;
					failureCount++;
					m_SensorNodes[currentSN].m_Packets.clear();
					m_SensorNodes[currentSN].m_CurrentPacketIterator = -1;
				}
			}

			previousEvents[currentSN] = currentEvent;



			isDone = IsDone(currentTime);
		}

		sr.ActualTotalDuration = currentTime;
		sr.FinalFailureIndex = failureCount;

		m_TransferredTotalDuration = transferredTotalDuration;

	}

	bool ExampleSimulator::IsDone(double currentTime)
	{
		return currentTime >= m_SimulatorParameters.TotalSimulationTime;
	}






}

