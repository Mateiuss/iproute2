#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "utils.h"
#include "tc_util.h"

struct tc_ptrim_qopt {
	/* parameters */
	uint8_t limit;
	uint8_t rr_max_count;

	/* statistics */
	uint64_t trimmed_packets;
};

static int _get_size(uint8_t *size, const char *str)
{
	char *endptr;
	long val = strtol(str, &endptr, 10);

	if (*endptr != '\0' || val < 0 || val > UINT8_MAX)
		return -1;

	*size = (uint8_t)val;

	return 0;
}

static void explain(void)
{
	fprintf(stderr, "Usage: ... ptrim [ limit NUMBER ] [ rr_max_count NUMBER ]\n");
}

static int ptrim_parse_opt(const struct qdisc_util *qu, int argc, char **argv,
			  struct nlmsghdr *n, const char *dev)
{
	int ok = 0;
	struct tc_ptrim_qopt opt = {};

	while (argc > 0) {
		if (strcmp(*argv, "limit") == 0) {
			NEXT_ARG();
			if (_get_size(&opt.limit, *argv)) {
				fprintf(stderr, "%s: Illegal value for \"limit\": \"%s\"\n", qu->id, *argv);
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "rr_max_count") == 0) { 
			NEXT_ARG();
			if (_get_size(&opt.rr_max_count, *argv)) {
				fprintf(stderr, "%s: Illegal value for \"rr_max_count\": \"%s\"\n", qu->id, *argv);
				return -1;
			}
			ok++;
		} else if (strcmp(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			fprintf(stderr, "%s: unknown parameter \"%s\"\n", qu->id, *argv);
			explain();
			return -1;
		}
		argc--; argv++;
	}

	if (ok)
		addattr_l(n, 1024, TCA_OPTIONS, &opt, sizeof(opt));
	return 0;
}

static int ptrim_print_opt(const struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	struct tc_ptrim_qopt *qopt;

	if (opt == NULL)
		return 0;

	if (RTA_PAYLOAD(opt)  < sizeof(*qopt))
		return -1;

	qopt = RTA_DATA(opt);
	print_uint(PRINT_ANY, "limit", "limit %u%%", qopt->limit);
	print_u64(PRINT_ANY, "trimmed_packets", " trimmed %llu",
			qopt->trimmed_packets);
	print_uint(PRINT_ANY, "rr_max_count", " rr_max_count %u", qopt->rr_max_count);
	return 0;
}

struct qdisc_util ptrim_qdisc_util = {
	.id = "ptrim",
	.parse_qopt = ptrim_parse_opt,
	.print_qopt = ptrim_print_opt,
};
