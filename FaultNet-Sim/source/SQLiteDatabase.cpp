#include "PCH.h"

#include "sqlite3.h"
#include "SQLiteDatabase.h"


namespace FaultNet_Sim
{

    std::shared_ptr<SQLiteDatabase> SQLiteDatabase::s_Database(new SQLiteDatabase("Results/Main.db"));

	SQLiteDatabase::SQLiteDatabase(std::string dbName)
	{
        bool exists = std::filesystem::exists(dbName);
        if (exists)
            std::filesystem::remove(dbName);

        int rc = sqlite3_open(dbName.c_str(), (sqlite3**)(&m_Connection));

        if (rc)
            throw std::runtime_error("Can't open database: " + std::string(sqlite3_errmsg((sqlite3*)m_Connection)));

        std::vector<std::string> createTableQueries =
        {
            R"(
                create table Problem(
	                ProblemID blob not null,
                    Description VARCHAR(64),
                    primary key(ProblemID)
                );
            )",

            R"(
                create table Simulator(
	                SimulatorID blob not null,
	                ProblemID blob not null,
                    Description VARCHAR(64),
                    SimulatorType VARCHAR(64),
                    TotalSimulationTime real,
                    TransferTime real,
                    RecoveryTime real,
                    EnergyRateSensing real,
                    EnergyRateTransfer real,
                    TransmissionRange real,
                    InterferenceRange real,
                    TransferredTotalDuration real,
                    primary key(ProblemID, SimulatorID),
                    foreign key(ProblemID) references Problem(ProblemID)
                );
            )",

