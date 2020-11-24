#include <errno.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PROGRAM_NAME "rconc"
#define VERSION      "0.0.1"

enum packet_type {
	PKT_RESPONSE = 0,
	PKT_AUTH_RESPONSE = 2,
	PKT_COMMAND = 2,
	PKT_AUTH = 3,
};

struct packet {
	int32_t length;
	int32_t request_id;
	enum packet_type type;
	char *payload; /* null-terminated command string */
	char terminator; /* should always be 0 */
};

void usage();

void
usage()
{
	fputs("usage: " PROGRAM_NAME " -H host -p port -P password\n", stderr);
}

int main(int argc, char *argv[])
{
	char *host = "localhost";
	uint16_t port = 25575;
	char *password = "";

	if (argc >= 2 && !strcmp(argv[1], "--help")) {
		usage();
		return 0;
	}

	int optchar;
	while ((optchar = getopt(argc, argv, "H:p:P:")) != -1) {
		switch (optchar) {
		case 'H':
			host = optarg;
			break;
		case 'p':
			port = (uint16_t)strtoul(optarg, NULL, 10);
			break;
		case 'P':
			password = optarg;
			break;
		default:
			usage();
			return 1;
		}
	}


	return 0;
}
