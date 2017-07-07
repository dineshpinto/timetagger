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
// Company: 
// Engineer: 
// 
// Create Date:    16:11:48 09/02/2011 
// Design Name: 
// Module Name:    BcdConverterTest 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module BcdConverterTest(
    );

parameter BITS=2;

reg clk;	
reg [(1<<BITS)-1:0] samples;
wire [BITS-1:0] subtimes;
wire first_sample;
wire last_sample;
wire edge_detected;

TaggerBcdConverterFalling #(.BITS(BITS)) tagger_bcd_converter (
	 .clk(clk), 
	 .samples(samples[(1<<BITS)-1:0]), 
	 .subtimes(subtimes[BITS-1:0]), 
	 .first_sample(first_sample), 
	 .last_sample(last_sample),
	 .edge_detected(edge_detected)
);

initial begin
	clk <= 0;
	samples <= 0;
end

always begin
	#1 clk <= ~clk;
end

always begin
	#100 samples <= samples+1;
end

endmodule
