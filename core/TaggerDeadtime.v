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
// deadtime filter
//////////////////////////////////////////////////////////////////////////////////
module TaggerDeadtime#(parameter BITS=1, parameter CONF_BITS=16) (
    input  wire						clk,
	 
    input  wire [BITS-1:0] 		in_subtimes,
	 input  wire 		 				in_edge_detected,
	 
    output reg  [BITS-1:0] 		out_subtimes,
	 output reg  						out_edge_detected,
	 
	 input  wire [CONF_BITS-1:0]	conf_deadtime
);

wire sample;
reg last_subtimes;
reg [CONF_BITS-1:0] counter;
reg counter_is_zero;
reg counter_is_one;
reg counter_is_two;
reg conf_is_zero;
reg conf_is_one;
reg conf_is_two;
wire conf_will_be_zero;
wire conf_will_be_one;
reg [CONF_BITS-1:0] conf_delay;


initial begin
	last_subtimes <= 0;
	out_edge_detected <= 0;
	out_subtimes <= 0;
	counter <= 0;
	counter_is_zero <= 1;
	counter_is_one <= 0;
	counter_is_two <= 0;
	conf_delay <= 0;
	conf_is_zero <= 1;
	conf_is_one <= 0;
	conf_is_two <= 0;
end

assign sample = (((last_subtimes > in_subtimes) && conf_will_be_one) || conf_will_be_zero) && in_edge_detected;

assign conf_will_be_zero = (out_edge_detected && conf_is_zero) || (!out_edge_detected && (counter_is_zero || counter_is_one));
assign conf_will_be_one = (out_edge_detected && conf_is_one) || (!out_edge_detected && (counter_is_one || counter_is_two));

always @(posedge clk) begin
	out_edge_detected <= sample;
	out_subtimes <= in_subtimes;
	if (sample) begin
		last_subtimes <= in_subtimes;
	end
	
	if (out_edge_detected) begin
		counter <= conf_delay;
		counter_is_two <= conf_is_two;
	end else begin
		if (counter_is_zero) begin
			counter <= counter;
			counter_is_two <= 0;
		end else begin
			counter <= counter - 1;
			counter_is_two <= counter == 3;
		end
	end
	counter_is_zero <= conf_will_be_zero;
	counter_is_one <= conf_will_be_one;
	
	conf_is_zero <= conf_deadtime == 0;
	conf_is_one <= conf_deadtime == 1;
	conf_is_two <= conf_deadtime == 2;
	conf_delay <= conf_deadtime;
end

endmodule