#include "network.h"

void Network::processMessage(Message msg)
{
	int id = msg.to;

	if (id >= processes.size() || id < 0)
	{
		throw bad_process_id();
	}

	WorkFunction wf = work_functions[processes[id].type];

	// если у процесса еще нет контекста - создаем его (см. dummy_function в main.cpp)
	if (processes[id].context == NULL)
	{
		processes[id].context = wf(id, this, NULL, Message(id));
	}

	if (!processes[id].shutdowned)
	{
		// собственно доставка сообщения
		if (!msg.empty)
		{
			printf("NETWORK: delivered message '%s' from %d to %d\n", msg.data.c_str(), msg.from, msg.to);
		}
		wf(id, this, processes[id].context, msg);
	}
}

void Network::runIdleCalls()
{
	for (int process_id = 0; process_id < processes.size(); ++process_id)
	{
		processMessage(Message(process_id));
	}
}

void Network::applyInstability()
{
	for (int process_id = 0; process_id < processes.size(); ++process_id)
	{
		if (!processes[process_id].shutdowned)
		{
			float shutdown_roll = rand() / (float)(RAND_MAX);

			if (shutdown_roll < processes[process_id].shutdown_probability)
			{
				processes[process_id].shutdowned = true;
				printf("NETWORK: process with id %d shutdowned accidently...\n", process_id);
			}	
		}
	}
}

void Network::applyInstability(int id)
{
	if (!processes[id].shutdowned)
        {
		processes[id].shutdowned = true;
                printf("NETWORK: process with id %d shutdowned accidently...\n", id);
      	}
}


void Network::registerWorkFunction(const string &name, WorkFunction function)
{
	work_functions[name] = function;
}

int Network::addProcess(const string &function_name, float shutdown_probability)
{
	int id = processes.size();
	processes.push_back(Process(function_name, shutdown_probability));

	return id;
}

void Network::sendMessage(Message msg)
{
	message_queue.push(msg);
	if (!msg.empty)
	{
		printf("NETWORK: sent message '%s' from %d to %d\n", msg.data.c_str(), msg.from, msg.to);
	}
}

void Network::nextTick()
{
	// Перекладываем все сообщения в temp_queue, чтобы не начать обрабатывать сообщения следующего тика
	queue<Message> temp_queue;

	while(!message_queue.empty()) {
	    temp_queue.push(message_queue.front());
	    message_queue.pop();
	}

	applyInstability();

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
	printf("NETWORK: All things done.\n");
}

int Network::nOfProcesses()
{
	return processes.size();
}

bool Network::ifAlive(int id)
{
	if (processes[id].shutdowned)
		return false;
	else
		return true;
}
