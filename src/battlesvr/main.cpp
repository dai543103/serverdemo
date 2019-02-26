#include "common/logger.h"
#include "battleserver.h"
#include "common/base.h"
#include "common/json.hpp"
#include <fstream>

#ifdef WIN32
#define  LISTENER_CONFIG_PATH "../../tools/config/port.json"
#define LOGS_PATH "../../log/battle"
#else
#define  LISTENER_CONFIG_PATH "./listener.json"
#define LOGS_PATH "./logs/battle"
#endif


std::shared_ptr<spdlog::logger> logger;

struct ListenInfo
{
	std::string ip;
	std::string dbhost;
};

void initSpdlog()
{
#ifdef WIN32
 	logger = spdlog::stdout_color_mt("console");
 	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] %v");
 	spdlog::set_level(spdlog::level::trace);
#else
	logger = spdlog::rotating_logger_mt("rotate_log", LOGS_PATH, 1048576 * 5, 3);
#endif
	SPDLOG_INFO("start log");
}


#ifndef WIN32

void IgnoreSignal(int iSignalValue)
{
	struct sigaction sig;
	sig.sa_handler = SIG_IGN;
	sig.sa_flags = 0;
	sigemptyset(&sig.sa_mask);
	sigaction(iSignalValue, &sig, NULL);
}

//初始化为守护进程的函数
void DaemonLaunch(void)
{
	pid_t pid;
	if ((pid = fork()) != 0)
	{
		exit(0);
	}

	setsid();
	IgnoreSignal(SIGINT);
	IgnoreSignal(SIGHUP);
	IgnoreSignal(SIGQUIT);
	IgnoreSignal(SIGTTOU);
	IgnoreSignal(SIGTTIN);
	IgnoreSignal(SIGCHLD);
	IgnoreSignal(SIGTERM);
	IgnoreSignal(SIGHUP);
	IgnoreSignal(SIGPIPE);

	if ((pid = fork()) != 0)
	{
		exit(0);
	}
	umask(0);
}

#endif

int main()
{
#ifndef WIN32
	DaemonLaunch();
#endif
	initSpdlog();
	g_eventhandle->registAllHandle();

	std::ifstream i(LISTENER_CONFIG_PATH);
	nlohmann::json j;
	i >> j;
	g_battleserver->start(j);
}
