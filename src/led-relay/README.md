LED and Relay Demo Application
------------------------------

The `led-relay` application demonstrates how to flash the LEDs on a
device and how to enable or disable the bypass relays.


Limitations
-----------

From the WeOS host system the path `/sys/class/leds` is shared with the
container application when LXC starts the container.  This results in a
limitation that access to physical LEDs and relays are not atomic and no
synchronization is done between containers or Host.  All container apps
having a shared path to the device file can alter any LED.

Usually this is not a problem, since a set of ports, e.g. relay controlled
ports, are at the same time "shared" with the container application.  The
ports behave a bit differently, compared to files, and are actually moved
to the container.  Being invisible to both the host system and any other
container application means other WeOS functions cannot use these ports,
and will also not be interested in the accompanying LEDs and relays.


Hardware Control
----------------

In the file system path `/sys/class/` we can see hardware on the device
we can interact with.  Interaction is done by reading and writing to
device files:

```sh
admin@viper-65-f7-80:/sys/class # ls
bdi           i2c-dev       net           rtc           usbmon
block         input         pci_bus       scsi_device   vc
bsg           iommu         phy           scsi_disk     virtio-ports
dma           leds          power_supply  scsi_generic  vtconsole
fm0           mdio_bus      ppp           scsi_host     watchdog
gpio          mem           pps           thermal       xt_idletimer
hwmon         misc          rapidio_port  tty
i2c-adapter   mtd           relay-ctrl    udc
```

In the `/sys/class/leds/` directory we find all system LEDs:

```sh
admin@viper-65-f7-80:/sys/class/leds # ls
bypass1:green       ethX4:green:link    gnss:green
bypass1:red         ethX4:yellow:state  gnss:red
bypass2:green       ethX5:green:link    viper:green:frnt
bypass2:red         ethX5:yellow:state  viper:green:on
ethX1:green:link    ethX6:green:link    viper:green:rstp
ethX1:yellow:state  ethX6:yellow:state  viper:red:frnt
ethX2:green:link    ethX7:green:link    viper:red:on
ethX2:yellow:state  ethX7:yellow:state  viper:red:rstp
ethX3:green:link    ethX8:green:link
ethX3:yellow:state  ethX8:yellow:state
```

Each entry is a directory with files for controlling an LED.

```sh
admin@viper-65-f7-80:/sys/class/leds # ls ethX1\:green\:link/
brightness      max_brightness  subsystem       uevent
device          power           trigger
```

For example, in the `max_brightness` file we find the value 255, and if
the device has a LED supporting different brightness levels we can write
a value between 0 and 255 to the brightness file.  If the LED does not
support brightness levels every value greater than 0 sets the led.

One other option is the `trigger`.  Reading the `trigger` file we find
a the list of supported values to set, with brackets `[..]` surrounding
the currectly set value.  Here `[none]` is the current value.

```sh
admin@viper-65-f7-80:/sys/class/leds # cat ethX1\:green\:link/trigger
[none] kbd-scrolllock kbd-numlock kbd-capslock kbd-kanalock kbd-shiftlock
kbd-altgrlock kbd-ctrllock kbd-altlock kbd-shiftllock kbd-shiftrlock
kbd-ctrlllock kbd-ctrlrlock dc1-online dc2-online timer ethX1-stp
ethX2-stp ethX3-stp ethX4-stp ethX5-stp ethX6-stp ethX7-stp ethX8-stp
bypass-relay
```

The bypass relay(s) are supported in the same way.  We can control them
writing to the device file `/sys/class/relay-ctrl/bypass-relay/value`.
Writing "2" to this file disables the bypass relays and writing a "1"
enables it.


Setting Up the Demo Application
-------------------------------

To create and configure a `led-relay` container application in WeOS,
follow these steps:

```sh
viper-65-f7-e0:/#> configure
viper-65-f7-e0:/config/#> app led-relay
viper-65-f7-e0:/config/app-led-relay/#> sh
Status       : Enabled
Name         : led-relay
Description  : 
Image        : led-relay
Init cmd     : /sbin/init
Loglevel     : 5
                                                                              
Shared Resources
TYPE  HOST              GUEST             OPTS
veth  led-relay         eth0
```

By default all container applications have a veth pair created that allows 
them to connect to the Host. 

To set up a shared file system resource between the Host system and the
container we use the `share` command.  Symlinks in `/sys/class/leds/`
link to files in `/sys/devices/`, which means we have to share the full
`/sys` path with the container.

```sh
viper-65-f7-e0:/config/app-led-relay/#> share path /sys as /sys
viper-65-f7-e0:/config/app-led-relay/#> sh
Status       : Enabled
Name         : led-relay
Description  : 
Image        : led-relay
Init cmd     : /sbin/init
Loglevel     : 5
                                                                              
Shared Resources
TYPE  HOST              GUEST             OPTS
path  /sys              /sys
veth  led-relay         eth0
```

Leave configuration and save running configuration to start
configuration.

```sh
viper-65-f7-e0:/config/app-led-relay/#> leave
viper-65-f7-80:/#> copy run start
```

Upon leaving the configuration context, WeOS starts the container.  To
list all started containers use:

```sh
viper-65-f7-80:/#> show app
NAME       STATE   AUTOSTART GROUPS IPV4 IPV6 UNPRIVILEGED
led-relay  RUNNING 1         -      -    -    false
```

The command `app attach led-relay` lets us attach to the container and
run commands in it. *The username to login is `root`, no password set up
by default.*

Run the `led-relay` application with flag `-h` to list options:

```sh
Usage: led-relay [OPTIONS]
  -b VAL   Disable(2) or Enable(1) bypass relays
  -c LED   Clear LED or relay value to 0
  -f LED   Start flashing LED
  -g LED   Get LED or relay value
  -h       Show this help text
  -s LED   Set LED or relay on
  -v       Show program version

Examples:
  Flash LED      : led-relay -f ethX5:yellow:state
  Set LED        : led-relay -s ethX6:yellow:state
  Clear LED      : led-relay -c ethX5:yellow:state
  Disable bypass : led-relay -b 2
  Enable bypass  : led-relay -b 1
```

**Example:**

```sh
~# led-relay -s ethX8:yellow:state
```
