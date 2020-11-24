#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#include <readline/history.h>
#include <readline/readline.h>

#include <sys/socket.h>
#include <sys/types.h>

/* MACROS */

#define PROGRAM_NAME "rconc"
#define VERSION      "0.0.1"

#define CMD_MAX_LEN  1446
#define RECV_TIMEOUT 5000

#define PROMPT "\033[1m»\033[0m "

#define DEFAULT_HOST     "localhost"
#define DEFAULT_PORT     "25575"
#define DEFAULT_PASSWORD ""

/* DATA TYPES */

enum packet_type {
	PKTTYPE_RESPONSE = 0,
	PKTTYPE_AUTH_RESPONSE = 2,
	PKTTYPE_COMMAND = 2,
	PKTTYPE_AUTH = 3,
};

struct packet {
	int32_t length;
	int32_t request_id;
	enum packet_type type;
	char payload[CMD_MAX_LEN + 1]; /* must end in two NUL characters */
};

/* GLOBAL VARIABLES */
int sock;

/* FUNCTION DECLARATIONS */

void close_connection();
void exit_handler();
void init_connection(const char *host, const char *port);
struct packet *populate_packet(struct packet *pkt, enum packet_type type, const char *data);
int rcon_auth(const char *password);
int rcon_command(const char *command);
const struct packet *recv_packet(struct packet *pkt);
int send_packet(const struct packet *pkt);
void usage();

/* FUNCTION DEFINITIONS */

/* close connection */
void
close_connection()
{
	if (sock >= 0)
		close(sock);
}

/* close connection when exiting */
void
exit_handler()
{
	close_connection();
}

/* connect to remote host */
void
init_connection(const char *host, const char *port)
{
	struct addrinfo *result, *p = NULL;

	struct addrinfo hints = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM,
		.ai_flags = 0,
		.ai_protocol = 0,
	};

	int ret = getaddrinfo(host, port, &hints, &result);
	if (ret != 0) {
		fprintf(stderr, "\033[01;31merror\033[0m: getaddrinfo: %s\n", gai_strerror(ret));
		exit(EX_NOHOST);
	}

	/* loop through returned addresses */
	for (p = result; p != NULL; p = p->ai_next) {
		sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (sock < 0)
			continue;

		if (connect(sock, p->ai_addr, p->ai_addrlen) != -1)
			break;

		close(sock);
	}

	freeaddrinfo(result);

	if (p == NULL || sock <= 0) {
		perror("\033[01;31merror\033[0m: connection failed");
		exit(EX_NOHOST);
	}
}

/* fill a packet with the data provided */
struct packet *
populate_packet(struct packet *pkt, enum packet_type type, const char *data)
{
	int datalen = strlen(data);
	if (datalen > CMD_MAX_LEN) {
		fprintf(stderr, "\033[01;31merror\033[0m: command string too long (%d chars). Maximum allowed is %d chars.", datalen, CMD_MAX_LEN);
		return NULL;
	}
	pkt->type = type;

	/* create a random packet ID */
	pkt->request_id = abs(rand());

	strncpy(pkt->payload, data, CMD_MAX_LEN);

	pkt->length = sizeof(int32_t) * 2 + datalen + 2;

	return pkt;
}

int
rcon_auth(const char *password)
{
	struct packet pkt;
	populate_packet(&pkt, PKTTYPE_AUTH, password);

	if (send_packet(&pkt) == -1)
		return -1;

	if (recv_packet(&pkt) == NULL || pkt.type != PKTTYPE_AUTH_RESPONSE || pkt.request_id == -1)
		return -1;

	return 0;
}

int
rcon_command(const char *command)
{
	struct packet pkt;
	populate_packet(&pkt, PKTTYPE_COMMAND, command);

	if (send_packet(&pkt) == -1)
		return -1;

	return 0;
}

