#include "ping.h"

static void ping_help(t_data *data)
{
	printf("Usage: icmp-ping [OPTION...] HOST ...\n"
		"Send ICMP ECHO_REQUEST packets to network hosts.\n\n"
		" Options valid for all request types:\n\n"
		"  -c, --count=NUMBER         stop after sending NUMBER packets\n"
		"      --ttl=N                specify N as time-to-live\n"
		"  -T, --tos=NUM              set type of service (TOS) to NUM\n"
		"  -v, --verbose              verbose output\n"
		"  -w, --timeout=N            stop after N seconds\n"
		"  -W, --linger=N             number of seconds to wait for response\n\n"
		" Options valid for --echo requests:\n\n"
		"  -?, --help                 give this help list\n"
		"  -s, --size=NUMBER          send NUMBER data octets\n\n");
	success_exit(data);
}

static long parse_helper(t_data *data, char **argv, int argc, bool *flag, int *i)
{
	*flag = true;
	if (*i+1 == argc)
	{
		char buf[128];
		snprintf(buf, sizeof(buf), "option requires an argument -- '%s'\n%s", argv[*i], HELP_MSG);
		error_exit(data, buf, false);
	}
	(*i)++;
	char *endptr;
	long val = strtol(argv[*i], &endptr, 10);
	if (*endptr != '\0')
	{
		char buf[128];
		snprintf(buf, sizeof(buf), "invalid value (`%s' near `%s')", argv[*i], endptr);
		error_exit(data, buf, false);
	}
	return val;
}

static void error_option_value(t_data *data, long val, char *msg)
{
	char buf[128];
	snprintf(buf, sizeof(buf), "%s%ld", msg, val);
	error_exit(data, buf, false);
}

void parse(t_data *data, int argc, char **argv)
{
	memset(data, 0, sizeof(t_data));
	data->stats.rtt_min = DBL_MAX;
	data->target_addr.sin_family = AF_INET;
	data->target_addr.sin_port = 0;

	bool url_found = false;
	for(int i = 1; i < argc; ++i)
	{
		if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose"))
			data->flag_verbose = true;
		else if (!strcmp(argv[i], "-?") || !strcmp(argv[i], "--help") )
			ping_help(data);
		else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--size"))
		{
			long val = parse_helper(data, argv, argc, &data->flag_size, &i);
			if (val < 0 || val >= 65400)
				error_option_value(data, val, "option value too big: ");
			data->payload_size = (int)val;
		}
		else if (!strcmp(argv[i], "-W") || !strcmp(argv[i], "--linger"))
		{
			long val = parse_helper(data, argv, argc, &data->flag_linger, &i);
			if (val < 0 || val > INT32_MAX)
				error_option_value(data, val, "option value too big: ");
			else if (val == 0)
				error_option_value(data, val, "option value too small: ");
			data->linger = (int)val;
		}
		else if (!strcmp(argv[i], "-w") || !strcmp(argv[i], "--timeout"))
		{
			long val = parse_helper(data, argv, argc, &data->flag_timeout, &i);
			if (val < 0 || val > INT32_MAX)
				error_option_value(data, val, "option value too big: ");
			else if (val == 0)
				error_option_value(data, val, "option value too small: ");
			data->timeout = (int)val;
		}
		else if (!strcmp(argv[i], "-c") || !strcmp(argv[i], "--count"))
		{
			long val = parse_helper(data, argv, argc, &data->flag_count, &i);
			if (val <= 0 || val > INT32_MAX)
				data->flag_count = false;
			else
				data->max_count = (int)val;
		}
		else if (!strcmp(argv[i], "-T") || !strcmp(argv[i], "--tos"))
		{
			long val = parse_helper(data, argv, argc, &data->flag_tos, &i);
			if (val < 0 || val > 255)
				error_option_value(data, val, "option value too big: ");
			data->tos = (u_int8_t)val;
		}
		else if (!strcmp(argv[i], "--ttl"))
		{
			long val = parse_helper(data, argv, argc, &data->flag_ttl, &i);
			if (val < 0 || val > 255)
				error_option_value(data, val, "option value too big: ");
			else if (val == 0)
				error_option_value(data, val, "option value too small: ");
			data->ttl = (u_int8_t)val;
		}
		else if (argv[i][0] == '-' && strlen(argv[i]) > 1)
		{
			char buf[128];
			snprintf(buf, sizeof(buf), "invalid option -- '%c'\n%s", argv[i][1], HELP_MSG);
			error_exit(data, buf, false);
		}
		else if (!url_found)
		{
			data->target_url = strdup(argv[i]);
			if (!data->target_url)
				error_exit(data, "malloc", true);
			url_found = true;
		}
	}
	if (!url_found)
		error_exit(data, "missing host operand\n" HELP_MSG, false);
	if (!data->flag_size)
		data->payload_size = DEFAULT_ICMP_PAYLOAD_SIZE;
	if (!data->flag_linger)
		data->linger = DEFAULT_LINGER_TIME;
	if (!data->flag_tos)
		data->tos = DEFAULT_TOS;
	if (!data->flag_ttl)
		data->ttl = DEFAULT_TTL;
}
