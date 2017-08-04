`timescale 1ns / 1ps
///////////////////////////////////////////////////////////////////////////////
// Company: University of Stuttgart
// Engineer: Nikolas Abt
// 
// Create Date:    13.05.2015
// Design Name: 
// Module Name:    ExtensionSingleShot 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: module to prevent drift in single shot readout
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
///////////////////////////////////////////////////////////////////////////////

/*
ExtensionSingleShot
===================
ssr		: Initializes the memories
readout	: Tiggers memory readout
swap		: Switches current_memory from 1 to 0 (or 0 to 1)
photon	: Single photon count
reset		: Resets flipper to 0
flip		: Output of memories, HIGH when memory_one is higher  
testmem	: Stores current_memory (testing only)
*/

module ExtensionSingleShot (
	input wire ssr,
	input wire readout,
	input wire swap,
	input wire photon,
	input wire reset,
	output wire flip,
	output wire testmem
	);
		
	reg [24:0] memory_one;
	reg [24:0] memory_two;
	reg current_memory;
	reg flipper;
	
	// Initialization of memory_one and memory_two by ssr to state 0 and 
	// current_memory to state 1
	always @(posedge ssr or posedge swap) begin
		if (ssr)
			current_memory <= 1'b1;
		else if (swap)
			current_memory <= !(current_memory);
	end
	
	// Carries out the current memory
	assign testmem = current_memory;	
		
	// Check state of current_memory and assign memory accordingly
	always @(posedge ssr or posedge photon or posedge testmem) begin
		if (ssr) begin
			memory_one <= 23'b0;
			memory_two <= 23'b0;
		end
		else if (photon) begin
			if (testmem)
				memory_one <= memory_one + 23'b1;
			else
				memory_two <= memory_two + 23'b1;
		end
	end
	
	// Compare memories and set flipper HIGH is memory_one is larger
	always @(posedge readout or posedge reset) begin
		if (reset)
			flipper <= 1'b0;
		else if (readout) begin
			if (memory_one > memory_two)
				flipper <= 1'b1;		
			else
				flipper <= 1'b0;
		end
	end	

	// Carries output of HIGH or LOW based on flipper
	assign flip = flipper;
	
endmodule 