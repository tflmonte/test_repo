#include "app.h"
#include "gatt_db.h"
#include "adv_cons.h"
#ifdef LOG_SUPPORT
#include "log.h"
#endif
extern uint8_t boot_to_dfu;

void AppHandler(void){
#ifdef LOG_SUPPORT
	INIT_LOG();
#endif
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
				setup_adv(extended);
#ifdef LOG_SUPPORT
				SE_CALL(gecko_cmd_le_gap_set_advertise_configuration(0, 0));
				// Try secondary phy = 2M
//				SE_CALL(gecko_cmd_le_gap_set_advertise_phy(0, 1, 2));
				SE_CALL(gecko_cmd_le_gap_start_advertising(0, le_gap_user_data, le_gap_connectable_non_scannable));
#else
				gecko_cmd_le_gap_set_advertise_configuration(0, 0);
				gecko_cmd_le_gap_start_advertising(0, le_gap_user_data, le_gap_connectable_non_scannable);
#endif
				break;
			case gecko_evt_hardware_soft_timer_id:
				break;
			case gecko_evt_le_connection_opened_id:
#ifdef LOG_SUPPORT
				LOGI("Connection Opened.\r\n");
#endif
				break;
			case gecko_evt_le_connection_closed_id:
#ifdef LOG_SUPPORT
				LOGI("Connection Closed.\r\n");
#endif
				/* Check if need to boot to dfu mode */
				if (boot_to_dfu) {
					/* Enter to DFU OTA mode */
					gecko_cmd_system_reset(2);
				} else {
					/* Restart advertising after client has disconnected */
					gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);
				}
				break;

				/* Events related to OTA upgrading
				 ----------------------------------------------------------------------------- */

				/* Check if the user-type OTA Control Characteristic was written.
				 * If ota_control was written, boot the device into Device Firmware Upgrade (DFU) mode. */
			case gecko_evt_gatt_server_user_write_request_id:

				if (evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_ota_control) {
					/* Set flag to enter to OTA mode */
					boot_to_dfu = 1;
					/* Send response to Write Request */
					gecko_cmd_gatt_server_send_user_write_response(evt->data.evt_gatt_server_user_write_request.connection,
							gattdb_ota_control, bg_err_success);

					/* Close connection to enter to DFU OTA mode */
					gecko_cmd_le_connection_close(evt->data.evt_gatt_server_user_write_request.connection);
				}
				break;

			default:
				break;
		}
	}
}
