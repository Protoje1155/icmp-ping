#include "ping.h"

void init_socket(t_data *data)
{
	data->icmp_socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (data->icmp_socket_fd < 0)
		error_exit(data, "socket", true);
	
	// set a receive-timeout on the socket
	struct timeval tv;
	tv.tv_sec = data->linger;
	tv.tv_usec = 0;
	if (setsockopt(data->icmp_socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
		error_exit(data, "setsockopt SO_RCVTIMEO", true);

	// set a tos on the socket
	if (setsockopt(data->icmp_socket_fd, IPPROTO_IP, IP_TOS, &data->tos, sizeof(data->tos)) < 0)
		error_exit(data, "setsockopt IP_TOS", true);
	
	// set ttl on the socket
	if (setsockopt(data->icmp_socket_fd, IPPROTO_IP, IP_TTL, &data->ttl, sizeof(data->ttl)) < 0)
		error_exit(data, "setsockopt IP_TTL", true);  
}
