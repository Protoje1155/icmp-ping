#include "ping.h"

int g_run = true;

static void handle_sigint(int sig)
{
	(void)sig;
	g_run = false;
}

void setup_signal(t_data *data)
{
	struct sigaction sa;
	sa.sa_handler = handle_sigint;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGINT, &sa, NULL) == -1)
		error_exit(data, "sigaction", true);
}
