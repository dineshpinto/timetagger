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
// pulse generator 
// only for testing
//////////////////////////////////////////////////////////////////////////////////
module PulsTester #(parameter LUTS=1, parameter BITS=6)(
    output wire pulse,
	 output wire pulse_short,
	 input wire clk_input
);

reg enable;
reg test1;
reg test2;

(*KEEP="yes"*) wire [LUTS:0]	lut_input;

genvar i;
generate
	for(i=0; i<LUTS; i=i+1) begin: carry_chain
		LUT1 #(
			.INIT(2'b10)  // Specify LUT Contents
		) LUT1_inst (
			.O(lut_input[i+1]),   // LUT general output
			.I0(lut_input[i])  // LUT input
		);
	end
endgenerate


wire [1:0] ffdata;

FD FD_inst (
	.Q(ffdata[0]), 		 	// Data output
	.C(lut_input[LUTS]),    		// Clock input
	.D(!ffdata[1])
);

FD FD_inst2 (
	.Q(ffdata[1]), 		 	// Data output
	.C(!lut_input[LUTS]),    		// Clock input
	.D(ffdata[0])
);
wire checking_for_enable;
XORCY XORCY_inst (
	.O( checking_for_enable),   		// XOR local output signal
	.CI(ffdata[0]), 			// Carry input signal
	.LI(ffdata[1]) 			// LUT4 input signal
);	
XORCY XORCY_inst2 (
	.O( lut_input[0]),   		// XOR local output signal
	.CI(checking_for_enable), 			// Carry input signal
	.LI(enable) 			// LUT4 input signal
);

wire locked;
wire clk;

DCM_SP #(
	.CLKIN_PERIOD(5.0),  // Specify period of input clock
	.CLKOUT_PHASE_SHIFT("NONE"), // Specify phase shift of NONE, FIXED or VARIABLE
	.CLK_FEEDBACK("1X"),  // Specify clock feedback of NONE, 1X or 2X
	.DESKEW_ADJUST("SYSTEM_SYNCHRONOUS"), // SOURCE_SYNCHRONOUS, SYSTEM_SYNCHRONOUS or
													  //   an integer from 0 to 15
	.DLL_FREQUENCY_MODE("HIGH"),  // HIGH or LOW frequency mode for DLL
	.DUTY_CYCLE_CORRECTION("TRUE"), // Duty cycle correction, TRUE or FALSE
	.PHASE_SHIFT(0),     // Amount of fixed phase shift from -255 to 255
	.STARTUP_WAIT("TRUE")   // Delay configuration DONE until DCM LOCK, TRUE/FALSE
) DCM_SP_inst (
	.CLK0(clk),     // 0 degree DCM CLK output
	.LOCKED(locked), // DCM LOCK status output
	.CLKFB(clk),   // DCM clock feedback
	.CLKIN(ffdata[0]),   // Clock input (from IBUFG, BUFG or DCM)
	.PSCLK(0),   // Dynamic phase adjust clock input
	.PSEN(0),     // Dynamic phase adjust enable input
	.PSINCDEC(0), // Dynamic phase adjust increment/decrement
	.RST(!enable)        // DCM asynchronous reset input
);

generate
	if(BITS > 0) begin
		reg [BITS-1:0] divisor;
		always @(posedge clk)
			divisor <= divisor + 1;
		initial
			divisor <= 0;
		assign pulse = divisor[BITS-1];
	end else begin
		assign pulse = clk;
	end
endgenerate

always @(posedge clk) begin
	test2 <= test1;
end

reg [15:0] delayer;

always @(posedge clk_input) begin
	if(!delayer) begin
		if(!locked || (test1 != test2)) begin
			enable <= !enable;
		end
		test1 <= !test1;
	end
	delayer <= delayer + 1;
end

initial begin
	delayer <= 1;
	enable <= 0;
end


MUXCY MUXCY_inst (
	.O(pulse_short),	// Carry output signal
	.CI(!pulse), 		// Carry input signal
	.DI(0), 				// Data input signal
	.S(pulse)    		// MUX select, tie to '1' or LUT4 out
);


endmodule
