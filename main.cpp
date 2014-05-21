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

int main(int argc, char const *argv[])
{
	Network network;

	network.registerWorkFunction("dummy", dummy_function);
	network.addProcess("dummy");
	network.addProcess("dummy");
	network.addProcess("dummy", 0.5);
	network.sendMessage(Message(-1, 0, "HELLO"));

	network.run();

	return 0;
}