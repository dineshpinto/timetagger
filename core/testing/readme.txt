To use the dac_driver_testbench,

install:

 - icarus verilog (iverilog)
 - gtkwave
 
excecute in a shell:

	iverilog -o dac_driver.sim ../dac_driver.v dac_driver_testbench.v
	vvp dac_driver.sim -vcd

type 'finish' and enter

excecute in a shell:

	gtkwave dac_driver.vcd
	
