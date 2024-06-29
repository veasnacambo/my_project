#include <zephyr/drivers/gpio.h>
#include <stdio.h>
#include <zephyr/kernel.h>

#define GPIO_DEVICE_NAME_A "GPIOA"
#define GPIO_DEVICE_NAME_B "GPIOB"
#define GPIO_PIN_A0 0 // Pin 5 on GPIOA
#define GPIO_PIN_A1 1 // Pin 13 on GPIOB
#define LED_PIN1 9 // Pin 12 on GPIOB
#define LED_PIN2 11
#define LED_PIN3 15

int times()
{
    const struct device *gpio_dev_a;
    const struct device *gpio_dev_b;
    //gpio_port_value_t pin_value;
    int ret;

    gpio_dev_a = DEVICE_DT_GET_OR_NULL(DT_NODELABEL(gpioa));
    gpio_dev_b = DEVICE_DT_GET_OR_NULL(DT_NODELABEL(gpiob));

    if (!gpio_dev_a || !device_is_ready(gpio_dev_a)) {
        printf("GPIO device A not found or not ready\n");
        return -ENODEV;
    }

    if (!gpio_dev_b || !device_is_ready(gpio_dev_b)) {
        printf("GPIO device B not found or not ready\n");
        return -ENODEV;
    }

    // Configure the GPIO pin for the button A as an input
    ret = gpio_pin_configure(gpio_dev_a, GPIO_PIN_A0, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printf("Error configuring GPIO pin %d: %d\n", GPIO_PIN_A1, ret);
        return ret;
    }

    // Configure the GPIO pin for the button B as an input
    ret = gpio_pin_configure(gpio_dev_b, GPIO_PIN_A1, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printf("Error configuring GPIO pin %d: %d\n", GPIO_PIN_A1, ret);
        return ret;
    }

    // Configure the GPIO pin for the LED as an output
    ret = gpio_pin_configure(gpio_dev_b, LED_PIN1, GPIO_OUTPUT);
    if (ret < 0) {
        printf("Error configuring GPIO pin %d: %d\n", LED_PIN1, ret);
        return ret;
    }
    ret = gpio_pin_configure(gpio_dev_b, LED_PIN2, GPIO_OUTPUT);
    if (ret < 0) {
        printf("Error configuring GPIO pin %d: %d\n", LED_PIN2, ret);
        return ret;
    }
        ret = gpio_pin_configure(gpio_dev_b, LED_PIN3, GPIO_OUTPUT);
    if (ret < 0) {
        printf("Error configuring GPIO pin %d: %d\n", LED_PIN3, ret);
        return ret;
    }
    int x;
    for(int i =0; i<6;i++) {



		x = gpio_pin_get(gpio_dev_a,GPIO_PIN_A0);
		if (ret < 0) {
			// printk("Error reading GPIOA pin 5\n");
			return 0;
		}
		// printk("GPIOA pin 5 value: %d\n", x);


        // Read the value of the GPIO pin for the button B
        ret = gpio_pin_get(gpio_dev_b, GPIO_PIN_A1);
        if (ret < 0) {
            printf("Error reading GPIO pin %d: %d\n", GPIO_PIN_A0, ret);
            return ret;
        }

            // At least one button is pressed, turn the LED on
			if (x ==0)
			{
                gpio_pin_set(gpio_dev_b, LED_PIN3, 1);
                // printf("LED state: %d ",gpio_pin_get(gpio_dev_b,LED_PIN1));
			}
            else{
                gpio_pin_set(gpio_dev_b, LED_PIN3, 0);
            }

				
            
        k_sleep(K_MSEC(20));
		// gpio_pin_set(gpio_dev_b, LED_PIN1, 1);
        // gpio_pin_set(gpio_dev_b, LED_PIN2, 1);
        gpio_pin_set(gpio_dev_b, LED_PIN3, 1);
        // printf("LED state: %d ",gpio_pin_get(gpio_dev_b,LED_PIN1));
        
        k_sleep(K_MSEC(20));
		// gpio_pin_set(gpio_dev_b, LED_PIN1, 0);
        // gpio_pin_set(gpio_dev_b, LED_PIN2, 0);
        gpio_pin_set(gpio_dev_b, LED_PIN3, 0);
        // printf("LED state: %d\n ",gpio_pn_get(gpio_dev_b,LED_PIN1));
         
    }

    return 0;
}


