#include <ev.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <jansson.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SYSLOG_NAMES  /* Make prioritynames[] available from syslog.h */
#include "lag.h"
#include "util.h"

int loglevel = LOG_NOTICE;
extern char *__progname;

/* Configurable parameters */
uint16_t vid = 2;
struct lag_config lag_confs[2] = {
	{ .name = "west" },
	{ .name = "east" }
};

/* Runtime data */
static struct lag *lags[2] = { NULL, NULL };
struct backbone_id current_id;

static int set_station(void)
{
	struct ifaddrs *ifaddrs, *ifa;
	struct sockaddr_ll *sll;
	int ret = 1;

	if (getifaddrs(&ifaddrs))
		return 1;

	for (ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_PACKET)
			continue;

		/* Use the backbone bridge's MAC to identify this
		 * node. */
		if (strcmp(ifa->ifa_name, "backbone"))
			continue;

		sll = (struct sockaddr_ll *)ifa->ifa_addr;
		memcpy(current_id.station, sll->sll_addr, ETH_ALEN);
		ret = 0;
		break;
	}

	freeifaddrs(ifaddrs);
	return ret;
}

static bool set_id(const struct backbone_id *new_id)
{
	uint8_t *sta = current_id.station, *head = current_id.head;
	char netchid[0x40];
	int err;

	if (!memcmp(&current_id, new_id, sizeof(current_id)))
		return false;

	memcpy(&current_id, new_id, sizeof(current_id));

	LOG("New ID: %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x/"
	    "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x+%u",
	    sta[0], sta[1], sta[2], sta[3], sta[4], sta[5],
	    head[0], head[1], head[2], head[3], head[4], head[5],
	    current_id.id);

	/* Shell out to a script that does all the heavy lifting of
	 * setting up the relevant addresses, routes and tc rules. */
	snprintf(netchid, sizeof(netchid), "/usr/lib/backbone/net-chid.sh %u",
		 current_id.id);

	err = system(netchid);
	if (err) {
		ERR("Unable to update ID: %d\n", err);
		return false;
	}

	return true;
}

static int backbone_cb_id(struct backbone_id *id)
{
	*id = current_id;
	return 0;
}

static bool backbone_cb_update(void)
{
	struct backbone_id new_id, *id;
	int d, p;

	memset(&new_id, 0, sizeof new_id);
	memcpy(new_id.station, current_id.station, ETH_ALEN);

	/* First pass, iterate over all of our neighbors, find the
	 * best available head and attach to it. */
	for (d = 0; d < 2; d++) {
		for (p = 0; p < 2; p++) {
			id = &lags[d]->port[p]->neigh;

			/* If the sender was this station, we're the
			 * head. If our neighbor think we're the head,
			 * don't use that neighbor in the ID election. */
			if (!memcmp(id->station, current_id.station, ETH_ALEN))
				memcpy(id->head, current_id.station, ETH_ALEN);
			else if (!memcmp(id->head, current_id.station, ETH_ALEN))
				continue;

			/* Ignore all stations that are further from
			 * our current head than we are. */
			if (!memcmp(id->head, current_id.head, ETH_ALEN) &&
			    (id->id > current_id.id))
				continue;

			if (backbone_id_cmp(id, &new_id) > 0) {
				memcpy(&new_id.head, id->head, ETH_ALEN);

				/* If we are the head, use ID 1.
				 * Otherwise attach behind our
				 * neighbor. */
				if (!memcmp(id->head, current_id.station, ETH_ALEN))
					new_id.id = 1;
				else
					new_id.id = id->id + 1;
			}
		}
	}

	/* OK, now we know our ID, now do a second pass to figure out
	 * which ports are peered. */
	for (d = 0; d < 2; d++) {
		for (p = 0; p < 2; p++) {
			id = &lags[d]->port[p]->neigh;

			/* Never open a looped port, or a port which is
			 * attached to a different head. */
			if (!memcmp(id->station, current_id.station, ETH_ALEN) ||
			    memcmp(id->head, new_id.head, ETH_ALEN)) {
				lag_port_enable(lags[d]->port[p], false);
				continue;
			}

			switch (id->id - new_id.id) {
			case  1:
			case -1:
				/* Distance is 1, port has peered. */
				lag_port_enable(lags[d]->port[p], true);
				break;
			default:
				lag_port_enable(lags[d]->port[p], false);
			}
		}
	}

	return set_id(&new_id);
}

static int backbone_run(struct ev_loop *loop)
{
	char netup[0x40];
	int err, i;

	/* Create our two link aggregates */
	for (i = 0; i < 2; i++) {
		lag_confs[i].loop = loop;
		lag_confs[i].backbone_id = backbone_cb_id;
		lag_confs[i].update = backbone_cb_update;

		err = lag_new(&lag_confs[i], &lags[i]);
		if (err) {
			ERR("Unable to create lag %s: %d\n", lag_confs[i].name, err);
			return err;
		}
	}

	/* Now that the LAGs are created in the kernel, we can finish
	 * setting up the backbone. */
	snprintf(netup, sizeof(netup), "/usr/lib/backbone/net-up.sh %u", vid);
	err = system(netup);
	if (err) {
		ERR("Unable to create backbone bridge: %d\n", err);
		return err;
	}

	if (set_station()) {
		ERR("Unable to set station ID");
		return 1;
	}

	DBG("Starting event loop");
 	ev_run(loop, 0);
 	DBG("Exiting event loop");
 	return 0;
}

