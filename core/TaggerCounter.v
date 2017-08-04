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
// 16 bit counter with rollover flag
//////////////////////////////////////////////////////////////////////////////////
module TaggerCounter (
    input  wire			clk,
	 
	 output reg [15:0]	counter,
	 output reg				rollover
);

initial begin
	rollover <= 0;
	counter  <= 0;
end

always @(posedge clk) begin
	counter  <= counter + 1;
	rollover <= counter == 16'hFFFF;
end

endmodule
