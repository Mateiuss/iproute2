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

static void explain(void)
{
	fprintf(stderr, "Usage: ... ptrim [ limit NUMBER ]\n");
}

static int ptrim_parse_opt(const struct qdisc_util *qu, int argc, char **argv,
			  struct nlmsghdr *n, const char *dev)
{
	int ok = 0;
	struct tc_fifo_qopt opt = {};

	while (argc > 0) {
		if (strcmp(*argv, "limit") == 0) {
			NEXT_ARG();
			if (get_size(&opt.limit, *argv)) {
				fprintf(stderr, "%s: Illegal value for \"limit\": \"%s\"\n", qu->id, *argv);
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
	struct tc_fifo_qopt *qopt;

	if (opt == NULL)
		return 0;

	if (RTA_PAYLOAD(opt)  < sizeof(*qopt))
		return -1;
	qopt = RTA_DATA(opt);
	if (strcmp(qu->id, "bfifo") == 0)
		print_size(PRINT_ANY, "limit", "limit %s", qopt->limit);
	else
		print_uint(PRINT_ANY, "limit", "limit %up", qopt->limit);
	return 0;
}

struct qdisc_util ptrim_qdisc_util = {
    .id = "ptrim",
    .parse_qopt = ptrim_parse_opt,
    .print_qopt = ptrim_print_opt,
};
