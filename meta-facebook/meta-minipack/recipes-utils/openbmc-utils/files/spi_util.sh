#!/bin/sh

pca9534_ee_sel=0x22
pca9534_spi_sel=0x26
output_reg=0x1
mode_reg=0x3
SMB_CPLD="/sys/class/i2c-adapter/i2c-12/12-003e"
SCM_CPLD="/sys/class/i2c-adapter/i2c-2/2-0035"
debug=0

trap cleanup_spi INT TERM QUIT EXIT

function get_flash_first_type()
{
    type=`flashrom -p linux_spi:dev=/dev/spidev${1}.0 | grep Found \
                 | awk 'NR==1' | cut -d '"' -f 2`
    echo $type
}

function get_flash_size()
{
    flash_sz=`flashrom -p linux_spi:dev=/dev/spidev${1}.0 | grep "kB" \
                | awk 'NR==1' | cut -d ' ' -f 6 | cut -d '(' -f 2`
    echo $flash_sz
}

# $1: in file size $2: flash size $3: output file path
function pad_ff()
{
    out_file=$3
    pad_size=$(($2 - $1))
    dd if=/dev/zero bs=$pad_size count=1 | tr "\000" "\377" >> $out_file
}

function resize_file()
{
    in_file=$1
    out_file=$2
    spi_no=$3
    in_file_sz=`stat -c%s $in_file`
    flash_sz=$(get_flash_size $spi_no)
    flash_sz=$(($flash_sz * 1024))

    if [ $in_file_sz -ne $flash_sz ];then
        cp $in_file $out_file
        pad_ff $in_file_sz $flash_sz $out_file
    else
        mv $in_file $out_file
    fi
}

function dump_gpio_config(){
    if [ $debug = 0 ]; then
        return
    fi
    echo "Dump SPI configurations..."
    if [ $1 = "SPI1" ];then
        openbmc_gpio_util.py dump | grep SPI1
    else
        openbmc_gpio_util.py dump | grep SPI2
    fi
}

# currently only BCM5396 EEPROM
function set_spi1_to_gpio(){
    devmem_set_bit $(scu_addr 7C) 12
    devmem_set_bit $(scu_addr 7C) 13
    devmem_set_bit $(scu_addr 80) 15
    CLK="/tmp/gpionames/BMC_SPI1_CLK"
    CS0="/tmp/gpionames/BMC_SPI1_CS0"
    CS1="/tmp/gpionames/BMC_SPI1_CS1"
    MISO="/tmp/gpionames/BMC_SPI1_MISO"
    MOSI="/tmp/gpionames/BMC_SPI1_MOSI"
    echo out > $CLK/direction
    echo out > $CS0/direction
    echo out > $CS1/direction
    echo out > $MISO/direction
    echo out > $MOSI/direction
    dump_gpio_config SPI1
}

function set_spi1_to_spi(){
    devmem_set_bit $(scu_addr 70) 12
    dump_gpio_config SPI1
}

# currently no use
function set_spi2_to_spi_cs(){
    cs=$1
    if [ ${cs} == 0 ]; then
        # spi2 cs0
        devmem_set_bit $(scu_addr 88) 26
        devmem_set_bit $(scu_addr 88) 27
        devmem_set_bit $(scu_addr 88) 28
        devmem_set_bit $(scu_addr 88) 29
    else
        # spi2 cs1
        devmem_set_bit $(scu_addr 8C) 0
        devmem_set_bit $(scu_addr 88) 27
        devmem_set_bit $(scu_addr 88) 28
        devmem_set_bit $(scu_addr 88) 29
    fi
    dump_gpio_config SPI2
}

# currently no use
function set_spi2_to_gpio(){
    devmem_clear_bit $(scu_addr 88) 26
    devmem_clear_bit $(scu_addr 88) 27
    devmem_clear_bit $(scu_addr 88) 28
    devmem_clear_bit $(scu_addr 88) 29
    devmem_clear_bit $(scu_addr 8C) 0
    CLK="/tmp/gpionames/BMC_SPI2_CLK"
    CS0="/tmp/gpionames/BMC_SPI2_CS0"
    CS1="/tmp/gpionames/BMC_SPI2_CS1"
    MISO="/tmp/gpionames/BMC_SPI2_MISO"
    MOSI="/tmp/gpionames/BMC_SPI2_MOSI"
    echo out > $CLK/direction
    echo out > $CS0/direction
    echo out > $CS1/direction
    echo out > $MISO/direction
    echo out > $MOSI/direction
    dump_gpio_config SPI2
}