int times1()
{
    const struct device *gpio_dev_a;
    const struct device *gpio_dev_b;
    //gpio_port_value_t pin_value;
    int ret;

    gpio_dev_a = DEVICE_DT_GET_OR_NULL(DT_NODELABEL(gpioa));
    gpio_dev_b = DEVICE_DT_GET_OR_NULL(DT_NODELABEL(gpiob));

    if (!gpio_dev_a || !device_is_ready(gpio_dev_a)) {
        printf("GPIO device A not found or not ready\n");
        return -ENODEV;
    }

    if (!gpio_dev_b || !device_is_ready(gpio_dev_b)) {
        printf("GPIO device B not found or not ready\n");
        return -ENODEV;
    }

    // Configure the GPIO pin for the button A as an input
    ret = gpio_pin_configure(gpio_dev_a, GPIO_PIN_A0, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printf("Error configuring GPIO pin %d: %d\n", GPIO_PIN_A1, ret);
        return ret;
    }

    // Configure the GPIO pin for the button B as an input
    ret = gpio_pin_configure(gpio_dev_b, GPIO_PIN_A1, GPIO_INPUT | GPIO_PULL_UP);
    if (ret < 0) {
        printf("Error configuring GPIO pin %d: %d\n", GPIO_PIN_A1, ret);
        return ret;
    }

    // Configure the GPIO pin for the LED as an output
    ret = gpio_pin_configure(gpio_dev_b, LED_PIN1, GPIO_OUTPUT);
    if (ret < 0) {
        printf("Error configuring GPIO pin %d: %d\n", LED_PIN1, ret);
        return ret;
    }
    ret = gpio_pin_configure(gpio_dev_b, LED_PIN2, GPIO_OUTPUT);
    if (ret < 0) {
        printf("Error configuring GPIO pin %d: %d\n", LED_PIN2, ret);
        return ret;
    }
        ret = gpio_pin_configure(gpio_dev_b, LED_PIN3, GPIO_OUTPUT);
    if (ret < 0) {
        printf("Error configuring GPIO pin %d: %d\n", LED_PIN3, ret);
        return ret;
    }
    int x;
    for(int i =0; i<6;i++) {



		x = gpio_pin_get(gpio_dev_a,GPIO_PIN_A0);
		if (ret < 0) {
			// printk("Error reading GPIOA pin 5\n");
			return 0;
		}
		// printk("GPIOA pin 5 value: %d\n", x);


        // Read the value of the GPIO pin for the button B
        ret = gpio_pin_get(gpio_dev_b, GPIO_PIN_A1);
        if (ret < 0) {
            printf("Error reading GPIO pin %d: %d\n", GPIO_PIN_A0, ret);
            return ret;
        }

            // At least one button is pressed, turn the LED on
			if (x ==0)
			{
                gpio_pin_set(gpio_dev_b, LED_PIN3, 1);
                // printf("LED state: %d ",gpio_pin_get(gpio_dev_b,LED_PIN1));
			}
            else{
                gpio_pin_set(gpio_dev_b, LED_PIN3, 0);
            }

				
            
        k_sleep(K_MSEC(20));
		// gpio_pin_set(gpio_dev_b, LED_PIN1, 1);
        gpio_pin_set(gpio_dev_b, LED_PIN2, 1);
        // gpio_pin_set(gpio_dev_b, LED_PIN3, 1);
        // printf("LED state: %d ",gpio_pin_get(gpio_dev_b,LED_PIN1));
        
        k_sleep(K_MSEC(20));
		// gpio_pin_set(gpio_dev_b, LED_PIN1, 0);
        gpio_pin_set(gpio_dev_b, LED_PIN2, 0);
        // gpio_pin_set(gpio_dev_b, LED_PIN3, 0);
        // printf("LED state: %d\n ",gpio_pn_get(gpio_dev_b,LED_PIN1));
         
    }

    return 0;
}