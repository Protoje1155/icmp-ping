#pragma once
#include <stdio.h>
#include <float.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <string.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <signal.h>
#include <netdb.h>
#include <math.h>
#include <errno.h>

#define HELP_MSG "Try 'icmp-ping -?' or 'icmp-ping --help' for more information."

#define DEFAULT_ICMP_PAYLOAD_SIZE 56
#define DEFAULT_LINGER_TIME 1
#define DEFAULT_TOS 0
#define DEFAULT_TTL 64

typedef struct s_packet
{
    char *packet;
    struct timeval sent_time; 
} t_packet;

typedef struct s_stats
{
    int n_transmitted;
    int n_received;
    double rtt_min;
    double rtt_max;
    double rtt_sum;
    double rtt_sum_sq;
} t_stats;

typedef struct s_data
{
    int icmp_socket_fd;
    char *target_url;
    char *target_ip;
    struct sockaddr_in target_addr;
    bool sleep;

    bool flag_verbose;
    bool flag_size;
    bool flag_linger;
    bool flag_timeout;
    bool flag_count;
    bool flag_tos;
    bool flag_ttl;

    int payload_size;
    int linger;
    int timeout;
    int max_count;
    int current_count;
    int tos;
    int ttl;

    t_stats stats;
} t_data;

// args
void parse(t_data *data, int argc, char **argv);

// network
void ping(t_data *data);

// packet
t_packet *prep_icmp_packet(t_data *data);
void free_packet(t_packet *p);

// receive
void receive_packet(t_data *data, t_packet *packet);

// signal
extern int g_run;
void setup_signal(t_data *data);

//socket
void init_socket(t_data *data);

// utils
void error_exit(t_data *data, char *msg, bool use_perror);
void success_exit(t_data *data);