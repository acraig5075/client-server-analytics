#pragma once

#include <condition_variable>
#include <thread>
#include <mutex>
#include <queue>
#include <string>

namespace sql
	{
	class Connection;
	}

class ProducerConsumer
{
public:
	ProducerConsumer(const std::shared_ptr<sql::Connection> &db);

	void Produce(const std::string &str);
	void Consume();
	void StopConsuming();

private:
	const std::shared_ptr<sql::Connection> &m_db = nullptr;
	std::queue<std::string> m_queue;
	std::condition_variable m_cv;
	std::thread m_thread;
	std::mutex m_mtx;
	bool m_done = false;
};