static void dump(void)
{
	uint8_t *sta, *head;
	int i, p;

	sta = current_id.station;
	head = current_id.head;

	LOG("ID: %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x/"
	    "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x+%u",
	    sta[0], sta[1], sta[2], sta[3], sta[4], sta[5],
	    head[0], head[1], head[2], head[3], head[4], head[5],
	    current_id.id);

	for (i = 0; i < 2; i++) {
		if (!lags[i])
			continue;

		LOG("  LAG %s:", lags[i]->conf->name);
		for (p = 0; p < 2; p++) {
			if (!lags[i]->port[p])
				continue;

			LOG("    Port %s: Link:%s Enabled:%s Neigh:",
			    lags[i]->conf->port[p],
			    lag_port_has_link(lags[i]->port[p]) ? "Up" : "DOWN",
			    lag_port_is_enabled(lags[i]->port[p]) ? "Yes" : "NO");

			sta  = lags[i]->port[p]->neigh.station;
			head = lags[i]->port[p]->neigh.head;
			LOG("      %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x/"
			    "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x+%u",
			    sta[0], sta[1], sta[2], sta[3], sta[4], sta[5],
			    head[0], head[1], head[2], head[3], head[4], head[5],
			    lags[i]->port[p]->neigh.id);
		}
	}
}

static int loglvl(char *level)
{
	int i;

	for (i = 0; prioritynames[i].c_name; i++) {
		if (!strncasecmp(prioritynames[i].c_name, level, strlen(level)))
			return prioritynames[i].c_val;
	}

	return atoi(level);
}

static void sig_cb(struct ev_loop *loop, ev_signal *w, int revents)
{
	(void)(revents);

	switch (w->signum) {
	case SIGUSR1:
		if (loglevel == LOG_NOTICE)
			loglevel = LOG_DEBUG;
		else
			loglevel = LOG_NOTICE;
		setlogmask(LOG_UPTO(loglevel));

		LOG("Switching to log level %s",
		    (loglevel == LOG_NOTICE) ? "notice" : "debug");
                break;

	case SIGUSR2:
		dump();
		break;

	default:
		ev_break(loop, EVBREAK_ALL);
		break;
	}
}

struct sig {
	int signo;
	ev_signal watcher;
};

static struct sig sigs[] = {
	{ .signo = SIGINT },
	{ .signo = SIGHUP },
	{ .signo = SIGTERM },
	{ .signo = SIGQUIT },
	{ .signo = SIGUSR1 },
	{ .signo = SIGUSR2 },

	{ .signo = 0 }
};

static int usage(int code)
{
	fprintf(stderr, "\nUsage: %s [OPTIONS]\n\n"
		"  -h         Show summary of command line options and exit\n"
		"  -l level   Set log level: none, err, info, notice*, debug\n"
		"  -n         Run in foreground, do not detach from controlling terminal\n"
		"  -s         Use syslog, even if running in foreground, default w/o -n\n"
		"  -v         Show program version\n"
		"\n"
		"Use SIGUSR1 to toggle debug log level at runtime.\n"
		"\n", __progname);

	return code;
}

static int version(void)
{
	printf("%s\n", "DEVEL");
	return 0;
}

static int config_parse(char *file)
{
	json_error_t jerr;
	json_t *cfg;
	int v = -1;

	cfg = json_load_file(file, 0, &jerr);
	if (!cfg) {
		ERR("Unable to parse configuration file %s: %s",
		    file, jerr.text);
		return 1;
	}

	if (json_unpack_ex(cfg, &jerr, 0, "{ s:i s:[ss!] s:[ss!] }",
			   "vid", &v,
			   "west", &lag_confs[0].port[0], &lag_confs[0].port[1],
			   "east", &lag_confs[1].port[0], &lag_confs[1].port[1])) {
		ERR("Invalid configuration file %s: %s",
		    file, jerr.text);
		return 1;
	}

	if (v < 1 || v > 4094) {
		ERR("VLAN ID out of range [1-4094], was %d\n", v);
		return 1;
	}

	vid = v;

	/* TODO: Don't leak memory */
	return 0;
}

int main(int argc, char **argv)
{
	struct ev_loop *loop;
	struct sig *sig;
	int background = 1;
	int logcons    = 0;
	int log_opts   = LOG_NDELAY | LOG_PID;
	int c;

	memset(&current_id, 0, sizeof current_id);

	while ((c = getopt(argc, argv, "hl:nsv")) != EOF) {
		switch (c) {
		case 'h':
			return usage(0);
		case 'l':
			loglevel = loglvl(optarg);
			break;
		case 'n':
			background = 0;
			logcons++;
			break;
		case 's':
			logcons--;
			break;
		case 'v':
			return version();
		default:
			return usage(1);
		}
	}

	if (config_parse("/etc/backbone.json"))
		return 1;

	if (background)
		daemon(0, 0);
	if (logcons > 0)
		log_opts |= LOG_PERROR;

	openlog(__progname, log_opts, LOG_DAEMON);
	setlogmask(LOG_UPTO(loglevel));

	loop = ev_default_loop(0);

	for (sig = sigs; sig->signo; sig++) {
		ev_signal_init(&sig->watcher, sig_cb, sig->signo);
		ev_signal_start(loop, &sig->watcher);
	}

	return backbone_run(loop);
}
