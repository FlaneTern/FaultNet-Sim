#pragma once
#include "Global.h"
#include "Simulator.h"

namespace FaultNet_Sim
{
	class Problem
	{
	public:

		template<typename T>
		static inline std::shared_ptr<Problem> CreateProblem(std::string description) 
		{ 
			std::shared_ptr<Problem> problem(new T(description));
			s_Problems.push_back(problem);
			return problem;
		}

		inline void AddSimulator(std::shared_ptr<Simulator> simulator) { m_Simulators.push_back(simulator->Clone()); m_Simulators.back()->i_ProblemData = i_ProblemData; }
		inline void AddSimulator(std::vector<std::shared_ptr<Simulator>> simulators) 
		{ for(auto& simulator : simulators) { m_Simulators.push_back(simulator->Clone()); m_Simulators.back()->i_ProblemData = i_ProblemData; } }

		void Initialize();
		void Run();

		static void Join();

		inline int64_t GetProblemID() { return m_ProblemID; }
		inline std::string GetDescription() { return m_Description; }

		I_ProblemData i_ProblemData;

	protected:
		Problem(std::string description = "");
		virtual void GenerateSNs();
		virtual void GenerateFailures();

		void GenerateSNsPost();
		void GenerateFailuresPost();

		int64_t m_ProblemID;
		std::string m_Description;

		std::vector<SensorNode> m_SensorNodes;

		std::vector<std::shared_ptr<Simulator>> m_Simulators;

		static int64_t GenerateID();

	private:
		bool m_HasRun = false;
		bool m_Done = false;

		static std::vector<std::shared_ptr<Problem>> s_Problems;
		static std::counting_semaphore<g_NumberOfThreads> s_Semaphore;
		static std::vector<std::thread> s_Threads;

		void Log();
	};
}