function read_flash_to_file()
{
    spi_no=$1
    file=$2
    modprobe -r spidev
    modprobe spidev
    type=$(get_flash_first_type $spi_no)
    flashrom -p linux_spi:dev=/dev/spidev${spi_no}.0 -r $file -c $type
}

function write_flash_to_file(){
    spi_no=$1
    in_file=$2
    out_file="/tmp/${3}_tmp"
    modprobe -r spidev
    modprobe spidev
    resize_file $in_file $out_file $spi_no
    type=$(get_flash_first_type $spi_no)
    flashrom -p linux_spi:dev=/dev/spidev${spi_no}.0 -w $out_file -c $type
}

function erase_flash(){
    spi_no=$1
    modprobe -r spidev
    modprobe spidev
    type=$(get_flash_first_type $spi_no)
    flashrom -p linux_spi:dev=/dev/spidev${spi_no}.0 -E -c $type
}

function read_m95m02_to_file(){
    spi_no=$1
    file=$2
    eeprom_node="/sys/bus/spi/devices/spi${spi_no}.1/eeprom"
    modprobe -r at25
    modprobe at25
    echo "m95m02 eeprom node: $eeprom_node"
    m95m02-util read $eeprom_node $file
}

function write_m95m02(){
    spi_no=$1
    file=$2
    eeprom_node="/sys/bus/spi/devices/spi${spi_no}.1/eeprom"
    modprobe -r at25
    modprobe at25
    echo "m95m02 eeprom node: $eeprom_node"
    m95m02-util write $eeprom_node $file
}

function erase_m95m02(){
    spi_no=$1
    eeprom_node="/sys/bus/spi/devices/spi${spi_no}.1/eeprom"
    modprobe -r at25
    modprobe at25
    echo "m95m02 eeprom node: $eeprom_node"
    m95m02-util erase $eeprom_node
}

function read_spi1_dev(){
    spi_no=1
    dev=$1
    file=$2
    case ${dev} in
        ## W25Q128 , W25Q128 , W25Q257
        "BACKUP_BIOS" | "IOB_FPGA_FLASH" | "TH3_FLASH")
            read_flash_to_file $spi_no $file
        ;;
        "BCM5396_EE")
            echo "Reading bcm5396 eeprom to $file..."
            at93cx6_util_py3.py chip read --file $file
        ;;
    esac
}

function write_spi1_dev(){
    spi_no=1
    dev=$1
    file=$2
    case ${dev} in
        ## W25Q128 , W25Q128 , W25Q257
        "BACKUP_BIOS" | "IOB_FPGA_FLASH" | "TH3_FLASH")
            write_flash_to_file $spi_no $file $dev
        ;;
        "BCM5396_EE")
            echo "Writing bcm5396 eeprom to $file..."
            at93cx6_util_py3.py chip write --file $file
        ;;
    esac
}

function erase_spi1_dev(){
    spi_no=1
    dev=$1
    file=$2
    case ${dev} in
        ## W25Q128 , W25Q128 , W25Q257
        "BACKUP_BIOS" | "IOB_FPGA_FLASH" | "TH3_FLASH")
            erase_flash $spi_no
        ;;
        "BCM5396_EE")
            echo "Erasing bcm5396 eeprom..."
            at93cx6_util_py3.py chip erase
        ;;
    esac
}

function get_smb_cpld_spi_1b(){
    b0=`cat $SMB_CPLD/spi_1_b0`
    b1=`cat $SMB_CPLD/spi_1_b1`
    b2=`cat $SMB_CPLD/spi_1_b2`
    echo "spi_1b [ $b2 $b1 $b0 ]"
}

function set_smb_cpld_spi_1b(){
    if [ $# = 3 ]; then
        echo $3 > $SMB_CPLD/spi_1_b0
        echo $2 > $SMB_CPLD/spi_1_b1
        echo $1 > $SMB_CPLD/spi_1_b2
    else
        echo "Set SPI1 BIT FAILED"
    fi
}

function config_spi1_pin_and_path(){
    dev=$1

    case ${dev} in
        "BACKUP_BIOS")
            set_spi1_to_spi
            set_smb_cpld_spi_1b 0 1 1
            echo 0 > $SCM_CPLD/iso_spi_en
            echo 0 > $SCM_CPLD/com_spi_oe_n
            echo 1 > $SCM_CPLD/com_spi_sel
        ;;
        "IOB_FPGA_FLASH")
            set_spi1_to_spi
            set_smb_cpld_spi_1b 0 0 0
            echo 1 > $SMB_CPLD/fpga_spi_mux_sel
        ;;
        "TH3_FLASH")
            set_spi1_to_spi
            set_smb_cpld_spi_1b 0 0 1
            echo 1 > $SMB_CPLD/th3_spi_mux_sel
        ;;
        "BCM5396_EE")
            set_spi1_to_gpio
            set_smb_cpld_spi_1b 0 1 0
            echo 1 > $SMB_CPLD/cpld_bcm5396_mux_sel
            gpio_set BMC_BCM5396_MUX_SEL 1
        ;;
        *)
            echo "Please enter {IOB_FPGA_FLASH, TH3_FLASH, BCM5396_EE, BACKUP_BIOS}"
            return
        ;;
    esac
    echo "Config SPI1 Done."
}

