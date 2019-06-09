#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "jstick.c"
#include "motor.c"

#define DEBUG_JS 0

int keep_running = 1;	// end of program flag. it is controlled by the
						// joystick thread. if set to 0, all threads will
						// finish up and set their own flags to 1 so that
						// the main program can clean everything up and end.

int joystick_finished = 1;
int motors_finished = 1;

int shutdown_flag = 0, reboot = 0, close_program=0;	// flags set by joystick
													// commands so that the
													// program knows what to
													// do when finishing up.

/*
This is the main thread. In it, we are supposed to put everything that doesn't
belong in the infrastructure threads below it. Generally, it is used to test
new features using the joystick controller.
*/

// PI_THREAD(mira)
// {
//     mira_finished = 0;
// 	piHiPri(0);

// 	//void setup
// 	pinMode(DIR, OUTPUT);
// 	pinMode(STEP, OUTPUT);
// 	pinMode(MOD0, OUTPUT);
// 	pinMode(MOD1, OUTPUT);
// 	pinMode(MOD2, OUTPUT);

// 	//setep
// 	digitalWrite(MOD0, LOW);
// 	digitalWrite(MOD1, LOW);
// 	digitalWrite(MOD2, LOW);

// 	//direção
// 	digitalWrite(DIR, LOW);
// 	//digitalWrite(DIR, HIGH);

//     while(keep_running)
//     {
// 		//void loop
// 		digitalWrite(STEP, HIGH);
// 		delay(PERIOD/2);
// 		digitalWrite(STEP, LOW);
// 		delay(PERIOD/2);

// 	}
	
// 	mira_finished = 1;
// }

/*
This is the joystick thread. It runs the code behind reading and interpreting
joystick commands. The joystick is the primary way of commanding the robot and
will be eventually used as safety trigger. Aside from that, it allows the
operator to control the robot without the need for opening a terminal.
All joystick support functions available in the jstick.c file.
*/
PI_THREAD(joystick)
{
    joystick_finished = 0;
	piHiPri(0);

    init_joystick(&js, devname);
    if(DEBUG_JS) init_print_js();
 
	// START+SELECT finishes the program
    while((!(js.select && js.start)) && (keep_running)) 
    {
		if(DEBUG_JS) update_print_js();
		if(js.disconnect)
        {
        	//conectar novamente caso desconect
		    init_joystick(&js, devname);
		}
		
        update_joystick(&js);
	}
	// If a D-Pad key is pressed along with START+SELECT when finishing
	// the program, special finishing up routines are called inside the
	// clean_up() function. They are:

	// DOWN+START+SELECT: shuts the Raspberry Pi Zero W down
	if(js.dpad.down) shutdown_flag = 1;
	// UP+START+SELECT: reboots the Raspberry Pi Zero W
	if(js.dpad.up) reboot = 1;

	keep_running = 0;
	joystick_finished = 1;
}

// setMotorSpeed(LMOTOR, 0);
// setMotorSpeed(RMOTOR, 0);
// write_motors();

PI_THREAD(motors)
{
	motors_finished = 0;
	piHiPri(0);

	init_motors();


	int antigoR = 0;
	int antigoL = 0;
	int antigoU = 0;
	int antigoD = 0;

	int mudou = 0;

	
	while(keep_running)
	{
		if(js.lanalog.right != antigoR)
		{
			setMotorSpeed(XMOTOR, js.lanalog.right);
			// //inicio da zueira
			// if(js.lanalog.right == 0)
			// {
			// 	setMotorSpeed(XMOTOR, 0);
			// }
			// else
			// {
			// 	setMotorSpeed(XMOTOR, 300);
			// }
			// //fim da zueira
			antigoR = js.lanalog.right;
			mudou = 1;
		}
		else
		{
			if(js.lanalog.left != antigoL)
			{
				setMotorSpeed(XMOTOR, -js.lanalog.left);
				// //inicio da zueira
				// if(js.lanalog.left == 0)
				// {
				// 	setMotorSpeed(XMOTOR, 0);
				// }
				// else
				// {
				// 	setMotorSpeed(XMOTOR, -300);
				// }
				// //fim da zueira
				antigoL = js.lanalog.left;
				mudou = 1;
			}
			else
			{
				mudou = 0;
			}
		}

		if(js.lanalog.up != antigoU)
		{
			setMotorSpeed(YMOTOR, js.lanalog.up);
			// //inicio da zueira
			// if(js.lanalog.up == 0)
			// {
			// 	setMotorSpeed(YMOTOR, 0);
			// }
			// else
			// {
			// 	setMotorSpeed(YMOTOR, 300);
			// }
			// //fim da zueira
			antigoU = js.lanalog.up;
			mudou = 1;
		}
		else
		{
			if(js.lanalog.down != antigoD)
			{
				setMotorSpeed(YMOTOR, -js.lanalog.down);
				// //inicio da zueira
				// if(js.lanalog.down == 0)
				// {
				// 	setMotorSpeed(YMOTOR, 0);
				// }
				// else
				// {
				// 	setMotorSpeed(YMOTOR, -300);
				// }
				// //fim da zueira
				antigoD = js.lanalog.down;
				mudou = 1;
			}
		}

		if(mudou)
		{
			//printf("voumndar!\n");
			if(write_motors()!=0)
			{
				init_motors();
			}
			//printf("\tmandei\n");
		}
	}
	motors_finished = 1;
	
}


int am_i_su()
{
    if(geteuid())
    	return 0;
    return 1;
}

void clean_up()
{
	int nao_terminou = 1;
	
	do {
		nao_terminou = !(joystick_finished && motors_finished); 
		/*
		printf("%d...", main_finished);
		printf("%d...", joystick_finished);
		printf("%d...", debug_finished);
		printf("%d...", sensors_finished);
		printf("%d...", supervisory_finished);
		printf("%d...", plot_finished);
		printf("%d...", motors_finished);
		printf("%d...\n", nao_terminou);
		*/
	} while(nao_terminou);

	system("clear&");
	if(shutdown_flag) system("sudo shutdown now&");
	else if(reboot) system("sudo shutdown -r now&");
}

int main(int argc, char* argv[])
{
	if(!am_i_su())
	{
		printf("Restricted area. Super users only.\n");
		return 0;
	}

	wiringPiSetupPhys();

	
	piThreadCreate(joystick);
	piThreadCreate(motors);


	while(keep_running) delay(100);
	clean_up();

	return 0;
}