

# icmp-ping

A reimplementation of the `ping` utility from GNU Inetutils in C.

`icmp-ping` sends ICMP ECHO_REQUEST packets to network hosts and displays their responses.

## Options

```text
Usage: icmp-ping [OPTION...] HOST ...
Send ICMP ECHO_REQUEST packets to network hosts.

Options valid for all request types:

  -c, --count NUMBER         stop after sending NUMBER packets
      --ttl N                specify N as time-to-live
  -T, --tos NUM              set type of service (TOS) to NUM
  -v, --verbose              verbose output
  -w, --timeout N            stop after N seconds
  -W, --linger N             number of seconds to wait for response

Options valid for --echo requests:

  -?, --help                 give this help list
  -s, --size NUMBER          send NUMBER data octets
```

## Installation

Build the project:

```bash
make
```

Grant permission to create raw sockets:

```bash
sudo setcap cap_net_raw+ep ./icmp-ping
```

## Usage

```bash
icmp-ping [OPTION...] HOST
```

Examples:

```bash
icmp-ping 8.8.8.8
icmp-ping -c 4 8.8.8.8
icmp-ping --ttl 64 8.8.8.8
icmp-ping --size 50 8.8.8.8
```
