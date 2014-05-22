#ifndef _NETWORK_H
#define _NETWORK_H 1

#include <string>
#include <queue>
#include <map>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <cstdio>

using namespace std;

// Свой класс ошибки для случая обращения к несуществующему процессу
class bad_process_id : public std::logic_error {
public:
	bad_process_id(): std::logic_error("Bad process ID") {}
};

// Предварительное объявление класса
class Network;

struct Message
{
	int from, to;
	string data;
	bool empty; // флаг "пустого" сообщения

	// Пустое сообщение рассылается всем процессам в конце каждого тика,
	// независимо от того получал ли процесс какие-либо сообщения

	Message(int from, int to, string data) : from(from), to(to), data(data), empty(false) {}; // конструктор "нормального" сообщения
	Message(int to) : to(to), from(-1), empty(true) {}; // конструктор "пустого" сообщения
};

struct Process
{
	string type;   // имя зарегестриованной функции-обработчика
	void* context; // контекст процесса (см. dummy_function в main.cpp)
	float shutdown_probability; // вероятность того что процесс упадет (навсегда) в течении тика
	bool shutdowned;

	Process(string type, float shutdown_probability) : 
		type(type),
		context(NULL),
		shutdown_probability(shutdown_probability),
		shutdowned(false) {};
};

// шаблон функции-обработчика
typedef void* (* WorkFunction)(int id, Network *network, void *context, Message msg);

// Класс моделирующий сеть и работу процессов
class Network
{
	// сколько тиков без сообщений подряд должно произойти, чтобы сеть завершила свою работу
	const static int max_idle_ticks = 100;

	// текущее количество тиков без сообщений подряд
	int idle_ticks;

	queue<Message> message_queue;
	map<string, WorkFunction> work_functions;
	vector<Process> processes;

	// доставить сообщение процессу
	void processMessage(Message msg);

	// разослать "пустые" сообщения всем процессам
	void runIdleCalls();

	// для каждого процесса "убивает" его с заданной вероятностью
	void applyInstability();

public:
	Network() : idle_ticks(0) { srand(clock()); };

	void registerWorkFunction(const string &name, WorkFunction function);

	int addProcess(const string &function_name, float shutdown_probability = 0.0);

	// добавляет сообщение в очередь (не доставляет его процессу)
	void sendMessage(Message msg);

	// итерация работы сети
	void nextTick();

	// полный запуск запуск сети, завершается по достижению max_idle_ticks
	void run();

	// возвращает количество процессов в сети
	int nOfProcesses();

	// возвращает true, если процесс жив
	bool ifAlive(int);

        // убивает один процесс с 100% вероятностью
        void applyInstability(int);
};

#endif
