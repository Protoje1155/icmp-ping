#include "ping.h"

void error_exit(t_data *data, char *msg, bool use_perror)
{
	if (data->target_ip)
		free(data->target_ip);
	if (data->target_url)
		free(data->target_url);
	if (data->icmp_socket_fd > 0)
		close(data->icmp_socket_fd);
	if (use_perror)
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "ft_ping: %s", msg);
		perror(buf);
	}    
	else if (msg != NULL)
		fprintf(stderr, "ft_ping: %s\n", msg);
	exit(1);
}

void success_exit(t_data *data)
{
	if (data->target_ip)
		free(data->target_ip);
	if (data->target_url)
		free(data->target_url);
	if (data->icmp_socket_fd > 0)
		close(data->icmp_socket_fd);
	exit(0);
}
