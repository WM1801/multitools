#include <zephyr/kernel.h> 
#include <zephyr/drivers/led.h> 
#include <zephyr/sys/printk.h>

#include "button.h"

#define LED0_NODE DT_ALIAS(led0)


/* Получение спецификации LED с помощью макроса LED_DT_SPEC_GET */
static const struct led_dt_spec blue_led = LED_DT_SPEC_GET(DT_ALIAS(led0));

int main(void)
{
    int ret; 

    /* Проверка готовности устройства */
    if (!led_is_ready_dt(&blue_led)) {
        printk("LED device not ready\n");
        return -1;
    }

    ret = button_init(); 
    if (ret < 0) {
        return ret; 
    }


    while (1) {

        switch(button_poll()){ 
            case BUTTON_EVT_PRESS:
            printk("button press\n");
            led_on_dt(&blue_led);
            break;
        case BUTTON_EVT_RELEASE:
            printk("button release\n");
            led_off_dt(&blue_led);
            break;
        case BUTTON_EVT_NONE:
            break;
        }
        k_sleep(K_MSEC(5));
    }
}

