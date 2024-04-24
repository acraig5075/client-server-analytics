#include "ProducerConsumer.h"
#include "..\Utilities\SqlUtils.h"
#include <iostream>

ProducerConsumer::ProducerConsumer(sqlite3 *db)
	: m_db(db)
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
			int inserts = InsertDatabase(m_db, m_queue.front());

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
