#include <iostream>

#include "network.h"

using namespace std;

// тип контекста - произвольный struct
struct dummy_context
{
	int repeats;

	dummy_context() : repeats(3) {};
};

void* dummy_function(int id, Network *network, void *_context, Message msg)
{
	// если в качестве контексте передан NULL мы должны создать и вернуть контекст
	if (!_context)
	{
		return new dummy_context;
	}

	// так можно привести к нужному типу полученный контекст
	dummy_context* context = reinterpret_cast<dummy_context*>(_context);

	// если сообщение пустое - то это вызов, который происходит в конце каждого тика для каждого процесса
	if (msg.empty)
	{
		if (context->repeats != 0)
		{
			printf("DUMMY[%d]: Hello, i'm dummy and i'm idle!\n", id);
			context->repeats--;
		}
	}
	else // если нет, очевидно, нам что-то прислали
	{
		printf("DUMMY[%d]: I received message '%s'!\n", id, msg.data.c_str());

		if (msg.data == "HELLO")
		{
			// а так происходит отсылка сообщений из процесса
			network->sendMessage(Message(id, 1, "I CAN SPEAK!"));
		}
	}

	return NULL;
}

struct bully_context
{
	const int timeout;

	//текущее число тиков, когда процесс не дождался нужного ответа
	int idle;

	//идентификатор начала выборов
	bool elections;

	//идентификатор ожидания сообщения COORDINATOR
	bool waitForCoordinator;
	
	bully_context() : idle(-1), timeout(5), elections(false), waitForCoordinator(false) {}; //даем -1, т.к. в конце каждого тика нам высылается пустое сообщение
};

void* bully_function(int id, Network *network, void *_context, Message msg)
{
        if (!_context)
        {
                return new bully_context;
        }
        bully_context* context = reinterpret_cast<bully_context*>(_context);

	if (msg.empty) 
	{	
		if (context->elections == true || context->waitForCoordinator == true)
		{
			context->idle++;
		}
	}
	else 
	{
		//printf("BULLY[%d]: I received message '%s'!\n", id, msg.data.c_str());
		context->idle = -1; //сбрасываем таймаут

		if (msg.data == "HELLO")
                {
			for (int i = id + 1; i < network->nOfProcesses(); i++)
			{
				if (network->ifAlive(i)) network->sendMessage(Message(id, i, "ELECTIONS"));
			}
			context->elections = true;
		}
		else if (msg.data == "ELECTIONS" && msg.from < id)
		{
			if (network->ifAlive(msg.from)) network->sendMessage(Message(id, msg.from, "ALIVE"));
			if (!context->elections)
			{
				for (int i = id + 1; i < network->nOfProcesses(); i++)
                        	{
                                	if (network->ifAlive(i)) network->sendMessage(Message(id, i, "ELECTIONS"));
                        	}
                        	context->elections = true;
			}
		}
		else if (msg.data == "COORDINATOR")
		{
			if (msg.from < id)
			{
                        	for (int i = id + 1; i < network->nOfProcesses(); i++)
                        	{
                        	        if (network->ifAlive(i)) network->sendMessage(Message(id, i, "ELECTIONS"));
                        	}
                        	context->elections = true;
			}
			else
			{
				context->elections = false;
				context->waitForCoordinator = false;
			}
                }
		else if (msg.data == "ALIVE")
		{
			context->waitForCoordinator = true;
			context->elections = false;
		}

	}
	
	if (context->idle == context->timeout)
	{
		if (context->elections)
		{
			//процесс считает себя победителем
			if (id + 1 == network->nOfProcesses())
			{
				network->applyInstability(id);
			}
			else
			{
				for (int i = 0; i < network->nOfProcesses(); i++)
                		{
					if (i != id)
					{
                        			if (network->ifAlive(i)) network->sendMessage(Message(id, i, "COORDINATOR"));
					}
                		}
				context->idle = -1; //сбрасываем таймаут
				context->elections = false; //заканчиваем режим выборов
			}
		}
	}
	if (context->idle == context->timeout + 1)
        {
		if (context->waitForCoordinator)
		{
			for (int i = id + 1; i < network->nOfProcesses(); i++)
                        {
                                if (network->ifAlive(i)) network->sendMessage(Message(id, i, "ELECTIONS"));
                        }
                        context->elections = true;
		}

	}

	return NULL;
}

int main(int argc, char const *argv[])
{
	Network network;

/*
	network.registerWorkFunction("dummy", dummy_function);
	network.addProcess("dummy");
	network.addProcess("dummy");
	network.addProcess("dummy", 0.5);
	network.sendMessage(Message(-1, 0, "HELLO"));
*/

	network.registerWorkFunction("bully", bully_function);
	network.addProcess("bully");
	network.addProcess("bully");
	network.addProcess("bully");
	network.addProcess("bully", 0.5);
	network.addProcess("bully");
	network.sendMessage(Message(-1, 0, "HELLO"));

	network.run();

	return 0;
}
