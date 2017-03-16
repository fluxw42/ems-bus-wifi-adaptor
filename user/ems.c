#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"


#define DEBUG_PIN_NO   14
#define DEBUG_PIN_MASK BIT14
#define DEBUG_PIN_FUNC FUNC_GPIO14
#define DEBUG_PIN_MUX  PERIPHS_IO_MUX_MTMS_U

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void user_procTask(os_event_t *events);

static volatile os_timer_t some_timer;

void toggle_debug() {
        GPIO_OUTPUT_SET(DEBUG_PIN_NO, 1);
        GPIO_OUTPUT_SET(DEBUG_PIN_NO, 0);
}


void gpio_interrupt(void *arg) {

    uint32	gpio_status;

    gpio_status	=	GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

    toggle_debug();

}

void some_timerfunc(void *arg)
{

    //Do blinky stuff
    if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & BIT2)
    {
        //Set GPIO2 to LOW - TURN OFF the LED
        gpio_output_set(0, BIT2, BIT2, 0);
    }
    else
    {
        //Set GPIO2 to HIGH - TURN ON the LED
        gpio_output_set(BIT2, 0, BIT2, 0);
    }

}

//Do nothing function
static void ICACHE_FLASH_ATTR  user_procTask(os_event_t *events)
{
    os_delay_us(10);
}

//Init function
void ICACHE_FLASH_ATTR user_init()
{
    // Initialize the GPIO subsystem.
    gpio_init();

    //Set GPIO2 to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    PIN_FUNC_SELECT(DEBUG_PIN_MUX, DEBUG_PIN_FUNC);

    //Set GPIOs low
    GPIO_OUTPUT_SET(2, 0);
	GPIO_OUTPUT_SET(DEBUG_PIN_NO, 0);


 	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U,FUNC_GPIO3);               //GPIO Alternate Function
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(3));                                  //Configure it in input mode.
    ETS_GPIO_INTR_DISABLE();                                          //Close the GPIO interrupt
    ETS_GPIO_INTR_ATTACH(&gpio_interrupt,NULL);                       //Register the interrupt function
    gpio_pin_intr_state_set(GPIO_ID_PIN(3), GPIO_PIN_INTR_ANYEGDE);   //Falling edge trigger
    ETS_GPIO_INTR_ENABLE() ;                                          //Enable the GPIO interrupt

    //Disarm timer
    os_timer_disarm(&some_timer);

    //Setup timer
    os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL);

    //Arm the timer, &some_timer is the pointer 1000 is the fire time in ms
    //0 for once and 1 for repeating timer
    os_timer_arm(&some_timer, 5000, 1);

    //Start os task
    system_os_task(user_procTask, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);
}

