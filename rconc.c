#include <stdio.h>

#define PROGRAM_NAME "rconc"
#define VERSION      "0.0.1"

enum packet_type {
	PKT_RESPONSE      = 0,
	PKT_AUTH_RESPONSE = 2,
	PKT_COMMAND       = 2,
	PKT_AUTH          = 3,
};

struct packet {
	int length;
	int request_id;
	enum packet_type type;
	char *payload;
};

int
main(int argc, char *argv[])
{
	return 0;
}
