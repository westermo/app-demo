#include <ev.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SYSLOG_NAMES  /* Make prioritynames[] available from syslog.h */
#include "util.h"

int loglevel = LOG_ERR;
extern char *__progname;

/* Runtime data */


static int dhcp_boot_run(struct ev_loop *loop)
{
	char cmd[40];
	int err;

	/* Start Dnsmasq */
	snprintf(cmd, sizeof(cmd), "/bin/cfg-provider.sh &");
	err = system(cmd);
	if (err) {
		ERR("Unable to starts dnsmasq: %d\n", err);
		return err;
	}

	DBG("Starting event loop");
 	ev_run(loop, 0);
 	DBG("Exiting event loop");
 	return 0;
}

static void dump(void)
{
	LOG("Loglevel %d\n", loglevel);
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

static int config_change(const char *file_in, const char *file_out)
{
	char cfgup[120];
	int err;

	/* Change the hostname in input file*/
	snprintf(cfgup, sizeof(cfgup), "sed \'s/\"hostname\": \".*\"/\"hostname\": \"Host_name_from_app\"/\' %s > %s", file_in, file_out);
	err = system(cfgup);
	if (err) {
		ERR("Unable to change hostname: %d\n", err);
		return err;
	}
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
	const char *input_file = "/mnt/pre-config.cfg";
	const char *output_file = "/mnt/config.cfg";

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

	if (config_change(input_file, output_file))
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

	return dhcp_boot_run(loop);
}