const struct packet *
recv_packet(struct packet *pkt)
{
	ssize_t ret, received;

	memset(pkt, 0, sizeof(*pkt));

	ret = recv(sock, &(pkt->length), sizeof(pkt->length), 0);
	if (ret == -1) {
		perror("\033[01;31merror\033[0m: socket error");
		return NULL;
	} else if (ret == 0) {
		fputs("\033[01;31merror\033[0m: no data recieved\n", stderr);
		return NULL;
	} else if ((size_t)ret < sizeof(pkt->length) || pkt->length < 10) {
		fputs("\033[01;31merror\033[0m: malformed packet\n", stderr);
		return NULL;
	}

	received = 0;
	while (received < (ssize_t)pkt->length) {
		ret = recv(sock, (char *)pkt + sizeof(pkt->length) + received, pkt->length - received, 0);
		if (ret < 0) {
			fputs("\033[01;31merror\033[0m: connection lost\n", stderr);
			exit(EX_UNAVAILABLE);
		}
		received += ret;
	}

	return pkt;
}

int
send_packet(const struct packet *pkt)
{
	int length = pkt->length + sizeof(int32_t);
	int sent = 0;
	int ret;

	while (sent < length) {
		ret = send(sock, pkt, length, 0);

		if (ret == -1) {
			perror("\033[01;31merror\033[0m: failed to send packet");
			return -1;
		}

		sent += ret;
	}

	return 0;
}

/* print usage information */
void
usage()
{
	fputs("usage: " PROGRAM_NAME " [-H host] [-p port] [-P password]\n", stderr);
}

int
main(int argc, char *argv[])
{
	char *host = NULL;
	char *port = NULL;
	char *password = NULL;

	/* `--help' option prints usage info */
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--help")) {
			usage();
			return 0;
		}
	}

	/* parse options */
	int optchar;
	while ((optchar = getopt(argc, argv, "hH:p:P:")) != -1) {
		switch (optchar) {
		case 'h':
			usage();
			return EX_USAGE;
		case 'H':
			host = optarg;
			break;
		case 'p':
			port = optarg;
			break;
		case 'P':
			password = optarg;
			break;
		default:
			usage();
			return EX_USAGE;
		}
	}

	if (host == NULL) {
		host = DEFAULT_HOST;
		fputs("\033[01;33mwarning\033[0m: no host specified. falling back to `localhost'.\n", stderr);
	}

	if (port == NULL) {
		port = DEFAULT_PORT;
		fputs("\033[01;33mwarning\033[0m: no port specified. falling back to `25575'.\n", stderr);
	}

	if (password == NULL) {
		password = DEFAULT_PASSWORD;
		if (password[0] == '\0')
			fputs("\033[01;33mwarning\033[0m: no password specified. falling back to a blank password.\n", stderr);
		else
			fprintf(stderr, "\033[01;33mwarning\033[0m: no password specified. falling back to `%s'.", password);
	}

	/* seed PRNG */
	srand(time(NULL));

	/* register exit handler function */
	if (atexit(&exit_handler) != 0) {
		perror("\033[01;31merror\033[0m: failed to set exit handler");
		return EX_OSERR;
	}

	init_connection(host, port);

	/* authenticate with server */
	if (rcon_auth(password) != 0) {
		fputs("\033[01;31merror\033[0m: login failed\n", stderr);
		return EX_UNAVAILABLE;
	}

	/* main program loop */
	char *input;
	struct packet pkt;
	while ((input = readline(PROMPT)) != NULL) {
		/* ignore blank lines */
		if (input[0] == '\0')
			continue;

		if (!strcmp(input, "quit") || !strcmp(input, "exit"))
			break;

		/* send command to server */
		populate_packet(&pkt, PKTTYPE_COMMAND, input);
		send_packet(&pkt);

		/* print response if any was sent */
		if (recv_packet(&pkt) != NULL && pkt.type == PKTTYPE_RESPONSE && pkt.length > 0)
			puts(pkt.payload);

		if (!strcmp(input, "stop"))
			break;
	}

	return 0;
}
