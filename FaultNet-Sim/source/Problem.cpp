#include "PCH.h"
#include "Problem.h"
#include "DatabaseData.h"
#include "SQLiteDatabase.h"

namespace FaultNet_Sim
{
	std::vector<std::shared_ptr<Problem>> Problem::s_Problems;
	std::counting_semaphore<g_NumberOfThreads> Problem::s_Semaphore(g_NumberOfThreads);
	std::vector<std::thread> Problem::s_Threads;

	int64_t Problem::GenerateID()
	{
		static int64_t currentProblemID = 0;

		currentProblemID++;

		return currentProblemID;
	}

	Problem::Problem(std::string description)
		: m_ProblemID(GenerateID()), m_Description(description)
	{
	}

	void Problem::Initialize()
	{
		GenerateSNs();
		GenerateSNsPost();
		GenerateFailures();
		GenerateFailuresPost();

		Log();
	}

	void Problem::Run()
	{
		s_Threads.push_back(
				std::thread([&]()
					{
						if (m_HasRun)
							throw std::runtime_error("This problem has been ran !");
						m_HasRun = true;


						std::vector<std::thread> runThreads;

						auto runFunc = [&](std::shared_ptr<Simulator> sim) { sim->Run(m_ProblemID, m_SensorNodes); s_Semaphore.release(); };

						for (int i = 0; i < m_Simulators.size(); i++)
						{
							s_Semaphore.acquire();
							runThreads.push_back(std::thread(runFunc, m_Simulators[i]));
						}

						for (int i = 0; i < runThreads.size(); i++)
							runThreads[i].join();

					}
				)
			);
	}

	void Problem::GenerateSNs()
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

	void Problem::GenerateFailures()
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

	void Problem::GenerateSNsPost()
	{

	}

	void Problem::GenerateFailuresPost()
	{

	}

	void Problem::Log()
	{
		SQLiteDatabase::Get()->PushQueue(Data::ConvertData(*this));
	}


	void Problem::Join()
	{
		for (int i = 0; i < s_Threads.size(); i++)
		{
			s_Threads[i].join();
		}
	}
}
