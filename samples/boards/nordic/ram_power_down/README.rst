.. zephyr:code-sample:: nrf-ram-power-down
   :name: nRF RAM power-down

   Power down unused non-secure RAM in System ON via the TF-M RAM-control service.

Overview
********

On nRF54L and nRF7120 the RAM power and retention control (MEMCONF) is a secure
peripheral, so a non-secure (TF-M) application cannot drive it directly. This
sample uses the TF-M RAM-control service to power down the RAM sections that lie
above the image's used RAM, keeping only the sections the application needs.

The application:

#. reads back ``MEMCONF.CONTROL`` (one bit per 32 KiB section, 1 = powered),
#. powers down the unused tail of its non-secure RAM
   (from ``_image_ram_end``, rounded up to a 32 KiB section, to the end of the
   non-secure SRAM region),
#. reads ``CONTROL`` again to show the now-cleared section bits, and
#. keeps running, proving only unused sections were powered off.

Requirements
************

A board with a Nordic SoC that supports RAM control and is built for a
non-secure (TF-M) target, for example ``nrf54l15dk/nrf54l15/cpuapp/ns`` or
``nrf7120dk/nrf7120/cpuapp/ns``.

Building and Running
********************

.. zephyr-app-commands::
   :zephyr-app: samples/boards/nordic/ram_power_down
   :board: nrf54l15dk/nrf54l15/cpuapp/ns
   :goals: build flash
   :compact:

Sample Output
=============

.. code-block:: console

   *** Booting Zephyr OS ... ***
   nrf54l15dk@... RAM power-down demo
   NS RAM 0x20020000..0x20040000, image uses up to 0x200210xx
   MEMCONF CONTROL before: 0x000000ff
   Power down 0x20028000..0x20040000 (96 KiB): ok
   MEMCONF CONTROL after: 0x0000001f
   Running with unused RAM powered off.
