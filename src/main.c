#include "ping.h"

static void print_stats(t_data *data)
{
	t_stats stats = data->stats;
	printf("--- %s ping statistics ---\n", data->target_url);
	int lost = stats.n_transmitted - stats.n_received;
	int packet_loss = 0;
	if (stats.n_transmitted > 0)
		packet_loss = (lost * 100) / stats.n_transmitted;
	printf("%d packets transmitted, %d packets received, %d%% packet loss\n", stats.n_transmitted, stats.n_received, packet_loss);

	double avg = 0.0;
	if (stats.n_received > 0)
		avg = stats.rtt_sum / stats.n_received;
	
	// stddev = sqrt{[sum(rtt^2) / N] - ([sum(rtt) \ N])^2}
	double stddev = 0.0;
	if (stats.n_received > 0)
		stddev = sqrt((stats.rtt_sum_sq / stats.n_received) - pow((stats.rtt_sum / stats.n_received), 2));

	if (stats.rtt_min == DBL_MAX)
		stats.rtt_min = 0;
	
	printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", stats.rtt_min, avg, stats.rtt_max, stddev);
	if (packet_loss == 100)
		error_exit(data, NULL, false);
}

int main(int argc, char **argv)
{
	t_data data;

	parse(&data, argc, argv);
	init_socket(&data);
	setup_signal(&data);
	ping(&data);
	print_stats(&data);
	success_exit(&data);
}