function config_spi2_pin_and_path(){
    local pim_no
    pim=$1
    dev=$2
    pim_no=${pim:3:3}
    config_spi2_pim_path $pim_no $dev
    bus=$(((pim_no-1)*8 + 85))
    config_spi2_pim_dev_path $bus $dev
    echo "Config SPI2 Done."
}

function get_smb_cpld_spi_2b(){
    b0=`cat $SMB_CPLD/spi_2_b0`
    b1=`cat $SMB_CPLD/spi_2_b1`
    b2=`cat $SMB_CPLD/spi_2_b2`
    b3=`cat $SMB_CPLD/spi_2_b3`
    echo "spi_2b [ $b3 $b2 $b1 $b0 ]"
}

function set_smb_cpld_spi_2b(){
    if [ $# = 4 ]; then
        echo $4 > $SMB_CPLD/spi_2_b0
        echo $3 > $SMB_CPLD/spi_2_b1
        echo $2 > $SMB_CPLD/spi_2_b2
        echo $1 > $SMB_CPLD/spi_2_b3
    else
        echo "Set SPI2 BIT FAILED"
    fi
}

function config_spi2_pim_path(){
    pim_no=$1
    local dev=$2
    dev=${dev:0:3}  # Detect PHY EEPROM to change CS1
    # Not PHY means CS0 FPGA, else CS1 PHY#_EE
    case ${pim_no} in
        "1")
            if [ $dev != "PHY" ]; then
                set_smb_cpld_spi_2b 0 0 0 0
            else
                set_smb_cpld_spi_2b 1 0 0 0
            fi
        ;;
        "2")
            if [ $dev != "PHY" ]; then
                set_smb_cpld_spi_2b 0 0 0 1
            else
                set_smb_cpld_spi_2b 1 0 0 1
            fi
        ;;
        "3")
            if [ $dev != "PHY" ]; then
                set_smb_cpld_spi_2b 0 0 1 0
            else
                set_smb_cpld_spi_2b 1 0 1 0
            fi
        ;;
        "4")
            if [ $dev != "PHY" ]; then
                set_smb_cpld_spi_2b 0 0 1 1
            else
                set_smb_cpld_spi_2b 1 0 1 1
            fi
        ;;
        "5")
            if [ $dev != "PHY" ]; then
                set_smb_cpld_spi_2b 0 1 0 0
            else
                set_smb_cpld_spi_2b 1 1 0 0
            fi
        ;;
        "6")
            if [ $dev != "PHY" ]; then
                set_smb_cpld_spi_2b 0 1 0 1
            else
                set_smb_cpld_spi_2b 1 1 0 1
            fi
        ;;
        "7")
            if [ $dev != "PHY" ]; then
                set_smb_cpld_spi_2b 0 1 1 0
            else
                set_smb_cpld_spi_2b 1 1 1 0
            fi
        ;;
        "8")
            if [ $dev != "PHY" ]; then
                set_smb_cpld_spi_2b 0 1 1 1
            else
                set_smb_cpld_spi_2b 1 1 1 1
            fi
        ;;
        *)
            return -1
        ;;
    esac
}

function exec_and_print(){
    if [ $debug = 1 ]; then
		echo "$1"
	fi
	$1
}

function detect_pca9534(){
    bus=$1
    dev_addr=$2
    dev_no=${dev_addr:2:3}
    i2cdetect -y $bus $dev_addr $dev_addr | grep $dev_no > /dev/null
}

