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
// converting lots of samples in BCD
// it only detect raising edges
// if more than one edge was found, only
// the newest will be returned
// Warning: this is a recursiv algorithm
//////////////////////////////////////////////////////////////////////////////////
module TaggerBcdConverterRaising #(parameter BITS=1) (
	input  wire							clk,
	input  wire	[(1<<BITS)-1:0] 	samples,
   output reg  [BITS-1:0] 			subtimes,
	output reg  						first_sample,
	output reg  						last_sample,
	output reg	 						edge_detected
);

parameter NEWBITS = BITS-1;

initial begin
	subtimes <= 0;
	first_sample <= 0;
	last_sample <= 0;
	edge_detected <= 0;
end

generate
	if(BITS == 1) begin
		// exit condition: for two samples
		always @(posedge clk) begin
			subtimes[0]   <= samples[1] && !samples[0];
			last_sample   <= samples[1];
			first_sample  <= samples[0];
			edge_detected <= samples[1] && !samples[0];
		end
	end else  if(BITS == 2) begin
		// exit condition: for four samples
		// more complex, but needs less luts/ff
		always @(posedge clk) begin
			subtimes[0]   <= (samples[3]&&!samples[2]) || (samples[1]&&!samples[0]);
			subtimes[1]   <= (samples[3]&&!samples[2]) || (samples[2]&&!samples[1]);
			
			first_sample  <= samples[0];
			last_sample   <= samples[3];
			
			edge_detected <= (samples[3]&&!samples[2]) || (samples[1]&&!samples[0]) || (samples[2]&&!samples[1]);
		end
	end else begin
	   wire [NEWBITS-1:0] 	subtimes_0;
		wire [NEWBITS-1:0] 	subtimes_1;
		wire       				first_sample_0;
		wire      				first_sample_1;
		wire       				last_sample_0;
		wire      				last_sample_1;
		wire      				edge_detected_0;
		wire       				edge_detected_1;
		
		TaggerBcdConverterRaising #(.BITS(NEWBITS)) tagger_bcd_converter_0 (
			 .clk(clk), 
			 .samples(samples[(1<<NEWBITS)-1:0]), 
			 .subtimes(subtimes_0), 
			 .first_sample(first_sample_0),
			 .last_sample(last_sample_0),
			 .edge_detected(edge_detected_0)
		);
		TaggerBcdConverterRaising #(.BITS(NEWBITS)) tagger_bcd_converter_1 (
			 .clk(clk), 
			 .samples(samples[(1<<BITS)-1:(1<<NEWBITS)]), 
			 .subtimes(subtimes_1), 
			 .first_sample(first_sample_1),
			 .last_sample(last_sample_1),
			 .edge_detected(edge_detected_1)
		);		
		always @(posedge clk) begin
			first_sample  <= first_sample_0;
			last_sample   <= last_sample_1;
				
         // edge detected in the later part		
			if(edge_detected_1) begin 
				subtimes      <= {1'b1, subtimes_1};
				edge_detected <= 1;
				
			// different values between both parts
			end else if(!last_sample_0 && first_sample_1) begin
				subtimes      <= {1'b1, subtimes_1};
				edge_detected <= 1;
				
			// edge detected in the earlier part
			end else if(edge_detected_0) begin
				subtimes      <= {1'b0, subtimes_0};
				edge_detected <= 1;
				
			// no edge deteced
			end else begin
				subtimes      <= {1'b0, subtimes_0};
				edge_detected <= 0;
			end
		end
	end
endgenerate


endmodule
