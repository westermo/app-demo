# Leds application

The `Leds` application demonstrates how to read and write to the leds device file to control the physical leds on a device.

## Limitations

From the `Host` a path is shared to the `led device file` for a physical port and the path is then mounted in the container when LXC starts the container. 
This results in the limitation that access to physical leds is not atomic and no synchronization is done between containers or Host. All container app's having a shared path to the device file can alter its value.


## Configuration

On the `Host` symlinks are defined for the device files in `/sys/class/leds/` directory. To create and configure a container application `bar` of image type `leds` follow these steps:


```bash
viper-65-f7-e0:/#> config app bar leds
viper-65-f7-e0:/config/app-bar/#> sh
Status       : Enabled
Name         : bar
Description  : 
Image        : leds
Init cmd     : /sbin/init
Loglevel     : 5
                                                                              
Shared Resources
TYPE  HOST              GUEST             OPTS                                
veth  bar               eth0 
```

By default all container applications have a veth pair created that allows them to connect to the Host. To setup a shared path between the Host and the container we use the `shared` command. Symlinks in /sys/class/leds/ are linking to files in /sys/devices/ and this means that we have to share the /sys path between Host and container.

```bash
viper-65-f7-e0:/config/app-bar/#> share path /sys as /sys
viper-65-f7-e0:/config/app-bar/#> sh
Status       : Enabled
Name         : bar
Description  : 
Image        : leds
Init cmd     : /sbin/init
Loglevel     : 5
                                                                              
Shared Resources
TYPE  HOST              GUEST             OPTS                                
path  /sys              /sys
veth  bar               eth0  
```

Leave configuration and save running configuration to start configuration.

When leaving configuration WeOS starts the container and to list all started containers use:
```
viper-65-f7-80:/#> sh app
NAME STATE   AUTOSTART GROUPS IPV4 IPV6 UNPRIVILEGED                          
bar  RUNNING 1         -      -    -    false
```

The command `app attach <container>` lets us attach to the container and run commands in it. *(user name is root)*

Run the `Leds` application with flag -h to list options:

```
Usage: leds [OPTIONS]

  -h         Show summary of command line options and exit
  -p         Print interfaces
  -i         Start flashing interface led every -t sec
  -t         Flash frequency in seconds, default is 2.0 sec
  -1         Set led ON
  -0         Set led OFF

Example usage:
Flash led:   leds -i ethX5:yellow:state -t 2.0
Set led ON:  leds -1 ethX5:yellow:state

```

## Executing a command in the container from the Host
From the Host shell command line we can execute a command inside the container shell using lxc-attach.

Example:
```
lxc-attach -n bar -- leds -1 ethX8:yellow:state
```