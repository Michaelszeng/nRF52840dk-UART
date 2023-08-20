/*
 * THIS IS NOW BEING USED TO TEST UART RECEIVING
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/uart.h>

#define SLEEP_TIME_MS   1000


const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart1));

#define RECEIVE_BUFF_SIZE 4
// #define MAGIC_NUMBER '77'
static uint8_t* rx_buf; //A buffer to store incoming UART data


static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{

	switch (evt->type) {
	
	case UART_TX_DONE:
		// do something
		break;

	case UART_TX_ABORTED:
		// do something
		break;
		
	case UART_RX_RDY:
		// printk("	evt->data.rx.len: %d\n", evt->data.rx.len);
		// printk("	evt->data.rx.offset: %d\n", evt->data.rx.offset);
		// if (evt->data.rx.buf[evt->data.rx.offset] != '77') {
		// 	printk("Received data that did not start with the magic number. Ignoring it.\n");
		// 	// break;
		// }
		
		printk("data received: ");
		for (int i=0; i < evt->data.rx.len; i++) {
			printk("%02X ", evt->data.rx.buf[evt->data.rx.offset + i]);

			if (evt->data.rx.buf[evt->data.rx.offset + i] == '\n') {
				break;
			}
		}
		printk("\n");
		break;

	case UART_RX_BUF_REQUEST:
		// printk("UART_RX_BUF_REQUEST\n");
		rx_buf = k_malloc(RECEIVE_BUFF_SIZE * sizeof(uint8_t));
		if (rx_buf) {
			uart_rx_buf_rsp(uart, rx_buf, sizeof(rx_buf));
		} else {
			printk("WARNING: Not able to allocate UART receive buffer");
		}
		break;

	case UART_RX_BUF_RELEASED:
		// printk("UART_RX_BUF_RELEASED\n");
		k_free(rx_buf);
		break;
		
	case UART_RX_DISABLED:
		printk("UART_RX_DISABLED\n");
		uart_rx_enable(uart, rx_buf, sizeof(rx_buf), SYS_FOREVER_US);
		break;

	case UART_RX_STOPPED:
		printk("UART_RX_STOPPED\n");
		break;
		
	default:
		break;
	}
}


int main(void)
{
	k_msleep(1000);
	printk("Starting Program..\n");

	int ret;
	int err;

	// UART
	if (!device_is_ready(uart)) {
		printk("uart not ready. returning.\n");
		return -1;
	}

	ret = uart_callback_set(uart, uart_cb, NULL);
	if (ret) {
		return ret;
	}

	k_msleep(500);
	ret = uart_rx_enable(uart, rx_buf, sizeof(rx_buf), SYS_FOREVER_US);
	if (ret) {
		printk("uart_rx_enable faild with ret=%d\n", ret);
		return ret;
	}

	static char tx_buf[] = {'5', '7', '5', '\n'};

	int ctr = 0;
	while (1) {

		printk("Looping... %d\n", ctr);
		ctr++;

		err = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);
		if (err) {
			printk("uart_tx errored: %d.\n", err);
		}

		k_msleep(SLEEP_TIME_MS);

	}
	return 0;
}
