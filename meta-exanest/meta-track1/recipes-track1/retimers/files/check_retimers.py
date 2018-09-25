#!/usr/bin/env python3

import subprocess

def read_i2c ( i2c_bus, i2c_addr):
	cmd_str="/usr/sbin/i2cget -y {} 0x{:02x}".format(i2c_bus,i2c_addr)
	#print(cmd_str)
	out=subprocess.Popen(cmd_str, shell=True, stdout=subprocess.PIPE).stdout.read()
	#print(out)
	str="{}".format(out)
	return int(str.split("'")[-2].split("\\")[-2], base=16)


def write_i2c ( i2c_bus, i2c_addr, reg_addr , reg_value):
	cmd_str="/usr/sbin/i2cset -y {} 0x{:02x} 0x{:02x} 0x{:02x}".format(i2c_bus,i2c_addr,reg_addr, reg_value)
	#print(cmd_str)
	out=subprocess.Popen(cmd_str, shell=True, stdout=subprocess.PIPE).stdout.read()
	if(out):
		print(out)

def read_i2c_reg ( i2c_bus, i2c_addr, reg_addr ):
	cmd_str="/usr/sbin/i2cset -y {} 0x{:02x} 0x{:02x}".format(i2c_bus,i2c_addr,reg_addr)
	#print(cmd_str)
	out=subprocess.Popen(cmd_str, shell=True, stdout=subprocess.PIPE).stdout.read()
	return read_i2c(i2c_bus,i2c_addr)


for i2c_addr in [0x1B,0x20,0x22,0x23]:
#for i2c_addr in [0x20]:

	write_i2c(1,i2c_addr,0xFF,0x0)	
	clk_detect=bool(read_i2c_reg(1,i2c_addr,0xB) & 0x40)
	clk_conf=read_i2c_reg(1,i2c_addr,0x2)
	print("Retimer @0x{:02x} CLK_CONF: 0x{:02x} CLK_DETECT: {}".format(i2c_addr,clk_conf,clk_detect))
	for channel in range(16):
		write_i2c(1,i2c_addr,0xFF,0x0)	
		if channel < 8:
			write_i2c(1,i2c_addr,0xFC,(1<<channel))
			write_i2c(1,i2c_addr,0xFD,0)
		else:
			write_i2c(1,i2c_addr,0xFC,0)
			write_i2c(1,i2c_addr,0xFD,(1<<(channel-8)))
		write_i2c(1,i2c_addr,0xFF,0x1)
		chan_detect=read_i2c_reg(1,i2c_addr,0x1)
		chan_detect2=read_i2c_reg(1,i2c_addr,0x78)
		signal_det=bool(chan_detect2 & 0x20)
		cdr_lock=bool(chan_detect2 & 0x10)
		print("Channel {}:\t 0x01:0x{:02x} 0x78:{:02x} SIGNAL: {}\t CDR_LOCK {} ".format(channel,chan_detect,chan_detect2,signal_det, cdr_lock))	
	
