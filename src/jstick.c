#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>

/* test_bit  : Courtesy of Johan Deneux */
#define BITS_PER_LONG (sizeof(long) * 8)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)    ((array[LONG(bit)] >> OFF(bit)) & 1)

#define DEAD_ZONE 3500
#define ZERO_VAL 0
#define MAX_VAL 32768

#define B_KEY 305
#define Y_KEY 308
#define X_KEY 307
#define A_KEY 304
#define LB_KEY 310
#define RB_KEY 311
#define LT_KEY 2
#define RT_KEY 5
#define DPAD_UD 17
#define DPAD_LR 16
#define LANALOG_UD 1
#define LANALOG_LR 0
#define LANALOG_PRESS 317
#define RANALOG_UD 4
#define RANALOG_LR 3
#define RANALOG_PRESS 318
#define START 315
#define SELECT 314
#define HOME 316

// Global struct to contain all joystick button values
struct joystick js;

// Rumble effect variables.
static struct ff_effect effects; 
static struct input_event stop; 
static struct input_event play;
int rumble_flag;

// Device name of the XBox Controller being used.
char devname[] = "/dev/input/event0";
static unsigned long features[4];

struct analog
{
	int up, down, left, right;
	int press;
};

struct dpad
{
	int up, down, left, right;
};

struct joystick
{
	int device;
	int B, Y, X, A;
	int LB, RB, LT, RT;
	struct dpad dpad;
	struct analog lanalog;
	struct analog ranalog;
	int start, select;
	int home;
	int disconnect;
	int hasrumble;
	struct input_event last_event;
};

// Function to check if joystick device exists (is connected).
int exists(const char *fname)
{
	FILE *file;
	if (file = fopen(fname, "r"))
	{
		fclose(file);
		return 1;
	}
	return 0;
}

void init_print_js()
{
	printf("\e[2J\e[H");
	printf("                 ``****.....``     DEBUG MODE     ``.....****``                 \n");
	printf("             ```*`````````  ..`````````..`````````..  ````````*````             \n");
	printf("            .``````       `.`         \\ /          `.`       ``````.            \n");
	printf("          `.`              ``          *           ``              `.`          \n");
	printf("         .                   ``       / \\        ``                   .         \n");
	printf("        .       ****           ``````````````````         .-`-.        .        \n");
	printf("       `         .`.                                     .``*``.        `       \n");
	printf("      ``    ****`.*.`****       ....        ....     `````.`-`.`````    ``      \n");
	printf("      .           `             ..*.`      `. *.   `.` *`.     .`* `-    .      \n");
	printf("     .          ****             ``          ``     `....```-```....`     .     \n");
	printf("    ``                       .                           .`.*.`.          ``    \n");
	printf("    .                      .` `.                          .:.:.`           .    \n");
	printf("   .                       . * .                  ****                      .   \n");
	printf("  ``                   .````   ````.              .`.                       ``  \n");
	printf("  .                   `. *       * .`        ****`.*.`****                   .  \n");
	printf(" `                      ````   ````                `                          . \n");
	printf(" .                         . * .                  ****                        . \n");
	printf("``                          `.`                                                `\n");
	printf(".                                                                              .\n");
	printf(".                        ``````````````````````````````                        .\n");
	printf(".                     ```                              ```                     .\n");
	printf(".                  ```                                    ```                  .\n");
	printf(".                ``                                          ``                .\n");
	printf(".              ``                                              ``              .\n");
	printf(" .           ``                                                  ``           . \n");
	printf("  ``       ``                                                      ``       ``  \n");
	printf("    ```````                                                          ```````    \n");
}

