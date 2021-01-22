# DS1820

Try to read a DS1820 1-wire temperature sensor.

## Presence

Using Poor-Mans's Logic Analyzer:

    ..., 1
    057, 0
    583, 1
    610, 0
    714, 1

The master sets line down for 500us, then up again. After 30us slave takes the bus and keeps it down for 100us.
