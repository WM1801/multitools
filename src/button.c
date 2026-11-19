#include "button.h"


#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define BUTTON_NODE DT_ALIAS(sw0)

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios); 

#define PERIOD_READ_BUTTONS_MS 30


typedef enum { 
    BUTTON_UP, 
    BUTTON_DOWN, 
} button_state_t; 

static struct {
    button_state_t current_state; 
    button_state_t prev_state; 
    uint32_t time_prev_read; 
} btn = { 
    .current_state = BUTTON_UP, 
    .prev_state    = BUTTON_UP, 
    .time_prev_read= 0, 
}; 

int button_init(void)
{
    if (!gpio_is_ready_dt(&button)){
        return -ENODEV; 
    }
    return gpio_pin_configure_dt(&button, GPIO_INPUT); 
}

button_event_t button_poll(void)
{
    uint32_t now = k_uptime_get_32(); 
    if ((now -btn.time_prev_read) < PERIOD_READ_BUTTONS_MS) {
        return BUTTON_EVT_NONE; 
    }

    btn.time_prev_read = now; 

    btn.prev_state = btn.current_state; 
    btn.current_state = gpio_pin_get_dt(&button) ? BUTTON_DOWN : BUTTON_UP; 

    if (btn.current_state == btn.prev_state) {
        return BUTTON_EVT_NONE; 
    }

    return (btn.current_state == BUTTON_DOWN) ? BUTTON_EVT_PRESS
                                              : BUTTON_EVT_RELEASE; 
}