void update_print_js()
{
	printf("\033[%d;%dH%04d\n", 1, 20, js.LT);
	printf("\033[%d;%dH%04d\n", 1, 58, js.RT);
	printf("\033[%d;%dH%d\n", 2, 17, js.LB);
	printf("\033[%d;%dH%d\n", 2, 63, js.RB);
	printf("\033[%d;%dH%d\n", 4, 40, js.home);
	printf("\033[%d;%dH%04d\n", 6, 17, js.lanalog.up);
	printf("\033[%d;%dH%d\n", 7, 61, js.Y);
	printf("\033[%d;%dH%04d\n", 8, 13, js.lanalog.left);
	printf("\033[%d;%dH%d\n", 8, 19, js.lanalog.press);
	printf("\033[%d;%dH%04d\n", 8, 22, js.lanalog.right);
	printf("\033[%d;%dH%d\n", 9, 35, js.select);
	printf("\033[%d;%dH%d\n", 9, 47, js.start);
	printf("\033[%d;%dH%d\n", 9, 56, js.X);
	printf("\033[%d;%dH%d\n", 9, 66, js.B);
	printf("\033[%d;%dH%04d\n", 10, 17, js.lanalog.down);
	printf("\033[%d;%dH%d\n", 11, 61, js.A);
	printf("\033[%d;%dH%d\n", 13, 30, js.dpad.up);
	printf("\033[%d;%dH%04d\n", 13, 51, js.ranalog.up);
	printf("\033[%d;%dH%d\n", 15, 26, js.dpad.left);
	printf("\033[%d;%dH%d\n", 15, 34, js.dpad.right);
	printf("\033[%d;%dH%04d\n", 15, 46, js.ranalog.left);
	printf("\033[%d;%dH%d\n", 15, 52, js.ranalog.press);
	printf("\033[%d;%dH%04d\n", 15, 55, js.ranalog.right);
	printf("\033[%d;%dH%d\n", 17, 30, js.dpad.down);
	printf("\033[%d;%dH%04d\n", 17, 51, js.ranalog.down);
	if (js.hasrumble) {
		printf("\033[%d;%dH%s\n", 28, 0, "Device can rumble! c:");
	} else {
		printf("\033[%d;%dH%s\n", 28, 0, "Device can't rumble :c");
	}
}

// Opens joystick device file and sets all struct variables to 0.
void init_joystick(struct joystick *js, char devname[])
{
	while (!exists(devname))
		sleep(0.5);

	js->device = open(devname, O_RDWR);
	js->B = 0;
	js->Y = 0;
	js->X = 0;
	js->A = 0;
	js->LB = 0;
	js->RB = 0;
	js->LT = 0;
	js->RT = 0;
	js->dpad.up = 0;
	js->dpad.down = 0;
	js->dpad.left = 0;
	js->dpad.right = 0;
	js->lanalog.up = 0;
	js->lanalog.down = 0;
	js->lanalog.left = 0;
	js->lanalog.right = 0;
	js->lanalog.press = 0;
	js->ranalog.up = 0;
	js->ranalog.down = 0;
	js->ranalog.left = 0;
	js->ranalog.right = 0;
	js->ranalog.press = 0;
	js->start = 0;
	js->select = 0;
	js->home = 0;
	js->disconnect = 0;
	js->hasrumble = 0;

	/* Prepare the rumble effects */ 
	effects.type = FF_RUMBLE; 
	effects.u.rumble.strong_magnitude = 65535; 
	effects.u.rumble.weak_magnitude = 65535; 
	effects.replay.length = 1000; 
	effects.replay.delay = 0; 
	effects.id = -1; 
 
	/* Prepare the stop rumble event */ 
	stop.type = EV_FF; 
	stop.code = effects.id; 
	stop.value = 0; 
       
	/* Prepare the play rumble event */ 
	play.type = EV_FF; 
	play.code = effects.id; 
	play.value = 1;

	rumble_flag = 0;

	if (ioctl(js->device, EVIOCGBIT(EV_FF, sizeof(unsigned long) * 4), features) == -1)
	{
		/* This device can't rumble, or the drivers don't support it */
	} else {
		/* Success! This device can rumble! */
		if (test_bit(FF_RUMBLE, features))
		{
			js->hasrumble = 1;
		}
	}
}

/* Remaps the analog values to a range between 0 and 1023.
 * There was a need for setting up a dead zone because the analogs
 * never got back to 0 after the user let go of them.
 */
int analog_map(int v_max, int v_min, int value)
{
	if (v_max > v_min)
	{
		v_min += DEAD_ZONE;
		if (value < v_min)
			return 0;
	}
	else
	{
		v_min -= DEAD_ZONE;
		if (value > v_min)
			return 0;
	}
	return ((1023 * (value - v_min)) / (v_max - v_min));
}

