# Copyright 2018-present Facebook. All Rights Reserved.

SUPCPLD_SYSFS_DIR="/sys/class/i2c-dev/i2c-12/device/12-0043"
SCDCPLD_SYSFS_DIR="/sys/class/i2c-dev/i2c-4/device/4-0023"
SUP_PWR_ON_SYSFS="${SUPCPLD_SYSFS_DIR}/cpu_control"
PWR_SYSTEM_SYSFS="${SUPCPLD_SYSFS_DIR}/chassis_power_cycle"

wedge_iso_buf_enable() {
    return 0
}

wedge_iso_buf_disable() {
    return 0
}

wedge_is_us_on() {
    local val
    val=$(cat $SUP_PWR_ON_SYSFS 2> /dev/null | head -n 1)
    if [ -z "$val" ]; then
        return $default
    elif [ "$val" == "0x1" ]; then
        return 0            # powered on
    else
        return 1
    fi
}

wedge_board_type() {
    echo 'YAMP'
}

wedge_slot_id() {
    printf "1\n"
}

wedge_board_rev() {
    # assume P2
    return 2
}

# Should we enable OOB interface or not
wedge_should_enable_oob() {
    # YAMP uses BMC MAC since beginning
    return -1
}

wedge_power_on_board() {
    echo 1 > $SUP_PWR_ON_SYSFS
}

wedge_power_off_board() {
    echo 0 > $SUP_PWR_ON_SYSFS
    sleep 1
}
