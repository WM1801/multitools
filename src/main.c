#include <zephyr/kernel.h> 
#include <zephyr/device.h> 
#include <zephyr/drivers/led.h> 
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/__assert.h>
#include <string.h> 



#define STACKSIZE 1024

#define PRIORITY 7 

#define LED0_NODE DT_ALIAS(led0)
#define BUTTON_NODE DT_ALIAS(sw0)

#if !DT_NODE_HAS_STATUS_OKAY(LED0_NODE)
#error "Unsupported board: led0 devicetree alias is not defined"
#endif 

#if !DT_NODE_HAS_STATUS_OKAY(BUTTON_NODE)
#error "Unsupported board: sw0 devicetree alias is not defined" 
#endif 

struct printk_data_t { 
    void *fifo_reserved;  /*1st word reserved for use by fifo */
    uint8_t name[10]; 
    uint32_t cnt; 
}; 

K_FIFO_DEFINE(printk_fifo); 


//************************************************************** */
// task led 
//************************************************************** */
struct led { 
    struct gpio_dt_spec spec; 
    uint8_t num; 
};

static const struct led led0 = { 
	.spec = GPIO_DT_SPEC_GET_OR(LED0_NODE, gpios, {0}),
    .num = 0,
};


void blink(const struct led *led, uint32_t sleep_ms, uint32_t id)
{
    const struct gpio_dt_spec * spec = &led->spec; 
    int cnt = 0; 
    int ret; 

    if(!device_is_ready(spec->port)){
        printk("Error: %s device is not ready\n", spec->port->name);
        return; 
    }

    ret = gpio_pin_configure_dt(spec, GPIO_OUTPUT); 
    if (ret != 0) {
        printk("Error %d: failed to configure pin %d (LED %d)\n", 
        ret, spec->pin, led->num); 
        return; 
    }

    while(1) {
        gpio_pin_set(spec->port, spec->pin, cnt%2); 
        struct printk_data_t tx_data = { .name = "led", .cnt = cnt}; 
        size_t size = sizeof(struct printk_data_t); 
        char *mem_ptr = k_malloc(size); 
        __ASSERT_NO_MSG(mem_ptr != 0); 

        memcpy(mem_ptr, &tx_data, size); 
        k_fifo_put(&printk_fifo, mem_ptr);  

        k_msleep(sleep_ms); 
        cnt++; 
    }
};


void blink0(void)
{
    blink(&led0, 200, 0); 
}


//*************************************************************** */
// task log
//*************************************************************** */
void uart_out(void)
{
    while(1){
        struct printk_data_t *rx_data = k_fifo_get(&printk_fifo, K_FOREVER); 
        printk("%s : counter =%d\n", 
        rx_data->name, rx_data->cnt); 
        k_free(rx_data); 
    }
}

//*************************************************************** */
// task button 
//*************************************************************** */
#define PERIOD_READ_BUTTON_MS 30

typedef enum {
    BUTTON_UP, 
    BUTTON_DOWN, 
} button_state_t; 

struct button_config_t {
    struct gpio_dt_spec spec; 
    uint8_t num; 
};

struct button_t {
    struct button_config_t config;  
    button_state_t current_state; 
    button_state_t prev_state; 
};

static const struct button_config_t button0_config = { 
	.spec = GPIO_DT_SPEC_GET_OR(BUTTON_NODE, gpios, {0}),
    .num = 0,
};

static struct button_t button0 = { 
	.config = button0_config, 
    .current_state = BUTTON_UP, 
    .prev_state = BUTTON_UP,
};



void button_main(struct button_t *button, uint32_t sleep_ms, uint32_t id)
{
    const struct gpio_dt_spec * spec = &button->config.spec; 
    int cnt = 0; 
    int ret; 

    if(!device_is_ready(spec->port)){
        printk("Error: %s device is not ready\n", spec->port->name);
        return; 
    }

    ret = gpio_pin_configure_dt(spec, GPIO_INPUT); 
    if (ret != 0) {
        printk("Error %d: failed to configure pin %d (BUTTON %d)\n", 
        ret, spec->pin, button->config.num); 
        return; 
    }

    while(1) {

        button->prev_state = button->current_state; 
        button->current_state = gpio_pin_get_dt(&button->config.spec) ? \
            BUTTON_DOWN : \
            BUTTON_UP; 

        if (button->prev_state != button->current_state) {

            const char *evt = (button->current_state == BUTTON_DOWN) ? "press" : "release";

            struct printk_data_t tx_data = { .cnt = cnt };
            strncpy((char *)tx_data.name, evt, sizeof(tx_data.name));

            size_t size = sizeof(struct printk_data_t);
            char *mem_ptr = k_malloc(size);
            __ASSERT_NO_MSG(mem_ptr != 0);

            memcpy(mem_ptr, &tx_data, size);
            k_fifo_put(&printk_fifo, mem_ptr);
            cnt++; 
        }
        k_msleep(PERIOD_READ_BUTTON_MS); 
        
    }
};

void button0_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1); ARG_UNUSED(p2); ARG_UNUSED(p3);
    button_main(&button0, PERIOD_READ_BUTTON_MS, 0);
}


//K_THREAD_DEFINE(blink0_id, STACKSIZE, blink0, NULL, NULL, NULL, PRIORITY, 0, 0); 
K_THREAD_DEFINE(uart_out_id, STACKSIZE, uart_out, NULL, NULL, NULL, PRIORITY, 0, 0); 
K_THREAD_DEFINE(button0_id, STACKSIZE, button0_thread, NULL, NULL, NULL, PRIORITY, 0, 0); 
