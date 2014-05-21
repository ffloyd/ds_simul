#include "network.h"

void* Network::callProcess(int id, Message msg)
{
	void* context;
	if (id < process_contexts.size())
	{
		context = process_contexts[id];
	}
	else
	{
		context = NULL;
	}
	return process_types[processes[id]](id, this, context, msg);
}

void Network::idleCalls()
{
	for (int i = 0; i < processes.size(); ++i)
	{
		callProcess(i, Message());
	}
}

void Network::registerWorkFunction(const string &name, work_function functor)
{
	process_types[name] = functor;
}

int Network::addProcess(const string &work_function)
{
	int id = processes.size();
	processes.push_back(work_function);

	void* context = callProcess(id, Message());
	process_contexts.push_back(context);

	return id;
}

void Network::sendMessage(Message msg)
{
	message_queue.push(msg);
}

bool Network::nextTick()
{
	queue<Message> temp_queue;

	while(!message_queue.empty()) {
	    temp_queue.push(message_queue.front());
	    message_queue.pop();
	}

	if (temp_queue.empty())
	{
		idle_ticks++;
		idleCalls();
		return false;
	}
	else
	{
		idle_ticks = 0;
		while(!temp_queue.empty()) {
		    Message msg = temp_queue.front();
		    temp_queue.pop();

		    callProcess(msg.to, msg);
		}
	}

	idleCalls();
	return true;
}

void Network::run()
{
	while(idle_ticks < max_idle_ticks)
	{
		nextTick();
	}
	printf("All things done.\n");
}