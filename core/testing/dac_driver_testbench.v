/*
    core of TimeTagger, an OpalKelly based single photon counting library
    Copyright (C) 2013  Helmut Fedder <helmut.fedder@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*

    Logic testing of dac_driver.v

    To use the dac_driver_testbench,

    install:

    - icarus verilog (iverilog)
    - gtkwave
    
    excecute in a shell:

            iverilog -o dac_driver.sim dac_driver.v dac_driver_testbench.v
            vvp dac_driver.sim -vcd

    type 'finish' and enter

    excecute in a shell:

            gtkwave dac_driver.vcd
            
*/      

`default_nettype none
`timescale 1ns / 1ps

module main;

reg			clk;
reg [15:0]	din;
reg 		wr_en;
wire 		sclk;
wire 		sync;
wire		sdout;
	
DacDriver #(
    .CLK_DIV(0),
    .WIDTH(16),
    .HOLD(5)
)
dut (
	.clk(clk),
	.din(din),
	.wr_en(wr_en),
	.sclk(sclk),
	.sync(sync),
	.sdout(sdout)
);

initial begin
    $dumpfile("dac_driver.vcd");
    $dumpvars;
    	clk = 0;
    	din = 27;
    	wr_en = 0;
    #10 wr_en = 1;
    #10 wr_en = 0;
	#20000 $stop;
end

always #5 clk = !clk;

endmodule


