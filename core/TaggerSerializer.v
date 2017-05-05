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
// converts the simple subtimes into well formated 
// 32 bit words. Adds stubid nobs, if nothing was send
// for a long time
//////////////////////////////////////////////////////////////////////////////////
module TaggerSerializer#(parameter CHANNELS=1, parameter BITS=1)(
    input  wire							clk,
	 
	 // input data
    input  wire [BITS*CHANNELS-1:0] in_subtimes,
	 input  wire [CHANNELS-1:0] 		in_edge_detected,
	 input  wire [15:0]					in_counter,
	 input  wire							in_counter_rollover,
	 input  wire							in_overflow,
	 input  wire							in_empty,
	 output reg								in_not_stall,
	 
	 // output data
	 input  wire                  	write_full,
	 output reg  							write_enable,
    output reg [31:0] 					write_data
);

reg [1:0] state;
parameter [1:0] 
	s_waiting = 2'h0, 
	s_waiting2 = 2'h1, 
	s_rollover = 2'h2, 
	s_channels = 2'h3;

reg [3:0] channel;
wire [BITS-1:0] subtime [CHANNELS-1:0];

reg [6:0] zerocounter;

initial begin
	in_not_stall <= 0;
	write_enable <= 0;
	state <= s_waiting;
	channel <= 0;
	zerocounter <= 0;
end

always @(posedge clk) begin
	// default values
	write_enable <= 0;
	in_not_stall <= 0;
	zerocounter <= 0;
	write_data <= 0;
	
	if(~write_full) begin
		case(state)
			s_waiting: begin //waiting
				if(~in_empty) begin
					in_not_stall <= 1;
					state <= s_waiting2;
				end else begin
					{write_enable, zerocounter} <= zerocounter + 1;
				end
			end
			s_waiting2: begin
				state <= s_rollover;
			end
			s_rollover: begin // searching for rollovers + overflow
				write_data[30] <= in_overflow;
				write_data[31] <= in_counter_rollover;
				state <= s_channels;
				write_enable <= in_counter_rollover || in_overflow;
			end
			s_channels: begin // searching on channels
				if(channel == CHANNELS-1) begin
					state <= s_waiting;
					channel <= 0;
					if(~in_empty) begin
						in_not_stall <= 1;
						state <= s_waiting2;
					end
				end else begin
					state <= s_channels;
					channel <= channel+1;
				end
				write_enable <= in_edge_detected[channel];
				write_data[15:0]  <= in_counter;
				write_data[23:16] <= subtime[channel];
				write_data[27:24] <= channel;
				write_data[29] <= 1;
			end
		endcase
	end
end


genvar i;
generate
for(i=0; i<CHANNELS; i=i+1) begin: subtimes
	assign subtime[i] = in_subtimes[BITS*(i+1)-1:BITS*i];
end
endgenerate

endmodule
