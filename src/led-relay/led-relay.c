#include <unistd.h>
#include <stdio.h>    /* puts */
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define LED_PATH "/sys/class/leds/"
#define RELAY_PATH "/sys/class/relay-ctrl/bypass-relay/"
#define VERSION "1.0.0.0"

static int bypass_relay(char *val)
{
	char relay_path[512];
	FILE *fp = NULL;
	int retval = 1;

	snprintf(relay_path, sizeof(relay_path), RELAY_PATH "value");

	if (access(relay_path, F_OK) != 0) {
<<<<<<< HEAD
		fprintf(stderr, "Could not access file %s. E:%s\n",
=======
		printf("Could not access file %s. %s\n",
>>>>>>> daf014f8e535af3acc18d02d8d1b76baafa6cdb4
			relay_path,
			strerror(errno));
		goto cleanup;
	}

	fp = fopen(relay_path, "w");
	if (!fp) {
<<<<<<< HEAD
		fprintf(stderr, "Could not open file %s. E:%s\n",
=======
		printf("Could not open file %s. %s\n",
>>>>>>> daf014f8e535af3acc18d02d8d1b76baafa6cdb4
			relay_path,
			strerror(errno));
		goto cleanup;
	}

	fprintf(fp, "%s\n", val);
	retval = 0;

cleanup:
	if (fp != NULL)
		fclose(fp);

	return retval;
}

static int flash_led(char *val, char *port)
{
	char led_path[512];
	FILE *fp = NULL;
	int retval = 1;

	snprintf(led_path, sizeof(led_path), LED_PATH "%s/trigger", port);

	if (access(led_path, F_OK) != 0) {
<<<<<<< HEAD
		fprintf(stderr, "Could not access file %s. E:%s\n",
=======
		printf("Could not access file %s. %s\n",
>>>>>>> daf014f8e535af3acc18d02d8d1b76baafa6cdb4
			led_path,
			strerror(errno));
		goto cleanup;
	}

	fp = fopen(led_path, "w");
	if (!fp) {
<<<<<<< HEAD
		fprintf(stderr, "Could not open file %s. E:%s\n",
=======
		printf("Could not open file %s. %s\n",
>>>>>>> daf014f8e535af3acc18d02d8d1b76baafa6cdb4
			led_path,
			strerror(errno));
		goto cleanup;
	}

	fprintf(fp, "%s", val);
	retval = 0;

cleanup:
	if (fp != NULL)
		fclose(fp);

	return retval;
}

static int set_led(int val, char *port)
{
	char led_path[512];
	FILE *fp = NULL;
	int retval = 1;

	snprintf(led_path, sizeof(led_path), LED_PATH "%s/brightness", port);

	if (access(led_path, F_OK) != 0) {
<<<<<<< HEAD
		fprintf(stderr, "Could not access file %s. E:%s\n",
=======
		printf("Could not access file %s. %s\n",
>>>>>>> daf014f8e535af3acc18d02d8d1b76baafa6cdb4
			led_path,
			strerror(errno));
		goto cleanup;
	}

	fp = fopen(led_path, "w");
	if (!fp) {
<<<<<<< HEAD
		fprintf(stderr, "Could not open file %s. E:%s\n",
=======
		printf("Could not open file %s. %s\n",
>>>>>>> daf014f8e535af3acc18d02d8d1b76baafa6cdb4
			led_path,
			strerror(errno));
		goto cleanup;
	}

	fprintf(fp, "%d\n", val);
	retval = 0;

cleanup:
	if (fp != NULL)
		fclose(fp);

	return retval;
}

static int get_led(int *val, char *port)
{
	char led_value[256];
	char led_path[512];
	FILE *fp = NULL;
	int retval = 1;

	snprintf(led_path, sizeof(led_path), LED_PATH "%s/brightness", port);

	if (access(led_path, F_OK) != 0) {
<<<<<<< HEAD
		fprintf(stderr, "Could not access file %s. E:%s\n",
=======
		printf("Could not access file %s. %s\n",
>>>>>>> daf014f8e535af3acc18d02d8d1b76baafa6cdb4
			led_path,
			strerror(errno));
		goto cleanup;
	}

	fp = fopen(led_path, "r");
	if (!fp) {
<<<<<<< HEAD
		fprintf(stderr, "Could not open file %s. E:%s\n",
=======
		printf("Could not open file %s. %s\n",
>>>>>>> daf014f8e535af3acc18d02d8d1b76baafa6cdb4
			led_path,
			strerror(errno));
		goto cleanup;
	}

	if ((fgets(led_value, sizeof(led_value), fp)) == NULL) {
<<<<<<< HEAD
		fprintf(stderr, "Could not read led value. E:%s\n", strerror(errno));
=======
		printf("Could not read led value. %s\n", strerror(errno));
>>>>>>> daf014f8e535af3acc18d02d8d1b76baafa6cdb4
		goto cleanup;
	}

	*val = atoi(led_value);
	retval = 0;

cleanup:
	if (fp != NULL)
		fclose(fp);

	return retval;
}

static int usage(int ret, char *prog)
{
	fprintf(stderr, "\nUsage: %s [OPTIONS]\n\n"
		"  -b         Disable(2) or Enable(1) bypass relays\n"
		"  -c         Clear LED or relay value to 0\n"
		"  -f         Start flashing LED\n"
		"  -g         Get LED or relay value\n"
		"  -h         Show summary of command line options and exit\n"
		"  -s         Set LED or relay on\n"
		"  -v         Show program version\n"
		"\n"
		"Example usage:\n"
		"Flash led:   %s -f ethX5:yellow:state\n"
		"Set led ON:  %s -s ethX6:yellow:state\n"
		"Clear led:   %s -c ethX5:yellow:state\n"
		"Disable bypass: %s -b 2\n"
		"Enable bypass:  %s -b 1\n"
		"\n", prog, prog, prog, prog, prog, prog);
	return ret;
}

int main(int argc, char *argv[])
{
	char *prog, *ptr;
	int c, val;

	prog = argv[0];
	ptr = strrchr(prog, '/');
	if (ptr)
		prog = ptr+1;

	while ((c = getopt(argc, argv, "b:c:f:g:hs:v")) != EOF) {
		switch (c) {
		case 'b':
<<<<<<< HEAD
			if (!bypass_relay(optarg)) {
=======
			if (bypass_relay(optarg)) {
>>>>>>> daf014f8e535af3acc18d02d8d1b76baafa6cdb4
				fprintf(stderr, "Bypass relay is set to %s\n",
					optarg);
			} else {
				fprintf(stderr, "Failed setting bypass relay\n");
				return 1;
			}
			return 0;
		case 'c':
<<<<<<< HEAD
			if (!set_led(0, optarg)) {
=======
			if (set_led(0, optarg)) {
>>>>>>> daf014f8e535af3acc18d02d8d1b76baafa6cdb4
				flash_led("none", optarg);
				fprintf(stderr, "LED is cleared\n");
			} else {
				fprintf(stderr, "Failed clearing LED\n");
				return 1;
			}
			return 0;
		case 'f':
<<<<<<< HEAD
			if (!flash_led("timer", optarg)) {
=======
			if (flash_led("timer", optarg)) {
>>>>>>> daf014f8e535af3acc18d02d8d1b76baafa6cdb4
				fprintf(stderr, "LED is flashing\n");
			} else {
				fprintf(stderr, "Failed flashing LED\n");
				return 1;
			}
			return 0;
		case 'g':
<<<<<<< HEAD
			if (!get_led(&val, optarg)) {
				fprintf(stderr, "value of LED is %d\n",
					val);
			} else {
				fprintf(stderr, "Failed getting value\n");
=======
			if (get_led(&val, optarg)) {
				fprintf(stderr, "value of LED is %d\n",
					val);
			} else {
				fprintf(stderr, "Failed getting value %s\n",
					strerror(errno));
>>>>>>> daf014f8e535af3acc18d02d8d1b76baafa6cdb4
				return 1;
			}
			return 0;
		case 'h':
			return usage(0, prog);
		case 's':
<<<<<<< HEAD
			if (!set_led(1, optarg)) {
=======
			if (set_led(1, optarg)) {
>>>>>>> daf014f8e535af3acc18d02d8d1b76baafa6cdb4
				fprintf(stderr, "LED is set\n");
			} else {
				fprintf(stderr, "Failed setting LED\n");
				return 1;
			}
			return 0;
		case 'v':
			fprintf(stderr, "%s", VERSION);
			return 0;
		default:
			return usage(1, prog);
		}
	}

	return 1;
}

