#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/uart.h>

#include <string.h>  // For memset

/**
 * This program reads data over uart1, then prints the receved data along with debug info
 * to uart0 with.
 */

#define DEBUG_PRINT_ENABLED 1

const struct device *gps_uart = DEVICE_DT_GET(DT_NODELABEL(uart1));  // uart1: TX = P0.04, RX = P0.05.    TO BE USED FOR UART COMMS WITH GPS
const struct device *dbg_uart = DEVICE_DT_GET(DT_NODELABEL(uart0));  // uart0: TX = P0.28, RX = P0.30.    TO BE USED FOR UART DEBUGGING

#define BUFF_SIZE 50  // Note: UART_RX_RDY event only occurs when RX buffer is full.
static char* rx_buf;  // Buffer that is used internally by UART callback


static void dbg_print(char* buf, int len) {
	/**
	 * Helper function to print things over debug uart
 	 */
	if (DEBUG_PRINT_ENABLED) {
		uart_tx(dbg_uart, buf, len, SYS_FOREVER_US);
	}
}


void handle_uart_rx_data(struct uart_event *evt) {
	/**
	 * Parse string received over UART and store important info. 
	 * 
	 * IMPORTANT: THIS IS CALLED INSIDE OF gps_uart_cb SO WE CANNOT DEBUG PRINT HERE
  	 */
	for (int i=0; i < evt->data.rx.len; i++) {
		// dbg_print(&(evt->data.rx.buf[evt->data.rx.offset + i]), 1);
	}
}


static void gps_uart_cb(const struct device *dev, struct uart_event *evt, void *user_data) {
	/**
	 * IMPORTANT: DEBUG PRINTS INSIDE THE UART CALLBACK FUNCTION WILL CASE THE FUNCTION TO RETURN
 	 */

	int ret;

	switch (evt->type) {
		case UART_RX_RDY:
			handle_uart_rx_data(evt);
			break;
		case UART_RX_BUF_REQUEST:
			rx_buf = k_malloc(BUFF_SIZE * sizeof(uint8_t));
			if (rx_buf) {
				ret = uart_rx_buf_rsp(gps_uart, rx_buf, BUFF_SIZE);
				if (ret) { 
					dbg_print("uart_rx_buf_rsp failed.", sizeof("uart_rx_buf_rsp failed."));
				}
			} else {
				dbg_print("Not able to allocate UART receive buffer.", sizeof("Not able to allocate UART receive buffer."));
			}
			break;
		case UART_RX_BUF_RELEASED:
			k_free(rx_buf);
			break;
		case UART_RX_DISABLED:
			ret = uart_rx_enable(gps_uart, rx_buf, BUFF_SIZE, SYS_FOREVER_US);
			if (ret) { 
				dbg_print("uart_rx_enable failed.", sizeof("uart_rx_enable failed."));
			}
			break;
		case UART_RX_STOPPED:
			break;
		default:
			break;
	}
}


static void dbg_uart_cb(const struct device *dev, struct uart_event *evt, void *user_data) { }  // No need to do anything


int main(void) {

	k_msleep(1000);
	int ret;

	/////////////////////////////////// DEBUG UART SETUP ///////////////////////////////////
	if (!device_is_ready(dbg_uart)) {
		return -1;
	}

	ret = uart_callback_set(dbg_uart, dbg_uart_cb, NULL);
	if (ret) {
		return ret;
	}

	/////////////////////////////////// GPS UART SETUP ///////////////////////////////////
	if (!device_is_ready(gps_uart)) {
		dbg_print("gps_uart not ready.", sizeof("gps_uart not ready."));
		return -1;
	}

	ret = uart_callback_set(gps_uart, gps_uart_cb, NULL);
	if (ret) {
		dbg_print("gps_uart_cb set failed.", sizeof("gps_uart_cb set failed."));
		return ret;
	}

	k_msleep(1000);
	ret = uart_rx_enable(gps_uart, rx_buf, BUFF_SIZE, SYS_FOREVER_US);
	if (ret) {
		dbg_print("gps_uart uart_rx_enable failed.", sizeof("gps_uart uart_rx_enable failed."));
		return ret;
	}

	/////////////////////////////////// MAIN LOOP ///////////////////////////////////
	int ctr = 0;
	while (1) {
		ctr++;
		k_msleep(250);
	}

	return 0;
}
