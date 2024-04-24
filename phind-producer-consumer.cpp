#include <condition_variable>
#include <future>

std::mutex gMutex;
std::condition_variable cv;
std::future<void> consumerFuture;
std::queue<std::string> gMsgs;
char ch = 'a';


void updateLog()
{
	std::string str = std::to_string(ch);
	gMsgs.push_back(str);
	
	if (ch == 'z')
		ch = 'a';
	else
		ch++;
}

void produce() {
    srand (time(NULL));
    timespec ts = {0, 5*1000000};

    gThread = std::thread([&]() {
        while(true) {
            updateLog();
            nanosleep(&ts, NULL);
            cv.notify_one(); // Notify the consumer thread that new data is available
        }
    });
    printf("log thread created.\n");
}

void consume() {
    timespec ts = {0, 10*1000000};

    while (true) {
        //std::unique_lock<std::mutex> lock(gMutex);
        std::lock_guard<std::mutex> lock(gMutex);
        cv.wait(lock, []{return !gMsgs.empty();}); // Wait until the queue is not empty

        std::string log;
        while (!gMsgs.empty()) {
            log += gMsgs.front() + "\n";
            gMsgs.pop_front();
        }

        if (log.empty()) {
            log = "EMPTY";
        }

        printf("log: %s\n", log.c_str());
        nanosleep(&ts, NULL);
    }
}

int main() {
    produce();
    consumerFuture = std::async(std::launch::async, consume);
    consumerFuture.wait(); // Wait for the consumer thread to finish (optional, for demonstration)
}
