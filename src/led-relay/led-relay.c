/*
 * Copyright (c) 2021 Westermo Network Technologies
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LED_PATH    "/sys/class/leds/"
#define RELAY_PATH  "/sys/class/relay-ctrl/bypass-relay/"


#define COMPOSE(path, fmt)					\
        va_list ap;						\
								\
	va_start(ap, fmt);					\
        rc = vsnprintf(path, sizeof(path), fmt, ap);		\
        va_end(ap);						\
								\
	if (rc >= (int)sizeof(path)) {				\
		errno = ENAMETOOLONG;				\
		warn("Failed composing path %s", path);		\
		return 1;					\
	}

static int fwritef(char *buf, const char *fmt, ...)
{
	char path[512];
	FILE *fp;
	int rc;

	COMPOSE(path, fmt);

	fp = fopen(path, "w");
	if (!fp) {
		warn("Failed opening file %s for writing", path);
		return 1;
	}

	fprintf(fp, "%s\n", buf);

	return fclose(fp);
}

static int freadf(char *buf, size_t len, const char *fmt, ...)
{
	char path[512];
	FILE *fp;
	int rc;

	COMPOSE(path, fmt);

	fp = fopen(path, "r");
	if (!fp) {
		warn("Failed opening file %s for reading", path);
		return 1;
	}

	if (!fgets(buf, len, fp)) {
		warn("Failed reading file %s", path);
		rc = 1;
	} else {
		rc = 0;
	}

	return fclose(fp) || rc;
}

static int bypass_relay(char *val)
{
	return fwritef(val, RELAY_PATH "value");
}

static int flash_led(char *val, char *port)
{
	return fwritef(val, LED_PATH "%s/trigger", port);
}

static int set_led(char *val, char *port)
{
	return fwritef(val, LED_PATH "%s/brightness", port);
}

static int get_led(char *buf, size_t len, char *port)
{
	return freadf(buf, len, LED_PATH "%s/brightness", port);
}

static int usage(int rc, char *prog)
{
	fprintf(rc ? stderr : stdout,
		"Usage: %s [OPTIONS]\n\n"
		"  -b VAL   Disable(2) or Enable(1) bypass relays\n"
		"  -c LED   Clear LED or relay value to 0\n"
		"  -f LED   Start flashing LED\n"
		"  -g LED   Get LED or relay value\n"
		"  -h       Show this help text\n"
		"  -s LED   Set LED or relay on\n"
		"  -v       Show program version\n"
		"\n"
		"Examples:\n"
		"  Flash LED      : %s -f ethX5:yellow:state\n"
		"  Set LED        : %s -s ethX6:yellow:state\n"
		"  Clear LED      : %s -c ethX5:yellow:state\n"
		"  Disable bypass : %s -b 2\n"
		"  Enable bypass  : %s -b 1\n"
		"\n", prog, prog, prog, prog, prog, prog);

	return rc;
}

int main(int argc, char *argv[])
{
	char *prog, *ptr;
	char val[128];
	int c;

	prog = argv[0];
	ptr = strrchr(prog, '/');
	if (ptr)
		prog = ptr+1;

	while ((c = getopt(argc, argv, "b:c:f:g:hs:v")) != EOF) {
		switch (c) {
		case 'b':
			if (bypass_relay(optarg))
				errx(1, "Failed setting bypass relay");
			break;

		case 'c':
			if (set_led("0", optarg))
				errx(1, "Failed clearing LED %s", optarg);
			flash_led("none", optarg);
			break;

		case 'f':
			if (flash_led("timer", optarg))
				errx(1, "Failed flashing LED %s", optarg);
			break;

		case 'g':
			if (get_led(val, sizeof(val), optarg))
				return 1;
			puts(val);
			break;

		case 'h':
			return usage(0, prog);

		case 's':
			if (set_led("1", optarg))
				errx(1, "Failed setting LED %s", optarg);
			break;

		case 'v':
			puts(VERSION);
			break;

		default:
			return usage(1, prog);
		}
	}

	return 0;
}
