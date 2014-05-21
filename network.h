#ifndef _NETWORK_H
#define _NETWORK_H 1

#include <string>
#include <queue>
#include <map>
#include <vector>
#include <exception>
#include <stdexcept>
#include <cstdlib>

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
	float shutdown_probability;
	bool shutdowned;

	Process(string type, float shutdown_probability) : 
		type(type),
		context(NULL),
		shutdown_probability(shutdown_probability),
		shutdowned(false) {};
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

	void applyInstability();

public:
	Network() : idle_ticks(0) {};

	void registerWorkFunction(const string &name, WorkFunction function);

	int addProcess(const string &function_name, float shutdown_probability = 0.0);

	void sendMessage(Message msg);

	void nextTick();

	void run();
};

#endif