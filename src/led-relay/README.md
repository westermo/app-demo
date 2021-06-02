# LED and relay demo application

The `led-relay` application demonstrates how to flash the LEDs on a device
and how to enable or disable the bypass relays.

## Limitations

From the `WeOS host system` a path is shared to the `led device file` for a 
physical port and the path is then mounted in the container when LXC starts 
the container. 
This results in the limitation that access to physical leds and relays are not 
atomic and no synchronization is done between containers or Host. 
All container app's having a shared path to the device file can alter its value.

## Hardware control
In the file system path `/sys/class/` we can see hardware on the device we 
can interact with. Interaction is done by reading and writing to device files.

```bash
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

Looking inside the `/sys/class/leds/` directory we find all ports on the 
device represented as a directory:
```bash
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

Inside a ports directory we see the files for controlling its LED.

```bash
admin@viper-65-f7-80:/sys/class/leds # ls ethX1\:green\:link/
brightness      max_brightness  subsystem       uevent
device          power           trigger
```

For example if we look in the max_brightness file we find the value 255, and 
if the device has a led supporting different brightness levels we cat write 
a value between 0 and 255 to the brightness file. 
If the led does not support brightness levels every value greater 
than 0 will set the led.

One other option is the trigger. 
If looking in the trigger file we find a list of values to set, and `[none]` 
is the current value.
```bash
admin@viper-65-f7-80:/sys/class/leds # cat ethX1\:green\:link/trigger 
[none] kbd-scrolllock kbd-numlock kbd-capslock kbd-kanalock kbd-shiftlock kbd-altgrlock kbd-ctrllock kbd-altlock kbd-shiftllock kbd-shiftrlock kbd-ctrlllock kbd-ctrlrlock dc1-online dc2-online timer ethX1-stp ethX2-stp ethX3-stp ethX4-stp ethX5-stp ethX6-stp ethX7-stp ethX8-stp bypass-relay
```
In the same way we can control the bypass relays by writing to the device file 
`/sys/class/relay-ctrl/bypass-relay/value`.
Writing "2" to this file disables the bypass relays and writing a "1" enables.

## Configuration of demo application

To create and configure a container application `bar` of image 
type `led-relay` follow these steps:


```bash
viper-65-f7-e0:/#> config app bar led-relay

viper-65-f7-e0:/config/app-bar/#> sh
Status       : Enabled
Name         : bar
Description  : 
Image        : led-relay
Init cmd     : /sbin/init
Loglevel     : 5
                                                                              
Shared Resources
TYPE  HOST              GUEST             OPTS                                
veth  bar               eth0 
```

By default all container applications have a veth pair created that allows 
them to connect to the Host. 
To setup a shared path between the Host and the container we use 
the `shared` command. 
Symlinks in /sys/class/leds/ are linking to files in /sys/devices/ and this 
means that we have to share the /sys path between Host and container.

```bash
viper-65-f7-e0:/config/app-bar/#> share path /sys as /sys

viper-65-f7-e0:/config/app-bar/#> sh
Status       : Enabled
Name         : bar
Description  : 
Image        : led-relay
Init cmd     : /sbin/init
Loglevel     : 5
                                                                              
Shared Resources
TYPE  HOST              GUEST             OPTS                                
path  /sys              /sys
veth  bar               eth0  
```

Leave configuration and save running configuration to start configuration.

When leaving configuration WeOS starts the container and to list all 
started containers use:
```
viper-65-f7-80:/#> sh app
NAME STATE   AUTOSTART GROUPS IPV4 IPV6 UNPRIVILEGED                          
bar  RUNNING 1         -      -    -    false
```

The command `app attach <container>` lets us attach to the container and 
run commands in it. *(user name is root)*

Run the `led-relay` application with flag -h to list options:

```
Usage: led-relay [OPTIONS]
  -b         Disable(2) or Enable(1) bypass relays
  -c         Clear LED or relay value to 0
  -f         Start flashing LED
  -g         Get LED or relay value
  -h         Show summary of command line options and exit
  -s         Set LED or relay on
  -v         Show program version

Example usage:
Flash led:   led-relay -f ethX5:yellow:state
Set led ON:  led-relay -s ethX6:yellow:state
Clear led:   led-relay -c ethX5:yellow:state
Disable bypass: led-relay -b 2
Enable bypass:  led-relay -b 1
```

## Executing a command in the container from the Host
From the Host shell command line we can execute a command inside the 
container shell using lxc-attach.

Example:
```
lxc-attach -n bar -- led-relay -s ethX8:yellow:state
```
