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
// dummy filter, do nearly nothing
// except one: deadtime of one clock period
//////////////////////////////////////////////////////////////////////////////////
module TaggerFilter#(parameter CHANNELS=1, parameter BITS=1) (
    input  wire							clk,
	 
    input  wire [BITS*CHANNELS-1:0] in_subtimes,
	 input  wire [CHANNELS-1:0] 		in_edge_detected,
	 
    output reg  [BITS*CHANNELS-1:0] out_subtimes,
	 output reg  [CHANNELS-1:0] 		out_edge_detected,
	 
	 input  wire [CHANNELS-1:0] 		enable_channel,
	 input  wire 		               enable_laser_filter,
	 input  wire [CHANNELS*16-1:0]	deadtimes
);

// dead time
wire  [BITS*CHANNELS-1:0] 	dead_subtimes;
wire  [CHANNELS-1:0] 			dead_edge_detected;

// laster filter
reg photon_seen;

genvar i;
generate
for(i=0; i<CHANNELS; i=i+1) begin: dead
	TaggerDeadtime #(.BITS(BITS)) deadtime (
		 .clk(clk), 
		 .in_subtimes(in_subtimes[BITS*(i+1)-1:BITS*i]),
		 .in_edge_detected(in_edge_detected[i]),
		 .out_subtimes(dead_subtimes[BITS*(i+1)-1:BITS*i]),
		 .out_edge_detected(dead_edge_detected[i]),
		 .conf_deadtime(deadtimes[16*(i+1)-1:16*i])
	);
end
endgenerate


initial begin
	out_edge_detected <= 0;
	photon_seen <= 0;
end
	

always @(posedge clk) begin
	
	// laser filter
	photon_seen <= (photon_seen && !dead_edge_detected[7]) || dead_edge_detected[0];
	out_subtimes <= dead_subtimes;
	out_edge_detected <= dead_edge_detected & enable_channel[CHANNELS-1:0];
	out_edge_detected[7] <= dead_edge_detected[7] && (!enable_laser_filter || photon_seen) && enable_channel[7];

end

endmodule
