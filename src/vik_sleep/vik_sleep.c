/*
 * Copyright (c) 2026 Splitkb.com
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT splitkb_vik_sleep

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/init.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/activity.h>
#include <zmk/events/activity_state_changed.h>
#include <zmk/event_manager.h>

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

#define GET_SPEC_AND_COMMA(node_id, prop, idx) GPIO_DT_SPEC_GET_BY_IDX(node_id, prop, idx),

#define EXTRACT_CHILD_OUTPUTS(child_node)                                                          \
    COND_CODE_1(DT_NODE_HAS_PROP(child_node, output_gpios),                                        \
                (DT_FOREACH_PROP_ELEM(child_node, output_gpios, GET_SPEC_AND_COMMA)), ())

#define EXTRACT_CHILD_INPUTS(child_node)                                                           \
    COND_CODE_1(DT_NODE_HAS_PROP(child_node, input_gpios),                                         \
                (DT_FOREACH_PROP_ELEM(child_node, input_gpios, GET_SPEC_AND_COMMA)), ())

static const struct gpio_dt_spec outputs[] = {DT_INST_FOREACH_CHILD(0, EXTRACT_CHILD_OUTPUTS)};
static const struct gpio_dt_spec inputs[] = {DT_INST_FOREACH_CHILD(0, EXTRACT_CHILD_INPUTS)};
static const struct gpio_dt_spec ctrl = GPIO_DT_SPEC_INST_GET_OR(0, control_gpios, {0});

static void configure_pins(const struct gpio_dt_spec *pins, size_t count, gpio_flags_t flags) {
    for (size_t i = 0; i < count; i++) {
        if (pins[i].port) {
            gpio_pin_configure_dt(&pins[i], flags);
        }
    }
}

int vik_sleep_event_handler(const zmk_event_t *eh) {
    struct zmk_activity_state_changed *ev = as_zmk_activity_state_changed(eh);
    if (ev == NULL) {
        return -ENOTSUP;
    }

    switch (ev->state) {
    case ZMK_ACTIVITY_ACTIVE:
    case ZMK_ACTIVITY_IDLE:
        if (ctrl.port) {
            gpio_pin_configure_dt(&ctrl, GPIO_OUTPUT_ACTIVE);
        }
        configure_pins(outputs, ARRAY_SIZE(outputs), GPIO_OUTPUT_INACTIVE);
        configure_pins(inputs, ARRAY_SIZE(inputs), GPIO_INPUT);
        break;

    case ZMK_ACTIVITY_SLEEP:
        if (ctrl.port) {
            gpio_pin_configure_dt(&ctrl, GPIO_INPUT | GPIO_PULL_DOWN);
        }
        configure_pins(outputs, ARRAY_SIZE(outputs), GPIO_INPUT);
        configure_pins(inputs, ARRAY_SIZE(inputs), GPIO_INPUT);
        break;

    default:
        LOG_WRN("Unhandled activity state: %d", ev->state);
        return -EINVAL;
    }
    return 0;
}

ZMK_LISTENER(vik_sleep_listener, vik_sleep_event_handler);
ZMK_SUBSCRIPTION(vik_sleep_listener, zmk_activity_state_changed);

static int vik_sleep_init(void) {
    if (ctrl.port && !device_is_ready(ctrl.port)) {
        return -ENODEV;
    }
    
    if (ctrl.port) {
        gpio_pin_configure_dt(&ctrl, GPIO_OUTPUT_ACTIVE);
    }
    return 0;
}

SYS_INIT(vik_sleep_init, POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */
