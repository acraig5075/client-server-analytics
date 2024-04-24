#pragma once

#include <thread>
#include <mutex>
#include <queue>
#include <string>
#include <sqlite3.h>

class ProducerConsumer
{
public:
	ProducerConsumer(sqlite3 *db);

	void produce(const std::string &str);
	void consume();
	void stopConsuming();

private:
	sqlite3 *m_db = nullptr;
	std::queue<std::string> m_queue;
	std::condition_variable m_cv;
	std::thread m_thread;
	std::mutex m_mtx;
	bool m_done = false;
};
