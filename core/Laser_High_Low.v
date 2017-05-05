`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////////
module LaserHighLow (
	input wire photon,
	input wire readout,
	input reset,
	output wire flip
	);
	
	/*
	module ExtensionSingleShot (parameter n_memories) ( 
	add this if there will be more than two memories.
	*/
	
	reg [24:0] memory;
	
	reg flipper;

	
		
	always@(posedge photon) begin
		memory <= memory + 24'b1;
	end
	
	assign resetter = reset;
	
	//When the readout comes, we compare the two memories. When the first memory is higher, then a signal will be sent.
	always @(posedge readout, posedge reset) begin
		//diff = memory_one - memory_two;
		if(readout) begin
			if( memory > 24'b111111111111 )
				flipper = 1'b1;
			else
				flipper = 1'b0;
		end
		if(reset)
			flipper = 1'b0;
	end	
	
	//This is the output depending on the memories.
	assign flip = flipper;
	
endmodule 