function config_spi2_pim_dev_path(){
    bus=$1
    dev=$2

    # Configure output pin(0) to [0:2] 1111 1000 -> 0xf8
    spi_sel_output_bit=0xf8
    # Configure output pin(0) to [1:5] 1100 0001 -> 0xc1
    ee_sel_output_bit=0xc1

    detect_pca9534 $bus $pca9534_spi_sel
    spi_sel_ret=$?
    detect_pca9534 $bus $pca9534_ee_sel
    ee_sel_ret=$?
    if [ $spi_sel_ret != 0 ]; then
        echo "Cannot detect I2C device bus[$bus] addr[$pca9534_spi_sel]"
        return
	elif [ $ee_sel_ret != 0 ]; then
        echo "Cannot detect I2C device bus[$bus] addr[$pca9534_ee_sel]"
        return
    fi

    i2c_ret=$(i2cget -y $bus $pca9534_spi_sel $mode_reg)
    if [ $i2c_ret = "" ];then
        echo "Cannot read bus:$bus addr:$pca9534_spi_sel"
        return -1
    fi
    # Current value and with 0xf8
    config_val=$(printf "0x%x" $(($spi_sel_output_bit & $i2c_ret)))
    exec_and_print "i2cset -y $bus $pca9534_spi_sel $mode_reg $config_val"

    i2c_ret=$(i2cget -y $bus $pca9534_ee_sel $mode_reg)
    if [ $i2c_ret = "" ];then
        echo "Cannot read bus:$bus addr:$pca9534_ee_sel"
        return -1
    fi
    config_val=$(printf "0x%x" $(($ee_sel_output_bit & $i2c_ret)))
    exec_and_print "i2cset -y $bus $pca9534_ee_sel $mode_reg $config_val"

    read_spi_sel_val=$(i2cget -y $bus $pca9534_spi_sel $output_reg)
    read_spi_sel_val=$(($read_spi_sel_val | 0x7))

    read_spi_ee_val=$(i2cget -y $bus $pca9534_ee_sel $output_reg)
    read_spi_ee_val=$(($read_spi_ee_val | 0x3e))

    echo "Select PIM Device: $dev"
    case ${dev} in
        "PHY1_EE")
            set_spi2_to_spi_cs 1
            write_val=$(printf "0x%x" $(($read_spi_sel_val & 0xf8)))
            exec_and_print "i2cset -y $bus $pca9534_spi_sel $output_reg $write_val"
            write_val=$(printf "0x%x" $((read_spi_ee_val & 0xfb)))
            exec_and_print "i2cset -y $bus $pca9534_ee_sel $output_reg $write_val"
        ;;
        "PHY2_EE")
            set_spi2_to_spi_cs 1
            write_val=$(printf "0x%x" $(($read_spi_sel_val & 0xf9)))
            exec_and_print "i2cset -y $bus $pca9534_spi_sel $output_reg $write_val"
            write_val=$(printf "0x%x" $((read_spi_ee_val & 0xf7)))
            exec_and_print "i2cset -y $bus $pca9534_ee_sel $output_reg $write_val"
        ;;
        "PHY3_EE")
            set_spi2_to_spi_cs 1
            write_val=$(printf "0x%x" $(($read_spi_sel_val & 0xfa)))
            exec_and_print "i2cset -y $bus $pca9534_spi_sel $output_reg $write_val"
            write_val=$(printf "0x%x" $((read_spi_ee_val & 0xef)))
            exec_and_print "i2cset -y $bus $pca9534_ee_sel $output_reg $write_val"
        ;;
        "PHY4_EE")
            set_spi2_to_spi_cs 1
            write_val=$(printf "0x%x" $(($read_spi_sel_val & 0xfb)))
            exec_and_print "i2cset -y $bus $pca9534_spi_sel $output_reg $write_val"
            write_val=$(printf "0x%x" $((read_spi_ee_val & 0xdf)))
            exec_and_print "i2cset -y $bus $pca9534_ee_sel $output_reg $write_val"
        ;;
        "DOM_FPGA_FLASH")
            set_spi2_to_spi_cs 0
            write_val=$(printf "0x%x" $(($read_spi_sel_val & 0xfc)))
            exec_and_print "i2cset -y $bus $pca9534_spi_sel $output_reg $write_val"
            write_val=$(printf "0x%x" $((read_spi_ee_val & 0xfd)))
            exec_and_print "i2cset -y $bus $pca9534_ee_sel $output_reg $write_val"
        ;;
        *)
            echo "Please enter with {PHY#_EE, DOM_FPGA_FLASH}"
            return -1
        ;;
    esac
}

function read_spi2_dev(){
    spi_no=2
    dev=$1
    file=$2

    case ${dev:0:4} in
        "PHY"[1-4])
            # M95M02 EEPROM
            read_m95m02_to_file $spi_no $file
        ;;
        "DOM_")
            # W25Q32
            read_flash_to_file $spi_no $file
        ;;
        *)
            echo "SPI2 without this device [$dev]"
        ;;
    esac
}

