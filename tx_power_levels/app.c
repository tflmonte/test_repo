#include "app.h"
#include "gatt_db.h"
#include "native_gecko.h"
#include "stdio.h"
#include "retargetserial.h"


#define MIN_TEST_NUMBER					(-300) // This is the minimum tx_power to start the sweep
#define MAX_TEST_NUMBER					(80)   // This is the maximum tx_power to end the sweep

static int16_t tx_set;
static int16_t tx_to_set = MIN_TEST_NUMBER;


void AppHandler(void){
	RETARGET_SerialInit();

	while (1) {
		/* Event pointer for handling events */
		struct gecko_cmd_packet* evt;

		/* Check for stack event. */
		evt = gecko_peek_event();

		/* Handle events */
		switch (BGLIB_MSG_ID(evt->header)) {
			/* This boot event is generated when the system boots up after reset.
			 * Do not call any stack commands before receiving the boot event.
			 * Here the system is set to start advertising immediately after boot procedure. */
			case gecko_evt_system_boot_id:

				printf("--------------------------------------------Start--------------------------------------------\r\n");

				/* Set a timer to call system_tx_power every 10ms with incremental input and print out result */
				gecko_cmd_hardware_set_soft_timer(328, 1, 0);
				break;

			case gecko_evt_hardware_soft_timer_id:

				if(evt->data.evt_hardware_soft_timer.handle == 1){

					/* Set tx power and read what was actually set */
					tx_set = gecko_cmd_system_set_tx_power(tx_to_set)->set_power;

					/* Printf out input value and response from the command
					 * The print formatting can be changed as desired (e.g. so that a log can be imported as csv into excel)
					 */
					printf("set_tx_power(%03d) returns %03d\r\n", tx_to_set, tx_set);

					/* Increment and check if we have reached the Max value */
					if (tx_to_set++ == MAX_TEST_NUMBER){
						/* If max value has been reached print "End" and stop timer */
						printf("--------------------------------------------End--------------------------------------------\r\n");
						gecko_cmd_hardware_set_soft_timer(0, 1, 0);
					}
				}
				break;

			default:
				break;
		}
	}
}
