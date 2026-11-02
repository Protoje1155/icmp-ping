#include "ping.h"

static bool timeout_exceeded(struct timeval start, int sec_timeout)
{
	struct timeval check_time;
	gettimeofday(&check_time, NULL);
	return (check_time.tv_sec - start.tv_sec) >= sec_timeout;
}

static void resolve_host(t_data *data)
{
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	int err = getaddrinfo(data->target_url, NULL, &hints, &res);
	if (err != 0)
		error_exit(data, "unknown host", false);
	struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
	char ip_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &addr->sin_addr, ip_str, sizeof(ip_str));
	data->target_ip = strdup(ip_str);
	if (!data->target_ip)
		error_exit(data, "malloc", true);
	data->target_addr.sin_addr = addr->sin_addr;
	freeaddrinfo(res);
}

static void send_packet(t_data *data, t_packet *packet)
{
	// store rtt before send
	gettimeofday(&packet->sent_time, NULL);

	if (sendto(data->icmp_socket_fd, packet->packet, 8 + data->payload_size, 0, (struct sockaddr *)&data->target_addr, sizeof(data->target_addr)) < 0)
	{
		free_packet(packet);
		error_exit(data, "sendto", true);
	}
	data->stats.n_transmitted++;
}

void ping(t_data *data)
{
	resolve_host(data);
	if (data->flag_verbose)
		printf("PING %s (%s): %d data bytes, id 0x%x = %d\n", data->target_url, data->target_ip, data->payload_size, getpid() & 0xFFFF, getpid() & 0xFFFF);
	else
		printf("PING %s (%s): %d data bytes\n", data->target_url, data->target_ip, data->payload_size);
	struct timeval start_time;
	gettimeofday(&start_time, NULL);
	while (g_run)
	{
		if (data->flag_timeout && timeout_exceeded(start_time, data->timeout) ==  true)
			return;

		t_packet *packet = prep_icmp_packet(data);
		send_packet(data, packet);
		receive_packet(data, packet);
		free_packet(packet);

		if (data->flag_count && (data->max_count - data->current_count <= 0))
			return;

		if (g_run && data->sleep)
			sleep(1);
	}
}
