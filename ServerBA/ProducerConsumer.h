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
	ProducerConsumer(sql::Connection *db);

	void produce(const std::string &str);
	void consume();
	void stopConsuming();

private:
	sql::Connection *m_dbm = nullptr;
	std::queue<std::string> m_queue;
	std::condition_variable m_cv;
	std::thread m_thread;
	std::mutex m_mtx;
	bool m_done = false;
};
