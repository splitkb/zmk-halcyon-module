/*
 * Copyright (c) 2026 Splitkb.com
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT splitkb_battery_empty_led

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(battery_empty_led, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/battery.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_INST_PHANDLE(0, led), gpios);
static const uint8_t battery_threshold = DT_INST_PROP(0, battery_threshold_percentage);
static const uint32_t blink_duration = DT_INST_PROP(0, blink_duration_ms);
static const uint32_t blink_interval = DT_INST_PROP(0, blink_interval_ms);
static const uint32_t max_blinks = DT_INST_PROP(0, num_blinks);

static struct k_work_delayable blink_work;
static uint8_t blinks_remaining = 0;
static bool is_low_battery = false;
static bool led_is_on = false;

static void blink_work_handler(struct k_work *work) {
    if (led_is_on) {
        gpio_pin_set_dt(&led, 0);
        led_is_on = false;

        if (blinks_remaining > 0) {
            k_work_schedule(&blink_work, K_MSEC(blink_interval));
        }
    } else {
        if (blinks_remaining > 0) {
            gpio_pin_set_dt(&led, 1);
            led_is_on = true;
            blinks_remaining--;
            k_work_schedule(&blink_work, K_MSEC(blink_duration));
        }
    }
}

static int battery_listener(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);
    if (ev == NULL) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    if (ev->state_of_charge <= battery_threshold) {
        if (!is_low_battery) {
            is_low_battery = true;
            blinks_remaining = max_blinks;
            k_work_schedule(&blink_work, K_NO_WAIT);
        }
    } else {
        if (is_low_battery) {
            is_low_battery = false;
            k_work_cancel_delayable(&blink_work);
            gpio_pin_set_dt(&led, 0);
            led_is_on = false;
        }
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(battery_empty_led, battery_listener);
ZMK_SUBSCRIPTION(battery_empty_led, zmk_battery_state_changed);

static int btr_init(void) {
    if (!gpio_is_ready_dt(&led)) {
        LOG_ERR("LED GPIO device not ready");
        return -ENODEV;
    }

    int ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure LED GPIO");
        return ret;
    }

    k_work_init_delayable(&blink_work, blink_work_handler);

    uint8_t level = zmk_battery_state_of_charge();
    if (level <= battery_threshold) {
        is_low_battery = true;
        blinks_remaining = max_blinks;
        k_work_schedule(&blink_work, K_NO_WAIT);
    }

    return 0;
}

SYS_INIT(btr_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
