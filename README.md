# SCARD_On_MCU

## Introduction

PC/SC is the de-facto standard to make an application running on a high-end computer (say, a PC) interact with a smart card (SC).

SpringCard offers numerous USB products that comes with a PC/SC compliant driver for Windows. The same products may also be used under Linux and MacOS X thanks to the PCSC-Lite open-source project, and a equivalent stack is available for Android.

SpringCard also offers serial-based contactless smart card couplers (for instance, the SpringCard K663 family http://www.springcard.com/en/products/k663-ant.html) that are typically operated from microcontrollers (MCU) with strong memory and performance constraints.

The SCARD_On_MCU library aims to fill-in the gap between high-end systems and low-end MCUs.

## Compliant products

Every SpringCard coupler in the K663 family running a firmware version >= 2.02 is supported by the SCARD_On_MCU library.

Earlier version of K663 could be upgraded.

K531 and K632 are not supported.

## API Reference

The .html files within the /docs folder are your primary source of information.

## Sample

The /src/main.c file shows how to wait for a contactless smart card, get some data from it, and wait again until it is removed.

It also illustrates how to perform various actions on the K663 coupler (change runtime configuration, drive the LEDs etc).

## Porting the library to your MCU

Use the /src/hal/hal_skel_mcu.c file as reference.

Your hardware-abstraction layer (HAL) must provide 

### Serial communication with the K663.

The serial_init function shall configure the UART (38400bps, 8 data bits, 1 stop bit, no parity, no flow control).

The serial_send_byte and serial_send_bytes are used to transmit (TX) from the MCU to the K663.

Receiving (RX) must be done in an ISR. The ISR shall call serial_recv_callback for every byte that comes from the K663 to the MCU.
  
### Wait for the end of the communication.

ccid_recv_wait will be called in the context of the main task, and shall block until the ISR calls ccid_recv_wakeupFromISR (this is done by serial_recv_callback).

If you don't have an OS, just use a volatile BOOL to do so.

Under a multitasking system, you must use a 'Event' or 'Semaphore' object to implement this correctly.

Under FreeRTOS (http://www.freertos.org) you will typically use a Binary Semaphore as follow:
- ccid_recv_wait blocks using xSemaphoreTake
- ccid_recv_wakeupFromISR calls xSemaphoreGiveFromISR
- you may create the semaphore in the platform_init function.

### Provide a mean to wait for a specified time, from 1 to 10000ms. This is the role of sleep_ms.

## License

This library is Copyright (c) 2015 SPRINGCARD SAS, FRANCE - http://www.springcard.com

Permission is given to embed it in your own MCU (or even PC-based) projects, provided that this project is always used in conjunction with a genuine SpringCard product.

Please read the LICENSE.txt file for details.