// Updates the joystick struct according to the buttons pressed.
void update_joystick(struct joystick *js)
{
	struct input_event ev;
	int temp;

	// Checks if a successful reading of the device path was possible.
	// The code gets stuck in this read() function if no new input is given
	// but the device is still connected.
	if (read(js->device, &ev, sizeof(ev)) != -1)
	{
		if (ev.type != 0)
		{
			js->last_event = ev;

			// This switch encodes the input_event code value for our joystick.
			switch (ev.code)
			{
			case B_KEY:
				js->B = ev.value;
				break;
			case Y_KEY:
				js->Y = ev.value;
				break;
			case X_KEY:
				js->X = ev.value;
				break;
			case A_KEY:
				js->A = ev.value;
				break;
			case LB_KEY:
				js->LB = ev.value;
				break;
			case RB_KEY:
				js->RB = ev.value;
				break;
			case LT_KEY:
				js->LT = ev.value;
				break;
			case RT_KEY:
				js->RT = ev.value;
				break;
			case START:
				js->start = ev.value;
				break;
			case SELECT:
				js->select = ev.value;
				break;
			case HOME:
				js->home = ev.value;
				break;
			case LANALOG_PRESS:
				js->lanalog.press = ev.value;
				break;
			case RANALOG_PRESS:
				js->ranalog.press = ev.value;
				break;
			case DPAD_UD:
				if (ev.value == 1)
				{
					js->dpad.up = 0;
					js->dpad.down = 1;
				}
				else if (ev.value == -1)
				{
					js->dpad.up = 1;
					js->dpad.down = 0;
				}
				else
				{
					js->dpad.up = 0;
					js->dpad.down = 0;
				}
				break;
			case DPAD_LR:
				if (ev.value == 1)
				{
					js->dpad.right = 1;
					js->dpad.left = 0;
				}
				else if (ev.value == -1)
				{
					js->dpad.right = 0;
					js->dpad.left = 1;
				}
				else
				{
					js->dpad.left = 0;
					js->dpad.right = 0;
				}
				break;
			case LANALOG_UD:
				if (ev.value < ZERO_VAL)
				{
					temp = analog_map(-MAX_VAL, ZERO_VAL, ev.value);
					js->lanalog.down = 0;
					js->lanalog.up = temp;
				}
				else
				{
					temp = analog_map(MAX_VAL-1, ZERO_VAL, ev.value);
					js->lanalog.up = 0;
					js->lanalog.down = temp;
				}
				break;
			case LANALOG_LR:
				if (ev.value < ZERO_VAL)
				{
					temp = analog_map(-MAX_VAL, ZERO_VAL, ev.value);
					js->lanalog.right = 0;
					js->lanalog.left = temp;
				}
				else
				{
					temp = analog_map(MAX_VAL-1, ZERO_VAL, ev.value);
					js->lanalog.left = 0;
					js->lanalog.right = temp;
				}
				break;
			case RANALOG_UD:
				if (ev.value < ZERO_VAL)
				{
					temp = analog_map(-MAX_VAL, ZERO_VAL, ev.value);
					js->ranalog.down = 0;
					js->ranalog.up = temp;
				}
				else
				{
					temp = analog_map(MAX_VAL-1, ZERO_VAL, ev.value);
					js->ranalog.up = 0;
					js->ranalog.down = temp;
				}
				break;
			case RANALOG_LR:
				if (ev.value < ZERO_VAL)
				{
					temp = analog_map(-MAX_VAL, ZERO_VAL, ev.value);
					js->ranalog.right = 0;
					js->ranalog.left = temp;
				}
				else
				{
					temp = analog_map(MAX_VAL-1, ZERO_VAL, ev.value);
					js->ranalog.left = 0;
					js->ranalog.right = temp;
				}
				break;
			}
		}
	}
	else
	{ // If there is a reading error (probably because the
		// controller disconnected), close the device and set the
		// disconnect flag to 1.
		close(js->device);
		js->disconnect = 1;
	}
}

PI_THREAD(rumble) {
	piHiPri(0);

	int file = open(devname, O_RDWR);

	/* Stop the effect if it's playing */ 
	stop.code =  effects.id; 
	if (write(file, (const void*) &stop, sizeof(stop)) == -1) { 
		printf("error stopping effect"); 
	} 

	/* Send the effect to the driver */ 
	if (ioctl(file, EVIOCSFF, &effects) == -1) { 
		printf("Error to send rumble effect to drive!\n");
		printf("errno: %s\n", strerror(errno));
	} 

	/* Play the effect */ 
	play.code = effects.id; 

	if (write(file, (const void*) &play, sizeof(play)) == -1){ 
		printf("Error to play rumble effect!\n");
		printf("errno: %s\n", strerror(errno));
	} 

	usleep((effects.replay.length+50)*1000);
	close(file);
	rumble_flag = 0;
}

void js_rumble(int strong, int weak, int duration) {
	if (js.hasrumble) {		
		if (!rumble_flag) {
			if ((strong < 0) || (strong > 100) || (weak < 0) || (weak > 100) || (duration < 0)) {
				printf("Invalid arguments to rumble bro!\n");
				return;
			}

			rumble_flag = 1;

			/* Modify effect data to create a new effect */ 
			effects.u.rumble.strong_magnitude = strong*65535/100; 
			effects.u.rumble.weak_magnitude = weak*65535/100; 
			effects.replay.length = duration; 
			effects.id = -1;   /* ID must be set to -1 for every new effect */

			piThreadCreate(rumble);
		}
	} else {
		printf("No rumble support!\n");
	}
}