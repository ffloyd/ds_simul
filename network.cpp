#include "network.h"

void Network::processMessage(Message msg)
{
	int id = msg.to;

	if (id >= processes.size() || id < 0)
	{
		throw bad_process_id();
	}

	WorkFunction wf = work_functions[processes[id].type];

	if (processes[id].context == NULL)
	{
		processes[id].context = wf(id, this, NULL, Message(id));
	}

	wf(id, this, processes[id].context, msg);
}

void Network::runIdleCalls()
{
	for (int process_id = 0; process_id < processes.size(); ++process_id)
	{
		processMessage(Message(process_id));
	}
}

void Network::registerWorkFunction(const string &name, WorkFunction function)
{
	work_functions[name] = function;
}

int Network::addProcess(const string &function_name)
{
	int id = processes.size();
	processes.push_back(Process(function_name));

	return id;
}

void Network::sendMessage(Message msg)
{
	message_queue.push(msg);
}

void Network::nextTick()
{
	queue<Message> temp_queue;

	while(!message_queue.empty()) {
	    temp_queue.push(message_queue.front());
	    message_queue.pop();
	}

	if (temp_queue.empty())
	{
		idle_ticks++;
	}
	else
	{
		idle_ticks = 0;
		while(!temp_queue.empty()) {
		    processMessage(temp_queue.front());
		    temp_queue.pop();
		}
	}

	runIdleCalls();
}

void Network::run()
{
	while(idle_ticks < max_idle_ticks)
	{
		nextTick();
	}
	printf("All things done.\n");
}