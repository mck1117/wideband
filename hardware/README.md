## `board_module`

Wideband O2 sensor controller on a postage stamp. Contains all the analog and digital control logic to handle a Bosch LSU 4.2, 4.9, or ADV sensor.

Intended to either be mounted on an ECU as a module, or on `board_carrier` to run standalone.

## `board_carrier`

Stacks under `board_module`, has 5v regulator, CAN tranceiver, etc so that the module can exist standlone.
