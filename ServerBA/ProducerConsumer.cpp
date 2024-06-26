#include "ProducerConsumer.h"
#include "../Utilities/SqlUtils.h"
#include <iostream>
#include <sqlite3.h>

ProducerConsumer::ProducerConsumer(sqlite3 *db)
	: m_dbs(db)
{
	m_thread = std::thread(&ProducerConsumer::consume, this);
}

ProducerConsumer::ProducerConsumer(sql::Connection *db)
	: m_dbm(db)
{
	m_thread = std::thread(&ProducerConsumer::consume, this);
}

void ProducerConsumer::produce(const std::string &str)
{
	std::unique_lock<std::mutex> lock(m_mtx);
	m_queue.push(str);
	m_cv.notify_one(); // Notify the consumer thread that new data is available
}

void ProducerConsumer::consume()
{
	if (!m_dbs && !m_dbm)
		return;

	while (true)
		{
		std::unique_lock<std::mutex> lock(m_mtx);

		// Wait until the queue is not empty
		m_cv.wait(lock, [this]
			{
			return !m_queue.empty() && !m_done;
			}); 

		while (!m_queue.empty())
			{
			int inserts = 0;
			
			if (m_dbm)
				inserts = InsertDatabase(m_dbm, m_queue.front());
			else if (m_dbs)
				inserts = InsertDatabase(m_dbs, m_queue.front());

			m_queue.pop();

			// TODO release lock while processing

			std::cout
				<< "Consumed "
				<< inserts
				<< "\n";
			}
		}
}

void ProducerConsumer::stopConsuming()
{
	m_done = true;
}
