BR2_TARGET_GENERIC_HOSTNAME="dhcp-boot"

define(DEMO_VENDOR_ID,"dhcp-boot")
define(DEMO_VENDOR_NAME, "DHCP-boot Application")

BR2_PACKAGE_DNSMASQ=y
BR2_PACKAGE_DROPBEAR=y
BR2_PACKAGE_LLDPD=y
BR2_PACKAGE_SOCAT=y
BR2_PACKAGE_TCPDUMP=y
BR2_TARGET_ROOTFS_SQUASHFS=y
BR2_PACKAGE_DHCP_BOOT=y