#ifndef _NETWORK_H
#define _NETWORK_H 1

#include <string>
#include <queue>
#include <map>
#include <vector>
#include <exception>
#include <stdexcept>

using namespace std;

class bad_process_id : public logic_error {
public:
	bad_process_id(): logic_error("Bad process ID") {}
};

class Network;

struct Message
{
	int from, to;
	string data;
	bool empty;

	Message(int from, int to, string data) : from(from), to(to), data(data), empty(false) {};
	Message(int to) : to(to), from(-1), empty(true) {};
};

struct Process
{
	string type;
	void* context;

	Process(string type) : type(type), context(NULL) {};
};

typedef void* (* WorkFunction)(int id, Network *network, void *context, Message msg);

class Network
{
	const static int max_idle_ticks = 100;

	int idle_ticks;

	queue<Message> message_queue;
	map<string, WorkFunction> work_functions;
	vector<Process> processes;

	void processMessage(Message msg);

	void runIdleCalls();

public:
	Network() : idle_ticks(0) {};

	void registerWorkFunction(const string &name, WorkFunction function);

	int addProcess(const string &function_name);

	void sendMessage(Message msg);

	void nextTick();

	void run();
};

#endif