WeOS Application Demos
======================

This repository is the starting point for 3rd party developers looking
to deploy their applications as containers in the Westermo [WeOS][]
operating system.

In order for developers to easily access Westermo specific features it
is recommended that your application, just like app-demo, is built on
top of [NetBox][], which is where all such features and patches to any
Open Source package are published.

NetBox extends [Buildroot][] to provide a solid build system and recipes
for building thousands of open source packages, handling all intricacies
of dependency management, cross-compilation etc.  All these packages are
available for use in both NetBox and app-demo.

![Buildroot External Trees](doc/externals.svg)

The app-demo project is layered on top of NetBox, the same way NetBox is
layered on top of Buildroot, using the Buildroot [External Tree][]
facility.


Supported Platforms
-------------------

Like NetBox, this project supports the following Westermo platforms out
of the box, other variants are possible, but require custom setup:

| **Architecture** | **Platform** |
|------------------|--------------|
| PowerPC (T1023)  | Coronet      |
| ARM Cortex-A9    | Dagger       |
| Intel/AMD x86_64 | Zero         |


Available Demos
---------------

| **Description**   | **Coronet**           | **Dagger**           | **Zero**           | **SHA1 Checksum**  |
|-------------------|-----------------------|----------------------|--------------------|--------------------|
| [Backbone][]  App | [backbone-coronet][]  | [backbone-dagger][]  | [backbone-zero][]  | [backbone.sha1][]  |
| [DHCP Boot][] App | [dhcp-boot-coronet][] | [dhcp-boot-dagger][] | [dhcp-boot-zero][] | [dhcp-boot.sha1][] |
| [LED/Relay][] App | [led-relay-coronet][] | [led-relay-dagger][] | [led-relay-zero][] | [led-relay.sha1][] |

The default login credentials for all demo apps are `root` without any
password.  To enable remote login using SSH, you need to set a password
or configure the Dropbear SSH daemon to allow blank passwords.  Another
possibility is to user SSH keys.

> **Note:** Currently the nightly builds and defconfigs available here
>           target only the _Coronet_, _Dagger_ and _Zero_ platforms.
>           Other platforms will be supported later.  For details, see
>           the NetBox project.


Building a Demo
---------------

The [Buildroot Manual][] is very comprehensive and a great resource for
learning about the build system. In addition, running `make help` in the
app-demo root directory will show a summary of the most commonly used
commands.

In order to build a container image, the repository must first be
configured. To list the available configuration targets, run

    make list-defconfigs

To select, for example, the `backbone` demo, run

    make backbone_coronet_defconfig
	
A curses based configuration interface can be summoned with `make
menuconfig` to select any additional packages to build.

Finally we can build the image using `make`. Expect an initial build
to take around 15 minutes on a reasonably modern machine; subsequent
incremental builds are much faster.

Once the build has completed, your application image is available in
`output/images/rootfs.squashfs`.


Repository Layout
-----------------

The app-demo project follows the [Example layout][] proposed in the
Buildroot manual.

- `board/demo/${TARGET}`: Customizations for each target
- `configs/`: Default configurations
- `netbox/`: Git submodule pointing to the netbox repo which in turn
  contains a submodule reference to Buildroot
- `output/`: All generated artifacts are stored in this here
- `package/`: Recipes for building some piece of software, either by
  downloading a tarball or by referencing a directory in `src/`
- `src/`: Source code for sample applications developed by Westermo

This is an _example_ of how an external Buildroot tree can be set up and
structured, most of it can be changed to better suit the needs of both
the developer and organization, if required.  In fact, WeOS makes no
assumptions on the application container even using Buildroot at all;
you are free to use any build system as long as the result is a SquashFS
image containing an executable `/sbin/init` which is compatible with the
target architecture.

[WeOS]:              https://www.westermo.com/solutions/weos
[NetBox]:            https://github.com/westermo/netbox
[Buildroot]:         https://buildroot.org
[Buildroot Manual]:  https://buildroot.org/downloads/manual/manual.html
[External Tree]:     https://buildroot.org/downloads/manual/manual.html#outside-br-custom
[Example layout]:    https://buildroot.org/downloads/manual/manual.html#_example_layout
[Backbone]:          src/backbone/README.md
[DHCP Boot]:         src/dhcp-boot/README.md
[LED/Relay]:         src/led-relay/README.md
[backbone.zip]:      https://nightly.link/westermo/app-demo/workflows/nightly/master/app-demo-backbone-coronet.zip
[dhcp-boot.zip]:     https://nightly.link/westermo/app-demo/workflows/nightly/master/app-demo-dhcp-boot-coronet.zip

[backbone-coronet]:  https://nightly.link/westermo/app-demo/workflows/nightly-backbone/master/app-demo-backbone-coronet.zip
[backbone-dagger]:   https://nightly.link/westermo/app-demo/workflows/nightly-backbone/master/app-demo-backbone-dagger.zip
[backbone-zero]:     https://nightly.link/westermo/app-demo/workflows/nightly-backbone/master/app-demo-backbone-zero.zip
[backbone.sha1]:     https://nightly.link/westermo/app-demo/workflows/nightly-backbone/master/sha1.zip

[dhcp-boot-coronet]: https://nightly.link/westermo/app-demo/workflows/nightly-dhcp-boot/master/app-demo-dhcp-boot-coronet.zip
[dhcp-boot-zero]:    https://nightly.link/westermo/app-demo/workflows/nightly-dhcp-boot/master/app-demo-dhcp-boot-zero.zip
[dhcp-boot-dagger]:  https://nightly.link/westermo/app-demo/workflows/nightly-dhcp-boot/master/app-demo-dhcp-boot-dagger.zip
[dhcp-boot.sha1]:    https://nightly.link/westermo/app-demo/workflows/nightly-dhcp-boot/master/sha1.zip

[led-relay-coronet]:    https://nightly.link/westermo/app-demo/workflows/nightly-led-relay/master/app-demo-led-relay-coronet.zip
[led-relay-zero]:    https://nightly.link/westermo/app-demo/workflows/nightly-led-relay/master/app-demo-led-relay-zero.zip
[led-relay-dagger]:    https://nightly.link/westermo/app-demo/workflows/nightly-led-relay/master/app-demo-led-relay-dagger.zip
[led-relay.sha1]:    https://nightly.link/westermo/app-demo/workflows/nightly-led-relay/master/sha1.zip
