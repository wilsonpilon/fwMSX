Pull-up Resistor Test Tool by KdL (2021.10.16) / TEST IN A,(P)
==============================================================

This tool is designed to perform latency diagnostics on the data bus of external slots.
It specifically indicates the need to resize the pull-ups connected to the data bus in
case a large number of unused ports consecutively emit an error message.
Some single errors are considered normal because it is not possible to diagnose the
responsiveness of the pull-ups if the ports are occupied by internal circuits.
This tool is particularly suitable for the first version of the SX-2 and the prototypes.

In case you want to preserve the original size of the pull-ups, it will be possible to
replace the DC-DC power converter with a real power supply that can compensate for
voltage drops and provide the necessary current (mod by GodzIvan).

------------------------------------------
Usage:  RUN "TESTINAP.ASC"  from MSX-BASIC
------------------------------------------


______
Enjoy!
