rpi_blink_ip
============

Small C program that will blink one of the onboard LEDs on a Raspberry Pi, signaling its IPv4 address

Building
============

  - Run `make` to build a binary called "blink" that will work on your Raspberry Pi
  - Run `make debug` to build a binary that will print to stdout
  
Making it useful
================
1. Install netplugd, a daemon that monitors the status available network interfaces
  - `sudo apt-get install netplugd`
  - This should add the necessary init and rc scripts so that the service is launched on startup
2. Start netplugd, if it's not already: `service netplug start`
3. Copy the "blink" binary to some logical place: `cp blink /etc/netplug/`
4. Edit the netplug "action" script, which is called when the status of a network interface changes:

`vim /etc/netplug/netplug`

Look for a section like this:

```shell
case "$action" in
in)
    if [ -x /sbin/ifup ]; then
        exec /sbin/ifup "$dev"
```
change this line: `exec /sbin/ifup "$dev"` to this:

```shell
    /sbin/ifup "$dev" && /etc/netplug/blink
```

Now the blink program will execute when you plug in a network cable.

