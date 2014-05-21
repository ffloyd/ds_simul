#ifndef _NETWORK_H
#define _NETWORK_H 1

#include <string>
#include <queue>
#include <map>
#include <vector>

using namespace std;

class Network;

struct Message
{
	int from, to;
	string data;
	bool empty;

	Message(int from, int to, string data) : from(from), to(to), data(data), empty(false) {};
	Message() : empty(true) {};
};

typedef void* (* work_function)(int id, Network *network, void *context, Message msg);

class Network
{
	const static int max_idle_ticks = 100;

	int idle_ticks;

	queue<Message> message_queue;
	map<string, work_function> process_types;
	vector<string> processes;
	vector<void*>  process_contexts;

	void* callProcess(int id, Message msg);

	void idleCalls();

public:
	Network() : idle_ticks(0) {};

	void registerWorkFunction(const string &name, work_function functor);

	// returns process ID	
	int addProcess(const string &work_function);

	void sendMessage(Message msg);

	void log(int id, const char* format_string, ...);

	bool nextTick();

	void run();
};

#endif