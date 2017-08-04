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
// convert the input port to timestamps of edges
// Warning: one edge may be detected multiple times
//////////////////////////////////////////////////////////////////////////////////
module TaggerInput #(parameter CHANNELS=1, parameter BITS=1) (
    input  wire								clk,
	 
	 // input port: direct io port
    input  wire [(CHANNELS/2)-1:0] 		line,
	 
	 // output per port
    output wire [BITS*CHANNELS-1:0] 	subtimes,
	 output wire [CHANNELS-1:0] 	  		edge_detected
);

/////////////
// Sampler //
/////////////
wire [(1<<BITS)*(CHANNELS/2)-1:0] samples;

genvar i;
generate
	for(i=0; i<(CHANNELS/2); i=i+1) begin: tagger_sampler
		TaggerSampler #(.BITS(BITS)) tagger_sampler (
			 .clk(clk), 
			 .line(line[i]), 
			 .samples(samples[(1<<BITS)*i+((1<<BITS)-1):(1<<BITS)*i])
		);
	end
endgenerate	 

///////////////////
// BCD-Converter //
///////////////////


// falling and rising edges are switched --> hack to account for inverted LVDS lines
// Helmut Fedder, 2013-02-20
generate
	for(i=0; i<(CHANNELS/2); i=i+1) begin: tagger_bcd_converter
                TaggerBcdConverterFalling #(.BITS(BITS)) falling ( // * these lines switched
			 .clk(clk), 
			 .samples(samples[(1<<BITS)*i+((1<<BITS)-1):(1<<BITS)*i]), 
			 .subtimes(subtimes[i*BITS+BITS-1:i*BITS]), 
			 .edge_detected(edge_detected[i])
		);
                TaggerBcdConverterRaising #(.BITS(BITS)) raising ( // * these lines switched
			 .clk(clk), 
			 .samples(samples[(1<<BITS)*i+((1<<BITS)-1):(1<<BITS)*i]), 
			 .subtimes(subtimes[(i+(CHANNELS/2))*BITS+BITS-1:(i+(CHANNELS/2))*BITS]), 
			 .edge_detected(edge_detected[i+(CHANNELS/2)])
		);
	end
endgenerate	 

endmodule
