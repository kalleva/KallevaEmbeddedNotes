#Note008 Cheap Way To Measure Power Consumption Of An IoT Device

Power consumption is one of main parameters that are critical for an IoT device.
It really makes a big difference if a device can live on the same battery for a month or 5 years.

Usually IoT device spends most of its life some kind of sleep or standby mode.
And in this mode current it draws from the battery should be really low, usually on the order of microamperes.
When designing and writing firmware for such device extra care is needed to get device really consume as little current as possible.
Generally you can test how much power device is consuming by hooking it to special power source that will log device's power consumption.
But you don't always have such laboratory device at your disposal, so you can try to insert amperemeter in your circuit to look for a current that device draws. However some IoT devices have particular power consumption profiles.
They stay in the stop mode, drawing almost no current, but then they wake up, make some measurement with attached sensor and send obtained value to the server via LoRa or NBIoT. And when device send message using these technologies it starts to consume orders of magnitude more current (50 - 100mA).
And not all amperemeters can handle this sudden hike in current.
Usually they have several shunts hooked up in parallel and just switch between them when they are going from one measurements range to another.
This switching can be somewhat slow or involve breaking internal amperemeter circuit to switch to another shunt.
And device under test can be affected by this process, because it is unable to get enough current fast enough.
And there is simple and cheap way to get around of this using voltmeter, 1kOhm resistor and Schottky diode.



Connect this Shottky diode in forward direction to the power source and in parallel with diode connect 1kOhm resistor.
When voltage drop on the resistor is small (less than 0.25-0.35V depending on particular Shottky diode) all current goes through the shunt.
So you can take voltmeter and measure voltage drop on the shunt and through the Ohm's Law deduce the current.
With 1kOhm resistor, 1mV means that there is 1uA current flowing through the shunt.
When device under test starts to consume more power voltage drop on the shunt is increased beyond diode's forward voltage threshold, diode opens and allows current to flow through it.
Shottky diode opens really fast (time can be down to picoseconds) device under test doesn't notice this and continues its operation normally.
Of course you cannot measure consumption during that phase, because your voltmeter will just show you the constant voltage of forward potential barrier of the diode. But when device will go back to the stop/standby mode voltage drop on shunt will drop below forward potential barrier of the diode and all current will again flow through the shunt and you will be able to observe if device is really consuming as little power as you thought it should.
