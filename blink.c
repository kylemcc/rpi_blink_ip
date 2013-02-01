#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <errno.h>

#ifndef NDEBUG

static const char *LED_ON =  "***";
static const char *LED_OFF = "   ";
#define CTRL_FILE stdout

#else

#ifdef RPI
static const char *CTRL_FILE_NAME = "/sys/class/leds/led0/brightness";

/*
 * Turn off the mmc0 trigger (which indicates SD card activity)
 */
void __attribute__ ((constructor)) set_trigger()
{
	system("echo none > /sys/class/leds/led0/trigger");
}

/*
 * Turn the mmc0 (SD card) trigger back on
 */
void cleanup(void)
{
	system("echo mmc0 > /sys/class/leds/led0/trigger");
}

#else
static const char *CTRL_FILE_NAME = "/dev/null";
#endif


static const char *LED_ON =  "255";
static const char *LED_OFF = "0  ";

static FILE *__ctrl_file;
#define CTRL_FILE __ctrl_file

void __attribute__ ((constructor)) con()
{
	__ctrl_file = fopen(CTRL_FILE_NAME, "w+");
}

#endif

/*
 * short (.) = 1 unit long
 * long (-) = 3 units long
 * gap between elements of a letter (dots and dashes) = 1 unit long
 * short gap (between letters) = 3 units long
 * medium gap (between words) = 7 units long
 */

static const struct timeval SHORT		= {0, 350000}; // 350ms
static const struct timeval LONG			= {1, 50000};  // 1.05s
static const struct timeval ELEM_GAP	= {0, 350000}; // 350ms
static const struct timeval SHORT_GAP	= {1, 50000};  // 1.05s
static const struct timeval MED_GAP		= {2, 450000}; // 2.45s

/*
 * (ITU) Morse code definitions for digits 0 - 9,
 * the array index maps to the proper code.
 */
static const char *MORSE_CODE_NUMBERS[] = {
	"-----", // 0
	".----", // 1
	"..---", // 2
	"...--", // 3
	"....-", // 4
	".....", // 5
	"-....", // 6
	"--...", // 7
	"---..", // 8
	"----."  // 9
};

static const int NUM_CODES = 10;

static void my_sleep(struct timeval tv)
{
	select(0, NULL, NULL, NULL, &tv);
}

static struct ifaddrs *get_ip_addr()
{
	struct  ifaddrs *myaddrs;
	struct ifaddrs *ifa;

	if(getifaddrs(&myaddrs) != 0) {
		perror("Call to getifaddrs failed.");
		exit(1);
	}

	for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;

		if ((ifa->ifa_flags & (IFF_UP | IFF_LOOPBACK)) == IFF_UP 
				&& ifa->ifa_addr->sa_family == AF_INET) {
			// If it's up and not loopback, and IPv4
			// TODO: IPv6...that'll be fun
			return ifa;
		}
	}

	return NULL;
}

static void tap(struct timeval duration, struct timeval gap)
{
	fprintf(CTRL_FILE, "%s\r", LED_ON);
	fflush(CTRL_FILE);

	my_sleep(duration);

	fprintf(CTRL_FILE, "%s\r", LED_OFF);
	fflush(CTRL_FILE);

	my_sleep(gap);
}

static void flutter()
{
	int i;
	for (i=0; i < 5; i++) {
		struct timeval tv = {0, 50000};
		tap(tv, tv);
	}
}

static void process_char(char c)
{
	switch (c) {
		case '.':
			// This is not actually Morse code for the period (.) 
			// character. Just wanted something to clearly separate 
			// the octets that is quicker than Morse code for period (.-.-.-)
			flutter();
			my_sleep(SHORT_GAP);
			break;
		default:
			if (c > 47 && c < 58) {
				int number = c - 48;
				const char *code = MORSE_CODE_NUMBERS[number];
				const char *c;
				for (c = code; (c - code) < strlen(code); c++) {
					switch (*c) {
						case '.':
							tap(SHORT, ELEM_GAP);
							break;
						case '-':
							tap(LONG, ELEM_GAP);
							break;
						default:
							break;
					}
				}
			}
			break;
	}
}


int main(int argc, char *argv[])
{
#ifdef RPI
	atexit(cleanup);
#endif

	struct ifaddrs *my_ifaddr = get_ip_addr();

	if (my_ifaddr == NULL) {
		perror("Could not obtain IP address.");
		return -1;
	}

	struct sockaddr_in *sin = (struct sockaddr_in *)my_ifaddr->ifa_addr;
	char ip[INET_ADDRSTRLEN];

	inet_ntop(AF_INET, &(sin->sin_addr), ip, INET_ADDRSTRLEN);
	//printf("Local ip: %s\n", ip);

	flutter();
	my_sleep(MED_GAP);
	char *cur;
	for (cur = ip; (cur - ip) < INET_ADDRSTRLEN; cur++) { 
		my_sleep(SHORT_GAP);
		process_char(*cur);
	}

	return 0;
}

