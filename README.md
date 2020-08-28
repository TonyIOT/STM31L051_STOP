# STM31L051_STOP

STM32L051 has excellent performance in terms of low power consumption. According to the official DATA manual of ST, the current consumption can reach 0.4uA in stop without RTC mode and 0.8uA in stop with RTC mode.

The code consists of two parts:
1. Stop with RTC,wake up with RTC.
2. Stop without RTC,wake up with external interrupts, such as keystrokes and serial ports to receive external data.

## 1. Stop With RTC

1. Set the external setting before entering STOP mode, and the GPIO is set to the analog input state.
2. Set the wake-up time of the RTC in seconds.
3. Call the library function into STOP mode.
4. Restore the clock after quitting STOP mode.Note in particular that the system clock becomes MSI when you enter STOP mode.
5. Restore peripheral states, such as GPIO, serial port initialization.
6. Call the function directly where you need to enter STOP mode.

[Details accessible CSDN](https://blog.csdn.net/TonyIOT/article/details/99627621)

## 2. Stop Without RTC 

1. Before entering the STOP mode to set the external device, the GPIO should be set to the simulated input state, cancel the serial port configuration to restore the default.
2. Set the wake-up source interrupt, where keystrokes and serial rX-associated interrupts are configured.
3. Call the library function into STOP mode.
4. Restore the clock after quitting STOP mode.Note in particular that the system clock becomes MSI when you enter STOP mode.
5. Restore peripheral states, such as GPIO, serial port initialization.
6. Call the function directly where you need to enter STOP mode.

[Details accessible CSDN](https://blog.csdn.net/TonyIOT/article/details/99636969)
