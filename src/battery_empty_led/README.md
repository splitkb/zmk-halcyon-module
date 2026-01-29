# Battery empty LED

Blinks the specified LED the number of times specified in bursts when the battery percentage is the threshold or lower.

Devicetree example:
```
/ {
    battery_empty_led: battery_empty_led {
        compatible = "splitkb,battery-empty-led";
        status = "okay";
        battery_threshold_percentage = <20>;
        blink_duration_ms = <50>;
        blink_interval_ms = <2000>;
        num_blinks = <20>;
        led = <&blue_led>;
    };
};
```
