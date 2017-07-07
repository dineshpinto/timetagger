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
// sampling the input line
// and correct wrong ordered samples, eg 001011 -> 000111
// Warning: - this module can match one egde multiple times
//          - the "delayer" is only for simulation, the real
//            delayer is mux 
//          - the parameter OVERSAMPLE should be odd
//
//     c[i+1]    cd[i]      c[i]
// input -- delayer -- mux --+-- output (to next stage)
//                           |
//                           +-- xor -- ff (samples[i])
//                                   x[i]
//
//        later <-> early                  
// examples: 'hf00000 <- later raising edge
//           'hfffff0 <- early raising edge
//           'h0fffff <- later falling edge
//           'h00000f <- early falling edge
//////////////////////////////////////////////////////////////////////////////////
module TaggerSampler #(parameter BITS=1, parameter OVERSAMPLE=5)(
	input  wire							clk,
	input  wire 						line,
	output reg	[(1<<BITS)-1:0] 	samples
);

wire [(1<<BITS)+OVERSAMPLE-1:0] c;    	// chain
reg  [(1<<BITS)+OVERSAMPLE-2:0] cd;		// chain_delayed
wire [(1<<BITS)+OVERSAMPLE-2:0] x;		// xor_out
wire  [(1<<BITS)+OVERSAMPLE-2:0] su;   	// samples_uncorrected

assign c[(1<<BITS)+OVERSAMPLE-1] = line;

genvar i;
generate
	for(i=0; i<(1<<BITS)+OVERSAMPLE-1; i=i+1) begin: carry_chain
		MUXCY MUXCY_inst (
			.O(  c[i]), 		// Carry output signal
			.CI(cd[i]), 		// Carry input signal
			.DI(), 				// Data input signal
			.S(1'b1)    		// MUX select, tie to '1' or LUT4 out
		);
		always @(c[i+1]) begin // delayer
			#1 cd[i] <= c[i+1];
		end
		XORCY XORCY_inst (
			.O( x[i]),   		// XOR local output signal
			.CI(c[i]), 			// Carry input signal
			.LI(1'b0) 			// LUT4 input signal
		);		
		
		FD #(.INIT(1'b0) ) FD_inst (
			.Q(su[i]), 		 	// Data output
			.C(clk),    		// Clock input
			.D(x[i])       	// Data input
		);
	end

	integer k;
	integer j;
	for(i=0; i<(1<<BITS); i=i+1) begin: corrector
		always @(posedge clk) begin
			k = 0;
			for(j=0; j<OVERSAMPLE; j=j+1) 
				k = k + su[i+j];
			samples[i] <= k > (OVERSAMPLE/2);
		end
	end
endgenerate

initial begin
	cd <= 0;
	samples <= 0;
end

endmodule
