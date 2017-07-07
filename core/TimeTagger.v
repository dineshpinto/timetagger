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
// The TimeTagger
// it scans the input lines for edge, and returns a 32 bit
// word with timestamp for each one.
//
// Warning: if you change a parameter, 
//          you also have to touch the TaggerSerializer
//////////////////////////////////////////////////////////////////////////////////

module TimeTagger #(parameter CHANNELS=16, parameter BITS=7) (
    // input ports waiting for trigger
    input  wire						trig_clk,
    input  wire [(CHANNELS/2)-1:0] 	trig_line,
	 
	 // interface for writing data
	 input  wire                  write_full,
	 output wire  						write_enable,
    output wire [31:0] 				write_data,
	 
	 // interface for configuration
	 input  wire [CHANNELS-1:0] conf_enable_channel,
	 input  wire [CHANNELS*16-1:0] conf_deadtimes,

	 // laser filter control
	 input  wire conf_enable_laser_filter
);

///////////
// Input //
///////////

wire [BITS*CHANNELS-1:0] 	input_subtimes;
wire [CHANNELS-1:0] 			input_edge_detected;

TaggerInput #(.CHANNELS(CHANNELS), .BITS(BITS)) tagger_input (
    .clk(trig_clk), 
    .line(trig_line), 
    .subtimes(input_subtimes), 
    .edge_detected(input_edge_detected)
);
	 
////////////
// Filter //
////////////

wire [BITS*CHANNELS-1:0] 	filter_subtimes;
wire [CHANNELS-1:0] 			filter_raising_edge;
wire [CHANNELS-1:0] 			filter_edge_detected;

TaggerFilter #(.CHANNELS(CHANNELS), .BITS(BITS)) tagger_filter (
    .clk(trig_clk), 
    .in_subtimes(input_subtimes), 
    .in_edge_detected(input_edge_detected),
    .out_subtimes(filter_subtimes), 
    .out_edge_detected(filter_edge_detected),
    .enable_channel(conf_enable_channel),
    .enable_laser_filter(conf_enable_laser_filter),
	 .deadtimes(conf_deadtimes)
);

//////////////
// Purifier //
//////////////

TaggerPurifier #(.CHANNELS(CHANNELS), .BITS(BITS)) tagger_purifier (
    .clk(trig_clk), 
    .in_subtimes(filter_subtimes), 
    .in_edge_detected(filter_edge_detected), 
    .write_full(write_full), 
    .write_enable(write_enable), 
    .write_data(write_data)
    );

endmodule