function write_spi2_dev(){
    spi_no=2
    dev=$1
    case ${dev:0:4} in
        "PHY"[1-4])
            # M95M02 EEPROM
            write_m95m02 $spi_no $file
        ;;
        "DOM_") # Catch 4 char from "DOM_FPGA_FLASH"
            # W25Q32
            write_flash_to_file $spi_no $file $dev
        ;;
        *)
            echo "SPI2 without this device [$dev]"
        ;;
    esac
}

function erase_spi2_dev(){
    spi_no=2
    dev=$1
    case ${dev:0:4} in
        "PHY"[1-4])
            # M95M02 EEPROM
            erase_m95m02 $spi_no
        ;;
        "DOM_") # Catch 4 char from "DOM_FPGA_FLASH"
            # W25Q32
            erase_flash $spi_no
        ;;
        *)
            echo "SPI2 without this device [$dev]"
        ;;
    esac
}

function operate_spi1_dev(){
    op=$1
    dev=$2
    file=$3
    ## Operate devices ##
    case ${op} in
        "read")
                read_spi1_dev $dev $file
        ;;
        "write")
                write_spi1_dev $dev $file
        ;;
        "erase")
                erase_spi1_dev $dev
        ;;
        *)
            echo "Operation $op is not defined."
            usage
        ;;
    esac
}

function operate_spi2_dev(){
    op=$1
    dev=$2
    file=$3
	## Operate devices ##
    case ${op} in
        "read")
                read_spi2_dev $dev $file
        ;;
        "write")
                write_spi2_dev $dev $file
        ;;
        "erase")
                erase_spi2_dev $dev
        ;;
        *)
            echo "Operation $op is not defined."
            usage
        ;;
    esac
}

function cleanup_spi(){
    #echo "Caught Signal: reset spi selecth"
    echo 0 > $SMB_CPLD/fpga_spi_mux_sel
    echo 0 > $SMB_CPLD/th3_spi_mux_sel
    echo 0 > $SMB_CPLD/cpld_bcm5396_mux_sel
    echo 0 > $SCM_CPLD/com_spi_sel

    for i in {1..8}
    do
        bus=$(((i-1)*8 + 85))
        detect_pca9534 $bus $pca9534_ee_sel
        if [ $? = 0 ]; then
            read_spi_ee_val=$(i2cget -y $bus $pca9534_ee_sel $output_reg)
            write_ee_val=$(($read_spi_ee_val | 0x3e))
            i2cset -y $bus $pca9534_ee_sel $output_reg $write_ee_val 2> /dev/null
        fi
    done
}

function ui(){
    . /usr/local/bin/openbmc-utils.sh
    op=$1
    spi=$2

    # SPI 1.0 speed
    devmem 0x1e630010 32 0x407
    # SPI 2.0 speed
    devmem 0x1e631010 32 0x407
    # SPI 2.1 speed
    devmem 0x1e631014 32 0x2007
	## Open the path to device ##
    case ${spi} in
        "spi1")
            dev=$3
            file=$4
            config_spi1_pin_and_path $dev
            operate_spi1_dev $op $dev $file
        ;;
        "spi2")
            pim=$3
            dev=$4
            file=$5
            config_spi2_pin_and_path $pim $dev
            operate_spi2_dev $op $dev $file
        ;;
        *)
            echo "No such SPI bus."
            usage
            return -1
        ;;
    esac

}

function usage(){
    program=`basename "$0"`
    echo "======================================="
    echo "Usage:"
    echo "$program <OP> spi1 <SPI1_DEV> <File>"
    echo "$program <OP> spi2 <PIM> <SPI2_DEV> <File>"
    echo "  <OP>       : read, write, erase"
    echo "  <SPI1_DEV> : IOB_FPGA_FLASH, TH3_FLASH, BCM5396_EE, BACKUP_BIOS"
    echo "  <PIM>      : PIM[1-8]"
    echo "  <SPI2_DEV> : PHY[1-4]_EE, DOM_FPGA_FLASH"
    echo "=============== Example ==============="
    echo "    SPI1 : $program write spi1 IOB_FPGA_FLASH upgrade_file"
    echo "    SPI2 : $program read spi2 PIM1 PHY1_EE read_eeprom_file"
    echo "    SPI2 : $program erase spi2 PIM1 DOM_FPGA_FLASH"
    echo "======================================="
}

if [ $# -ge 1 ] || [ $# -le 5 ]; then
    ui $1 $2 $3 $4 $5
else
    usage
fi
