#include "ProducerConsumer.h"
#include "../Utilities/SqlUtils.h"
#include <iostream>

ProducerConsumer::ProducerConsumer(sql::Connection *db)
	: m_dbm(db)
{
	m_thread = std::thread(&ProducerConsumer::Consume, this);
}

void ProducerConsumer::Produce(const std::string &str)
{
	std::unique_lock<std::mutex> lock(m_mtx);
	m_queue.push(str);
	m_cv.notify_one(); // Notify the consumer thread that new data is available
}

void ProducerConsumer::Consume()
{
	if (!m_dbm)
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
			
			inserts = InsertDatabase(m_dbm, m_queue.front());

			m_queue.pop();

			// TODO release lock while processing

			std::cout
				<< "Consumed "
				<< inserts
				<< "\n";
			}
		}
}

void ProducerConsumer::StopConsuming()
{
	m_done = true;
}
