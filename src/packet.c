#include "ping.h"

static void fill_payload(t_data *data, char *p)
{
	char *off =  p + sizeof(struct icmphdr);
	for (int i = 0; i < data->payload_size; i++)
		*off++ = (unsigned int)i;
}

// RFC-1071 Internet checksum algorithm
static uint16_t checksum(void *buf, int count)
{
	uint16_t *data = buf;
	uint32_t sum = 0;

	while (count > 1)
	{
		sum += *data++;
		count -= 2;
	}
	if (count == 1)
		sum += *(uint8_t *)data;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	return (uint16_t)(~sum);
}

// 8 bytes of header, 56 bytes of payload (0-55 'dec' filled)
t_packet *prep_icmp_packet(t_data *data)
{
	static int counter = 0;
	int icmp_size = 8 + data->payload_size;
	char *packet = (char *)malloc(sizeof(char) * icmp_size);
	if (!packet)
		error_exit(data, "malloc", true);
	memset(packet, 0, icmp_size);
	struct icmphdr *icmp = (struct icmphdr *)packet;
	icmp->type = ICMP_ECHO;
	icmp->code = 0;
	icmp->un.echo.id = htons(getpid() & 0xFFFF);
	icmp->un.echo.sequence = htons(counter++);
	fill_payload(data, packet);
	icmp->checksum = checksum(packet, icmp_size);

	t_packet *p = malloc(sizeof(t_packet));
	if (!p)
	{
		free(packet);
		error_exit(data, "malloc", true);
	}
	p->packet = packet;
	return p;
}

void free_packet(t_packet *p)
{
	free(p->packet);
	free(p);
}
