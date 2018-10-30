/*
 * (C) Copyright 2013 Xilinx, Inc.
 *
 * Configuration for Micro Zynq Evaluation and Development Board - MicroZedBoard
 * See zynq-common.h for Zynq common configs
 *
 * SPDX-License-Identifier:  GPL-2.0+
 */

#ifndef __CONFIG_ZYNQ_ICEOTOPE_BMC_H
#define __CONFIG_ZYNQ_ICEOTOPE_BMC_H


#define CONFIG_SYS_NO_FLASH

#define CONFIG_ZYNQ_SDHCI0
#define CONFIG_ZYNQ_USB

/*
 * A custom env for our usage, remove lots of crap
 */

#define CONFIG_EXTRA_ENV_SETTINGS \
  "ethaddr=00:0a:35:00:01:22\0" \
  "kernel_image=uImage\0" \
  "kernel_load_address=0x2080000\0" \
  "ramdisk_image=uramdisk.image.gz\0" \
  "ramdisk_load_address=0x4000000\0"  \
  "devicetree_image=devicetree.dtb\0" \
  "devicetree_load_address=0x2000000\0" \
  "bitstream_image=system.bit.bin\0"  \
  "boot_image=BOOT.bin\0" \
  "fit_image=openbmc_fit.itb\0" \
  "loadbit_addr=0x100000\0" \
  "loadbootenv_addr=0x2000000\0" \
  "kernel_size=0x500000\0"  \
  "devicetree_size=0x20000\0" \
  "ramdisk_size=0x5E0000\0" \
  "boot_size=0xF00000\0"  \
  "fdt_high=0x2000000\0" \
  "initrd_high=0xFFFFFFFF\0"  \
  "bootenv=uEnv.txt\0" \
  "loadbootenv=load mmc 0 ${loadbootenv_addr} ${bootenv}\0" \
  "importbootenv=echo Importing environment from SD ...; " \
    "env import -t ${loadbootenv_addr} $filesize\0" \
  "sd_uEnvtxt_existence_test=test -e mmc 0 /uEnv.txt\0" \
  "preboot=if test $modeboot = sdboot && env run sd_uEnvtxt_existence_test; " \
      "then if env run loadbootenv; " \
        "then env run importbootenv; " \
      "fi; " \
    "fi; \0" \
  "mmc_loadbit=echo Loading bitstream from SD/MMC/eMMC to RAM.. && " \
    "mmcinfo && " \
    "load mmc 0 ${loadbit_addr} ${bitstream_image} && " \
    "fpga load 0 ${loadbit_addr} ${filesize}\0" \
  "uenvboot=" \
    "if run loadbootenv; then " \
      "echo Loaded environment from ${bootenv}; " \
      "run importbootenv; " \
    "fi; " \
    "if test -n $uenvcmd; then " \
      "echo Running uenvcmd ...; " \
      "run uenvcmd; " \
    "fi\0" \
  "sdboot=if mmcinfo; then " \
      "run uenvboot; " \
      "echo Copying Linux from SD to RAM... && " \
      "load mmc 0 ${kernel_load_address} ${kernel_image} && " \
      "load mmc 0 ${devicetree_load_address} ${devicetree_image} && " \
      "load mmc 0 ${ramdisk_load_address} ${ramdisk_image} && " \
      "bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}; " \
    "fi\0" \
  "jtagboot=echo TFTPing Linux to RAM... && " \
    "tftpboot ${kernel_load_address} ${kernel_image} && " \
    "tftpboot ${devicetree_load_address} ${devicetree_image} && " \
    "tftpboot ${ramdisk_load_address} ${ramdisk_image} && " \
    "bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0" \
  "fitboot=echo TFTPing Linux to RAM... && " \
    "dhcp ${kernel_load_address} ${fit_image} && " \
    "bootm \0" \
  "fit_image=openbmc_fit.itb\0" \
  "qspiboot=echo Loading SD to RAM... && mmcinfo && load mmc 0 ${kernel_load_address} ${fit_image} && bootm ${kernel_load_address}\0" \


#define CONFIG_ZYNQ_I2C0
#define CONFIG_ZYNQ_I2C1
#define CONFIG_CMD_EEPROM
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN 1
#define CONFIG_SYS_I2C_SPEED 100000
#define CONFIG_ZYNQ_GEM_EEPROM_ADDR 0x50
#define CONFIG_ZYNQ_GEM_I2C_MAC_OFFSET  0x24
#define CONFIG_ZYNQ_EEPROM_BUS 1
#define CONFIG_SYS_I2C_ZYNQ
#define CONFIG_SYS_I2C
#define CONFIG_SYS_NUM_I2C_ADAPTERS 2

#define CONFIG_LZMA

#include <configs/zynq-common.h>


/* Remove this, we want to use the IP given by the DHCP */
#ifdef CONFIG_BOOTP_SERVERIP
#undef CONFIG_BOOTP_SERVERIP
#endif

#endif /* __CONFIG_ZYNQ_ICEOTOPE_BMC_H */
