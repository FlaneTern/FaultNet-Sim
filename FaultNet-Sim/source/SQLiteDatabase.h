#pragma once

#include "DatabaseData.h"

namespace FaultNet_Sim
{

	class SQLiteDatabase
	{
	public:
		inline static std::shared_ptr<SQLiteDatabase> Get() { return s_Database; }


		void PushQueue(Data data);

		void Log();

		inline void* GetConnection() { return m_Connection; }

		void Join();

	private:
		SQLiteDatabase(std::string dbName);

		static std::shared_ptr<SQLiteDatabase> s_Database;

		void* m_Connection = nullptr;

		bool m_Done = false;

		std::mutex m_Mutex;

		std::condition_variable m_DataCondition;

		std::queue<Data> m_DataQueue;

		std::thread m_LoggerThread;
	};

}
