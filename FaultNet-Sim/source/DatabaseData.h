#pragma once

#include "Simulator.h"
#include "Problem.h"

namespace FaultNet_Sim
{

    static constexpr int s_MaxTextLength = 64;
    enum class DataType
    {
        ProblemData = 1,
        SimulatorData,
        SensorNodeData,

    };

    struct ProblemData
    {
        int64_t ProblemID;
        char Description[s_MaxTextLength];
    };

    struct SimulatorData
    {
        int64_t SimulatorID;
        int64_t ProblemID;
        char Description[s_MaxTextLength];
        char SimulatorType[s_MaxTextLength];
        double TotalSimulationTime;
        double TransferTime;
        double RecoveryTime;
        double EnergyRateSensing;
        double EnergyRateTransfer;
        double TransmissionRange;
        double InterferenceRange;
        double TransferredTotalDuration;
    };

    struct SensorNodeData
    {
        int64_t SensorNodeID;
        int64_t SimulatorID;
        int64_t ProblemID;
        double PositionX;
        double PositionY;
        int64_t Parent;
        int64_t Level;
        double DeltaOpt;
        double CollectionTime;
        double WastedTime;
        double TotalDataSent;
        double EnergyConsumed;
        double EnergyWasted;
        double SentPacketTotalDelay;
        int64_t SentPacketCount;
        int64_t Color;
        double FailureMean;
        int64_t ChildCount = -1;
        int64_t DescendantCount = -1;
    };

    class Data
    {
    public:
        DataType m_DataType;
        std::vector<std::byte> m_Data;

        static Data ConvertData(Problem& problem);
        static Data ConvertData(Simulator& simulator);
        static Data ConvertData(SensorNode& sn, int64_t simulatorID, int64_t problemID);
    };
}