            R"(
                create table SensorNode(
                    SensorNodeID blob not null,
	                SimulatorID blob not null,
	                ProblemID blob not null,
                    PosX real,
                    PosY real,
                    Parent blob,
                    Level_ blob,
                    DeltaOpt real,
                    CollectionTime real,
                    WastedTime real,
                    TotalDataSent real,
                    EnergyConsumed real,
                    EnergyWasted real,
                    SentPacketTotalDelay real,
                    SentPacketCount blob,
                    Color blob,
                    FailureMean real,
                    ChildCount blob,
                    DescendantCount blob,
                    primary key(ProblemID, SimulatorID, SensorNodeID),
                    foreign key(SimulatorID) references Simulator(SimulatorID),
                    foreign key(ProblemID) references Problem(ProblemID)
                );
            )",

        };


        for (int i = 0; i < createTableQueries.size(); i++)
        {
            if (sqlite3_exec((sqlite3*)m_Connection, createTableQueries[i].c_str(), NULL, 0, NULL) != SQLITE_OK)
            {
                std::cerr << "Failed to create table: " << sqlite3_errmsg((sqlite3*)m_Connection) << std::endl;
                throw std::runtime_error("HERE");
            }
        }

        sqlite3_exec((sqlite3*)m_Connection, "BEGIN TRANSACTION;", NULL, NULL, NULL);

        m_LoggerThread = std::thread(&SQLiteDatabase::Log, this);
	}


    void bindSNData(sqlite3_stmt* statement, Data data)
    {
        SensorNodeData* snData = (SensorNodeData*)data.m_Data.data();

        sqlite3_bind_int64 (statement,  1, snData->SensorNodeID);
        sqlite3_bind_int64 (statement,  2, snData->SimulatorID);
        sqlite3_bind_int64 (statement,  3, snData->ProblemID);
        sqlite3_bind_double(statement,  4, snData->PositionX);
        sqlite3_bind_double(statement,  5, snData->PositionY);
        sqlite3_bind_int64 (statement,  6, snData->Parent);
        sqlite3_bind_int64 (statement,  7, snData->Level); 
        sqlite3_bind_double(statement,  8, snData->DeltaOpt);
        sqlite3_bind_double(statement,  9, snData->CollectionTime);
        sqlite3_bind_double(statement, 10, snData->WastedTime);
        sqlite3_bind_double(statement, 11, snData->TotalDataSent);
        sqlite3_bind_double(statement, 12, snData->EnergyConsumed);
        sqlite3_bind_double(statement, 13, snData->EnergyWasted);
        sqlite3_bind_double(statement, 14, snData->SentPacketTotalDelay);
        sqlite3_bind_int64 (statement, 15, snData->SentPacketCount); 
        sqlite3_bind_int64 (statement, 16, snData->Color); 
        sqlite3_bind_double(statement, 17, snData->FailureMean);
        sqlite3_bind_int64 (statement, 18, snData->ChildCount);
        sqlite3_bind_int64 (statement, 19, snData->DescendantCount);
    }

    void bindProblemData(sqlite3_stmt* statement, Data data)
    {
        ProblemData* pData = (ProblemData*)data.m_Data.data();

        sqlite3_bind_int64(statement, 1, pData->ProblemID);
        sqlite3_bind_text(statement, 2, pData->Description, 64, SQLITE_TRANSIENT);
    }

    void bindSimulatorData(sqlite3_stmt* statement, Data data)
    {
        SimulatorData* sData = (SimulatorData*)data.m_Data.data();

        sqlite3_bind_int64(statement, 1, sData->SimulatorID);
        sqlite3_bind_int64(statement, 2, sData->ProblemID);
        sqlite3_bind_text(statement, 3, sData->Description, 64, SQLITE_TRANSIENT);
        sqlite3_bind_text(statement, 4, sData->SimulatorType, 64, SQLITE_TRANSIENT);
        sqlite3_bind_double(statement, 5, sData->TotalSimulationTime);
        sqlite3_bind_double(statement, 6, sData->TransferTime);
        sqlite3_bind_double(statement, 7, sData->RecoveryTime);
        sqlite3_bind_double(statement, 8, sData->EnergyRateSensing);
        sqlite3_bind_double(statement, 9, sData->EnergyRateTransfer);
        sqlite3_bind_double(statement, 10, sData->TransmissionRange);
        sqlite3_bind_double(statement, 11, sData->InterferenceRange);
        sqlite3_bind_double(statement, 12, sData->TransferredTotalDuration);

    }

    void SQLiteDatabase::PushQueue(Data data)
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_DataQueue.push(data);
        m_DataCondition.notify_one();
    }

    class SQLiteStatement
    {
    public:
        SQLiteStatement()
            : m_BindFunc(nullptr), m_Statement(nullptr) {}

        SQLiteStatement(std::string query, void(*bindFunc)(sqlite3_stmt*, Data))
        {
            sqlite3* con = (sqlite3*)SQLiteDatabase::Get()->GetConnection();
            if (sqlite3_prepare_v2(con, query.c_str(), -1, &m_Statement, 0) != SQLITE_OK)
            {
                std::cerr << "Failed to prepare statement:" << sqlite3_errmsg(con) << std::endl;
                throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(con)));
            }

            m_BindFunc = bindFunc;

        }

        void Step(Data data)
        {
            sqlite3* con = (sqlite3*)SQLiteDatabase::Get()->GetConnection();
            m_BindFunc(m_Statement, data);

            if (sqlite3_step(m_Statement) != SQLITE_DONE)
            {
                std::cerr << "Execution failed: " << sqlite3_errmsg(con) << std::endl;
                throw std::runtime_error("Execution failed: " + std::string(sqlite3_errmsg(con)));
            }

            sqlite3_reset(m_Statement);
        }

    private:
        void(*m_BindFunc)(sqlite3_stmt*, Data) = nullptr;
        sqlite3_stmt* m_Statement = nullptr;
    };

    void SQLiteDatabase::Log()
    {
        std::unordered_map<DataType, SQLiteStatement> statementMap =
        {
            {DataType::ProblemData, SQLiteStatement("INSERT INTO Problem VALUES(?, ?)", bindProblemData)},
            {DataType::SimulatorData, SQLiteStatement("INSERT INTO Simulator VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", bindSimulatorData)},
            {DataType::SensorNodeData, SQLiteStatement("INSERT INTO SensorNode VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)", bindSNData)},
        };


        while (true) 
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_DataCondition.wait(lock, [&] { return !m_DataQueue.empty() || m_Done; });

            if (m_Done && m_DataQueue.empty())
            {
                sqlite3_exec((sqlite3*)SQLiteDatabase::Get()->GetConnection(), "END TRANSACTION;", NULL, NULL, NULL);
                sqlite3_close((sqlite3*)SQLiteDatabase::Get()->GetConnection());
                break;
            }


            Data data = m_DataQueue.front();
            m_DataQueue.pop();
            lock.unlock();

            statementMap[data.m_DataType].Step(data);
            
        }
    }

    void SQLiteDatabase::Join()
    {
        {
            std::unique_lock<std::mutex> lock(m_Mutex);
            m_Done = true;
        }
        m_DataCondition.notify_one();
        m_LoggerThread.join();
    }
}
