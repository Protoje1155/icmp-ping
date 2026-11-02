#include "ping.h"

/*
 * ICMP Destination Unreachable packet structure:
 *
 * Outer IP header (ip):
 *   Source      - router/host sending the error
 *   Destination - our host
 *
 * Outer ICMP header:
 *   Type 3      - Destination Unreachable
 *
 * ICMP payload:
 *   Original IP header   (orig_ip)
 *   Original ICMP header (orig_icmp)
 *   First 8 bytes of the original payload
 */

static void print_verbose(char *res_buf)
{
	// outer IP header
	struct iphdr *ip = (struct iphdr *)res_buf;

	// outer ICMP header
	struct icmphdr *icmp = (struct icmphdr *)(res_buf + ip->ihl * 4);

	// original IP header
	struct iphdr *orig_ip = (struct iphdr *)((uint8_t *)icmp + 8);

	// original ICMP header
	struct icmphdr *orig_icmp = (struct icmphdr *)((uint8_t *)orig_ip + orig_ip->ihl * 4);

	uint8_t *buf = (uint8_t *)orig_ip;

	printf("IP Hdr Dump:\n ");
	for (size_t i = 0; i + 1 < orig_ip->ihl * 4; i += 2)
		printf("%04x ", (buf[i] << 8) | buf[i+1]);
	
	struct in_addr src_addr, dst_addr;
	src_addr.s_addr = orig_ip->saddr;
	dst_addr.s_addr = orig_ip->daddr;
	char src_str[INET_ADDRSTRLEN];
	char dst_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &src_addr, src_str, sizeof(src_str));
	inet_ntop(AF_INET, &dst_addr, dst_str, sizeof(dst_str));

	uint16_t flag_offset = ntohs(orig_ip->frag_off);
	u_int8_t flag = flag_offset >> 13;
	u_int16_t offset = flag_offset & 0x1FFF;

	printf("\nVr HL TOS  Len   ID Flg  off TTL Pro  cks      Src      Dst     Data\n");
	printf(" %d  %d  %02x %04x %04x   %d %04x  %02x  %02x %04x  %s  %s\n", 
			orig_ip->version,
			orig_ip->ihl,
			orig_ip->tos,
			ntohs(orig_ip->tot_len),
			ntohs(orig_ip->id),
			flag,
			offset,
			orig_ip->ttl,
			orig_ip->protocol,
			ntohs(orig_ip->check),
			src_str,
			dst_str);
	
	ssize_t total_len = ntohs(orig_ip->tot_len);
	ssize_t ip_hdr_len = orig_ip->ihl * 4;
	ssize_t icmp_msg_size = total_len - ip_hdr_len;

	printf("ICMP: type %d, code %d, size %ld, id 0x%04x, seq 0x%04x\n",
			orig_icmp->type,
			orig_icmp->code,
			icmp_msg_size,
			ntohs(orig_icmp->un.echo.id),
			ntohs(orig_icmp->un.echo.sequence)); 
}

static bool is_our_packet(char *res)
{
	struct iphdr *ip = (struct iphdr *)res;
	struct icmphdr *icmp = (struct icmphdr *)(res + ip->ihl * 4);
	struct iphdr *orig_ip = (struct iphdr *)((uint8_t *)icmp + 8);
	struct icmphdr *orig_icmp = (struct icmphdr *)((uint8_t *)orig_ip + orig_ip->ihl * 4);
	if (ntohs(orig_icmp->un.echo.id) == (getpid() & 0xFFFF))
		return true;
	return false;
}

static double calculate_rtt(t_packet *packet)
{
	struct timeval after;
	gettimeofday(&after, NULL);
	double ms_after = after.tv_sec * 1000.0 + after.tv_usec / 1000.0;
	double ms_before = packet->sent_time.tv_sec * 1000.0 + packet->sent_time.tv_usec / 1000.0;
	double rtt = ms_after - ms_before;
	return rtt;
}

void receive_packet(t_data *data, t_packet *packet)
{
	char res_buf[IP_MAXPACKET];
	struct sockaddr_in src_addr;
	socklen_t addr_len = sizeof(src_addr);
	bool error = false;
	data->sleep = true;

	while (g_run)
	{
		ssize_t bytes = recvfrom(data->icmp_socket_fd, res_buf, sizeof(res_buf), 0,
								(struct sockaddr *)&src_addr, &addr_len);
		if (bytes < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			{
				data->sleep = false;
				return;
			}
			else
			{
				free_packet(packet);
				error_exit(data, "recvfrom", true);      
			}
		}

		struct iphdr *ip = (struct iphdr *)res_buf;
		struct icmphdr *icmp = (struct icmphdr *)(res_buf + ip->ihl * 4);
		ssize_t icmp_size = bytes - (ip->ihl * 4);
		
		if (icmp->type == ICMP_ECHOREPLY && ntohs(icmp->un.echo.id) == (getpid() & 0xFFFF))
		{
			double rtt = calculate_rtt(packet);
			if (data->stats.rtt_min > rtt)
				data->stats.rtt_min = rtt;
			if (data->stats.rtt_max < rtt)
				data->stats.rtt_max = rtt;
			data->stats.rtt_sum += rtt;
			data->stats.rtt_sum_sq += rtt * rtt;
			data->stats.n_received++;
			printf("%zd bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n",
				icmp_size,
				inet_ntoa(src_addr.sin_addr),
				ntohs(icmp->un.echo.sequence),
				ip->ttl,
				rtt);
			data->current_count++;
			break;
		}
		else if (icmp->type == ICMP_DEST_UNREACH && is_our_packet(res_buf))
		{
			switch (icmp->code)
			{
				case ICMP_NET_UNREACH:
					printf("%zd bytes from %s: Destination Net Unreachable\n",
						icmp_size, inet_ntoa(src_addr.sin_addr));
					break;
				case ICMP_HOST_UNREACH:
					printf("%zd bytes from %s: Destination Host Unreachable\n",
						icmp_size, inet_ntoa(src_addr.sin_addr));
					break;
				case ICMP_PROT_UNREACH:
					printf("%zd bytes from %s: Protocol Unreachable\n",
						icmp_size, inet_ntoa(src_addr.sin_addr));
					break;
				case ICMP_PORT_UNREACH:
					printf("%zd bytes from %s: Port Unreachable\n",
						icmp_size, inet_ntoa(src_addr.sin_addr));
					break;
				default:
					printf("%zd bytes from %s: Destination Unreachable (code=%d)\n",
						icmp_size, inet_ntoa(src_addr.sin_addr), icmp->code);
					break;
			}
			error = true;
			data->current_count++;
		}
		else if (icmp->type == ICMP_TIME_EXCEEDED && is_our_packet(res_buf))
		{
			switch (icmp->code)
			{
				case ICMP_EXC_TTL:
					printf("%zd bytes from %s: Time to live exceeded\n",
						icmp_size, inet_ntoa(src_addr.sin_addr));
					break;
				case ICMP_EXC_FRAGTIME:
					printf("%zd bytes from %s: Fragment reassembly time exceeded\n",
						icmp_size, inet_ntoa(src_addr.sin_addr));
					break;
				default:
					break;
			}
			error = true;
			data->current_count++;
		}

		if (data->flag_verbose && error)
			print_verbose(res_buf);
		if (error)
			break;
	}
}
