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
// format the data
// Because of the serialization, a buffer is needed.
// Overflow handling is done in here
//////////////////////////////////////////////////////////////////////////////////
module TaggerPurifier#(parameter CHANNELS=1, parameter BITS=1)(
    input  wire							clk,
	 
    input wire [BITS*CHANNELS-1:0] 	in_subtimes,
	 input wire [CHANNELS-1:0] 		in_edge_detected,
	
	 // interface for writing data
	 input  wire                  	write_full,
	 output wire  							write_enable,
    output wire [31:0] 					write_data
);


/////////////
// Counter //
/////////////

wire [15:0]	counter;
wire			counter_rollover;
	 
TaggerCounter tagger_counter (
    .clk(clk), 
    .counter(counter), 
    .rollover(counter_rollover)
);

////////////
// Buffer //
////////////
wire buffer_not_stall;
wire buffer_empty;

wire [BITS*CHANNELS-1:0] 	buffer_subtimes;
wire [CHANNELS-1:0] 			buffer_edge_detected;
wire [15:0]						buffer_counter;
wire								buffer_counter_rollover;
wire								buffer_overflow;

TaggerBuffer #(.CHANNELS(CHANNELS), .BITS(BITS)) tagger_buffer  (
    .clk(clk), 
    .in_subtimes(in_subtimes), 
    .in_edge_detected(in_edge_detected), 
    .in_counter(counter), 
    .in_counter_rollover(counter_rollover), 
    .out_not_stall(buffer_not_stall), 
	 .out_empty(buffer_empty),
    .out_subtimes(buffer_subtimes), 
    .out_edge_detected(buffer_edge_detected), 
    .out_counter(buffer_counter), 
    .out_counter_rollover(buffer_counter_rollover), 
    .out_overflow(buffer_overflow)
);

////////////////
// Serializer //
////////////////

TaggerSerializer #(.CHANNELS(CHANNELS), .BITS(BITS)) tagger_serializer (
    .clk(clk), 
    .in_subtimes(buffer_subtimes), 
    .in_edge_detected(buffer_edge_detected), 
    .in_counter(buffer_counter), 
    .in_counter_rollover(buffer_counter_rollover), 
    .in_overflow(buffer_overflow), 
    .in_empty(buffer_empty), 
    .in_not_stall(buffer_not_stall), 
    .write_full(write_full), 
    .write_enable(write_enable), 
    .write_data(write_data)
);

endmodule
