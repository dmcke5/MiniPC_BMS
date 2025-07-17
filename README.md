# MiniPC BMS
A tiny UPS for powering Mini PC's from battery!

 ![Alt text](PowerManagement.png "PMS")

## Features:
- USB-C PD charging
- Secondary charger input
- USB-HID UPS communication to provide the operating system with battery information
- Inbuilt fuel gauge IC for accurate battery capacity monitoring
- Extremely low powered-down power consumption (5.1uA)
- Inbuilt 19v boost for stable output at up to 3A.
- Solid state switch for triggering PC power button

## Software:
The software for this project is based on this HID power device library:
https://github.com/abratchik/HIDPowerDevice

The software is complete and functional, although it could use some polish and some comments.
I would recommend changing the "debug" variable in MiniPC_UPS.ino to true before you flash the code for the first time.
This will enable debug output and stop the BMS from trying to shut your computer down due to a "Low battery".
Once you've confirmed all of the values are being read correctly, you should be safe to go back in and set that variable to false
and the BMS will begin functioning.

## Known issues

USB-C PD charging is slow. I found a small issue in the PCB design (which I have now fixed) before uploading that may or may not have solved this problem. I suspect that the issues I've had with this are just down to soldering or faulty hardware because the second charger input works as intended and it is basically identical to the first one after all of the USB PD components. I have checked and the correct values are being requested from the USB charger so the issue has somewhere down the line from the USB PD hardware. This was working for me at one stage, which is why I suspect its an assembly issue.

USB HID is sometimes not detected by windows. This one has been difficult to track down, but I'm certain its a software issue that's causing the problem. A simple restart of the system always seems to rectify the problem so I've left it alone for now.
