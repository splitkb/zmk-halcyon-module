# Charging IC LED

Sets the specified LED to `ON` when a charger is connected. This is done by reading the STAT pin from a charging IC.

Devicetree example:
```
/ {
    charging_led_controller {
        compatible = "splitkb,charging-ic-led";
        status = "okay";
        stat-gpios = <&gpio1 6 (GPIO_ACTIVE_LOW | GPIO_PULL_UP)>;
        stat_assert_delay_ms = <600>;
        led = <&orange_led>; 
    };
};
```
