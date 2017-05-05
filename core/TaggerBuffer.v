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
// buffers incomming data for a stallable output
// with overflow handling
// if an overflow is detected, no more edge will be
// accepted until the fifo is empty
//////////////////////////////////////////////////////////////////////////////////
module TaggerBuffer#(parameter CHANNELS=1, parameter BITS=1)(
    input  wire							clk,
	 
	 // input data
    input  wire [BITS*CHANNELS-1:0] in_subtimes,
	 input  wire [CHANNELS-1:0] 		in_edge_detected,
	 input  wire [15:0]					in_counter,
	 input  wire							in_counter_rollover,
	 
	 // buffered, output data
	 input  wire							out_not_stall,
	 output reg 							out_empty,
    output reg  [BITS*CHANNELS-1:0] out_subtimes,
	 output reg  [CHANNELS-1:0] 		out_edge_detected,
	 output reg  [15:0]					out_counter,
	 output reg 							out_counter_rollover, 
	 output reg 							out_overflow
    );
	 
////////////////////////////////////////////////////
// Delay anything and calculate wr_en_detected //
////////////////////////////////////////////////////
reg [BITS*CHANNELS-1:0] in_subtimes_delayed;
reg [CHANNELS-1:0] 		in_edge_detected_delayed;
reg [15:0]					in_counter_delayed;
reg							in_counter_rollover_delayed;
reg							in_wr_en_delayed;

always @(posedge clk) begin
	in_subtimes_delayed <= in_subtimes;
	in_edge_detected_delayed <= in_edge_detected;
	in_counter_delayed <= in_counter;
	in_counter_rollover_delayed <= in_counter_rollover;
	in_wr_en_delayed <= in_counter_rollover || (|in_edge_detected);
end
 
//////////
// fifo //
//////////

parameter	FIFOS = ((1+BITS)*CHANNELS+17)/16+1;

wire 			[FIFOS-1:0]		full;
wire			[FIFOS-1:0] 	empty;
(* EQUIVALENT_REGISTER_REMOVAL="NO" *)reg			[FIFOS-1:0]		rd_en;
wire 			[FIFOS*16-1:0] dout;
reg  			[FIFOS*16-1:0] din;
(* EQUIVALENT_REGISTER_REMOVAL="NO" *)reg			[FIFOS-1:0]		wr_en;
reg 								waiting_after_overflow;

// generate multiple 16bit fifos
genvar i;
generate
for(i=0; i<FIFOS; i=i+1) begin: fifo
	PurifierBuffer purifier_buffer (
	  .clk(clk), // input clk
	  .din(din[i*16+15:i*16]), // input [15 : 0] din
	  .wr_en(wr_en[i]), // input wr_en
	  .rd_en(rd_en[i]), // input rd_en
	  .dout(dout[i*16+15:i*16]), // output [15 : 0] dout
	  .full(), // output full
	  .almost_full(full[i]), // output overflow
	  .empty(), // output empty
	  .almost_empty(empty[i])
	);
end
endgenerate

///////////////////
// reading block //
///////////////////
integer k;
always @(posedge clk) begin
	for(k=0;k<FIFOS;k=k+1)
		rd_en[k] <= out_not_stall;
	
	out_empty <= empty[0];
	if(out_not_stall) begin
		// else, split dout into vars
		out_subtimes <= dout[(1+BITS)*CHANNELS+17:CHANNELS+18];
		out_edge_detected <= dout[CHANNELS+17:18];
		out_counter <= dout[17:2];
		out_counter_rollover <= dout[1];
		out_overflow <= dout[0];
	end
end

///////////////////
// writing block //
///////////////////

always @(posedge clk) begin
	
	// after an overflow, wait for empty
	if(waiting_after_overflow) begin
		// overflow, no edges
		din[0] <= 1'b1;
		din[1] <= 0;
		din[17:2] <= 0;
		din[CHANNELS+17:18] <= 0;
		din[(1+BITS)*CHANNELS+17:CHANNELS+18] <= 0;
		wr_en <= empty;
		waiting_after_overflow <= !empty[0];
	end else begin	
		// generating din from vars
		din[0] <= full[0];
		din[1] <= in_counter_rollover_delayed;
		din[17:2] <= in_counter_delayed;
		din[CHANNELS+17:18] <= in_edge_detected_delayed;
		din[(1+BITS)*CHANNELS+17:CHANNELS+18] <= in_subtimes_delayed;
		
		for(k=0;k<FIFOS;k=k+1)
			wr_en[k] <= !full[k] && in_wr_en_delayed;
		waiting_after_overflow <= full[0] /*&& (in_counter_rollover || (|in_edge_detected))*/;
	end
end


initial begin
	out_edge_detected <= 0;
	out_counter_rollover <= 0;
	out_overflow <= 0;	
	out_subtimes <= 0;
	out_counter <= 0;
	out_empty <= 1;
	din <= 0;
	wr_en <= 0;
	waiting_after_overflow <= 0;
end

endmodule
