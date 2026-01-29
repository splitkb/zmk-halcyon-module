# Splitkb.com ZMK sleep driver

Splitkb.com wireless Halcyon controller has a seperate net to turn on or off the 3V3 on the VIK connector. As ZMK, as of 2026-07-01, does not have support for multiple externel power nets, this adds a manual function to enable or disable the VIK power net when the board goes to sleep.

This can be added using the following devicetree options:
```
/ {
    vik_sleep: vik_sleep_mgr {
        compatible = "splitkb,vik-sleep";
        status = "okay";
        control-gpios = <&halcyon_conn 33 GPIO_ACTIVE_HIGH>; 
    };
};
```

As only shutting down the nets does not completely stop all current draw, some specific `CS`, `BUSY`, `DC` or `RESET` pins also have to be "disabled". In your shield add another node:
```
&vik_sleep {
    epaper_pins {
        output-gpios = <&vik_conn 6 GPIO_ACTIVE_LOW>,  // CS
		               <&vik_conn 3 GPIO_ACTIVE_LOW>,  // RESET
					   <&vik_conn 0 GPIO_ACTIVE_HIGH>; // DC
        input-gpios = <&vik_conn 5 GPIO_ACTIVE_HIGH>;  // BUSY
    };
};
```
