/*
 * Copyright (c) 2026 Splitkb.com
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT splitkb_charging_ic_led

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(charging_led, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/usb.h>
#include <zmk/event_manager.h>
#include <zmk/events/usb_conn_state_changed.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static const struct gpio_dt_spec stat = GPIO_DT_SPEC_INST_GET(0, stat_gpios);
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_INST_PHANDLE(0, led), gpios);
static const uint32_t stat_assert_delay = DT_INST_PROP(0, stat_assert_delay_ms);

static struct gpio_callback stat_cb;
static struct k_work_delayable work;
static bool usb_active = false;

static void work_handler(struct k_work *w) {
    if (!usb_active) {
        gpio_pin_set_dt(&led, 0);
        return;
    }

    int stat_val = gpio_pin_get_dt(&stat);
    if (stat_val >= 0) {
        gpio_pin_set_dt(&led, stat_val);
    }
}

static void stat_isr(const struct device *port, struct gpio_callback *cb, uint32_t pins) {
    int is_charging = gpio_pin_get_dt(&stat);
    k_work_reschedule(&work, is_charging ? K_MSEC(stat_assert_delay) : K_NO_WAIT);
}

static void update_monitoring_state(void) {
    bool usb_present = zmk_usb_is_powered();

    if (usb_present == usb_active) {
        return;
    }

    usb_active = usb_present;

    if (usb_present) {
        gpio_pin_configure_dt(&stat, GPIO_INPUT | GPIO_PULL_UP);
        gpio_pin_interrupt_configure_dt(&stat, GPIO_INT_EDGE_BOTH);
        k_work_reschedule(&work, K_NO_WAIT);
    } else {
        gpio_pin_interrupt_configure_dt(&stat, GPIO_INT_DISABLE);
        gpio_pin_configure_dt(&stat, GPIO_DISCONNECTED);
        k_work_cancel_delayable(&work);
        gpio_pin_set_dt(&led, 0);
    }
}

static int usb_listener(const zmk_event_t *eh) {
    update_monitoring_state();
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(charging_led_listener, usb_listener);
ZMK_SUBSCRIPTION(charging_led_listener, zmk_usb_conn_state_changed);

static int chg_init(void) {
    if (!gpio_is_ready_dt(&led) || !gpio_is_ready_dt(&stat)) {
        LOG_ERR("Charging LED GPIO devices not ready");
        return -ENODEV;
    }

    gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);

    gpio_init_callback(&stat_cb, stat_isr, BIT(stat.pin));
    gpio_add_callback(stat.port, &stat_cb);

    k_work_init_delayable(&work, work_handler);

    update_monitoring_state();

    return 0;
}

SYS_INIT(chg_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
