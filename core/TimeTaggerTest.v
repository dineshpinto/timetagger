`timescale 1ns / 1ps
/*
    core of TimeTagger, an OpalKelly based single photon counting library
    Copyright (C) 2011  Markus Wick <wickmarkus@web.de>

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
//////////////////////////////////////////////////////////////////////////////////
// simulation workbench for the timetagger
// the delay of #23 per clock is about the real 
// ratio of the tap line and the input clock
//////////////////////////////////////////////////////////////////////////////////
module TimeTaggerTest(
    );

reg clk;	
reg [7:0] line;
reg [15:0] conf_enable_channel;

reg write_full;
wire write_enable;
wire [31:0] write_data;

TimeTagger tagger (
    .trig_clk(clk), 
    .trig_line(line), 
    .write_full(write_full), 
    .write_enable(write_enable), 
    .write_data(write_data), 
    .conf_enable_channel(conf_enable_channel)
    );
	 
initial begin
	conf_enable_channel = 16'h0000;
	clk = 0;
	line = 0;
	write_full = 0;
end

always begin
	#1000 conf_enable_channel = 16'h0000;
	#20001 conf_enable_channel = 16'hffff;
end
	 
always
	#43 clk = ~clk; // do not change it :-)

always begin
	#133 line[7] = 1;
	#13  line[7] = 0;
end

endmodule
