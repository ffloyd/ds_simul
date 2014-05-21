#include <iostream>

#include "network.h"

using namespace std;

struct dummy_context
{
	int repeats;

	dummy_context() : repeats(3) {};
};

void* dummy_function(int id, Network *network, void *_context, Message msg)
{
	if (!_context)
	{
		return new dummy_context;
	}

	dummy_context* context = reinterpret_cast<dummy_context*>(_context);

	if (msg.empty)
	{
		if (context->repeats != 0)
		{
			printf("DUMMY[%d]: Hello, i'm dummy and i'm idle!\n", id);
			context->repeats--;
		}
	}
	else
	{
		printf("DUMMY[%d]: I received message '%s'!\n", id, msg.data.c_str());
	}

	return NULL;
}

int main(int argc, char const *argv[])
{
	Network network;

	network.registerWorkFunction("dummy", dummy_function);
	network.addProcess("dummy");
	network.addProcess("dummy");
	network.addProcess("dummy", 0.1);
	network.sendMessage(Message(-1, 0, "HELLO!"));

	network.run();

	return 0;
}