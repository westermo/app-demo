#include <stdio.h>
#include <errno.h>
#include <ev.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

extern char *__progname;

int toggle;  /* toggle led ON/OFF */

struct led {
	char port[64];
	int max_brightness;
} led;

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

/* read max brightness from driver and save it to struct */
static int get_max_brightness(const char *path, struct led *l)
{
	FILE *fp = NULL;
	char max_val[256];
	int retval = 1;

	if (access(path, F_OK) != 0) {
		printf("Could not access file %s. %s\n", path, strerror(errno));
		goto cleanup;
	}

	fp = fopen(path, "r");
	if (!fp) {
		printf("Could not open file %s. %s\n", path, strerror(errno));
		goto cleanup;
	}

	if ((fgets(max_val, sizeof(max_val), fp)) == NULL) {
		printf("Could not read max_brightness value. %s\n", strerror(errno));
		goto cleanup;
	}

	l->max_brightness = atoi(max_val);
	retval = 0;
cleanup:
	if (fp != NULL)
		fclose(fp);

	return retval;
}
/* write value to led driver if we have access to file */
static int set_led(const char *path, int value)
{
	FILE *fp = NULL;
	int retval = 1;

	if (access(path, F_OK) != 0) {
		printf("Could not access file %s. %s\n", path, strerror(errno));
		goto cleanup;
	}

	fp = fopen(path, "w");
	if (!fp) {
		printf("Could not open file %s. %s\n", path, strerror(errno));
		goto cleanup;
	}

	fprintf(fp, "%d\n", value);
	retval = 0;
	/* (value == 0) ? printf("OFF\n") : printf("ON\n"); */
cleanup:
	if (fp != NULL)
		fclose(fp);

	return retval;
}

/* callback for all signals */
static void sig_cb(struct ev_loop *loop, ev_signal *w, int revents)
{

	printf("\nsignal received\n");

	(void)(revents);

	switch (w->signum) {
	default:
		ev_break(loop, EVBREAK_ALL);
		printf("breaking loop\n");
		break;
	}
}

/* callback for ev_timer event */
static void timeout_cb(EV_P_ ev_timer *w, int revents)
{
	char led_path[256];

	snprintf(led_path, sizeof(led_path), "/sys/class/leds/%s/brightness", led.port);
	if (set_led(led_path, toggle * led.max_brightness) != 0) {
		ev_break(EV_A_ EVBREAK_ALL);
		return;
	}
	toggle = !toggle;

	/* restart timer */
	ev_timer_again(EV_A_ w);
}

static int starts_with(const char *pre, const char *str)
{
	int lenpre = strlen(pre);
	int lenstr = strlen(str);

	return lenstr < lenpre ? -1 : memcmp(pre, str, lenpre);
}

static void print_interfaces(void)
{
	DIR *d;
	struct dirent *dir;

	if (access("/sys/class/leds/", F_OK) != 0) {
		printf("Could not access /sys/class/leds/. %s\n", strerror(errno));
		return;
	}
	d = opendir("/sys/class/leds/");
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if ((dir->d_type == DT_LNK) && (starts_with("eth", dir->d_name) == 0))
				printf("%s\n", dir->d_name);
		}
		closedir(d);
	}
}

static int usage(int code)
{
	fprintf(stderr, "\nUsage: %s [OPTIONS]\n\n"
		"  -h         Show summary of command line options and exit\n"
		"  -p         Print interfaces\n"
		"  -i         Start flashing interface led every -t sec\n"
		"  -t         Flash frequency in seconds, default is 2.0 sec\n"
		"  -1         Set led ON\n"
		"  -0         Set led OFF\n"
		"\n"
		"Example usage:\n"
		"Flash led:   %s -i ethX5:yellow:state -t 2.0\n"
		"Set led ON:  %s -1 ethX5:yellow:state\n"
		"\n", __progname, __progname, __progname);

	return code;
}

int main(int argc, char **argv)
{
	struct ev_loop *loop;
	struct sig *sig;
	ev_timer timeout_watcher;
	int c;
	char led_path[256];
	double timeout_repeat = 2.;

	while ((c = getopt(argc, argv, "hpi:t:0:1:")) != EOF) {
		switch (c) {
		case 'h':
			return usage(0);
		case 'i':
			/* initialise led structure */
			snprintf(led.port, sizeof(led.port), "%s", optarg);
			snprintf(led_path, sizeof(led_path), "/sys/class/leds/%s/max_brightness", led.port);
			if (get_max_brightness(led_path, &led) != 0)
				return usage(1);
			break;
		case 'p':
			print_interfaces();
			return 0;
		case 't':
			errno = 0;
			timeout_repeat = strtof(optarg, NULL);
			if (errno != 0)
				return usage(1);
			break;
		case '0':
			snprintf(led.port, sizeof(led.port), "%s", optarg);
			snprintf(led_path, sizeof(led_path), "/sys/class/leds/%s/brightness", led.port);
			if (set_led(led_path, 0) != 0)
				return usage(1);
			return 0;
		case '1':
			snprintf(led.port, sizeof(led.port), "%s", optarg);
			snprintf(led_path, sizeof(led_path), "/sys/class/leds/%s/max_brightness", led.port);
			if (get_max_brightness(led_path, &led) != 0)
				return usage(1);
			snprintf(led_path, sizeof(led_path), "/sys/class/leds/%s/brightness", led.port);
			if (set_led(led_path, led.max_brightness) != 0)
				return usage(1);
			return 0;
		default:
			return usage(1);
		}
	}

	toggle = 0;
	loop = EV_DEFAULT;

	/* initialise signal watchers and start them */
	for (sig = sigs; sig->signo; sig++) {
		ev_signal_init(&sig->watcher, sig_cb, sig->signo);
		ev_signal_start(loop, &sig->watcher);
	}

	/* initialise timer watcher (start after 0 sec, repeat every -t sec) */
	ev_timer_init(&timeout_watcher, timeout_cb, 0., timeout_repeat);
	ev_timer_start(loop, &timeout_watcher);

	return ev_run(loop, 0);
}
