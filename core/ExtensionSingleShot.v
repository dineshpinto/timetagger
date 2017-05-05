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
module ExtensionSingleShot (
	input wire photon,
	input wire swap,
	input wire ssr,
	input wire readout,
	input reset,
	output wire flip,
	output wire testmem,
	output wire resetter
	);
	
	/*
	module ExtensionSingleShot (parameter n_memories) ( 
	add this if there will be more than two memories.
	*/
	
	reg [24:0] memory_one;
	reg [24:0] memory_two;
	
	/*
	reg [24:0] memory [n_memories:0]
	This is the starting point for several memories. If multiple memories are used, one have to think about a new memory swap and diff to decide what to do
	*/
	
	reg [23:0] diff;
	reg swapper;
	reg current_memory;
	
	reg flipper;
	
	/*
	XOR operation. 
	
	current_memory | swapper| "new" current_memory
	------------------------------------------
			0				0		|			0
			0				1		|			1
			1				0		|			1
			1				1		|			0
			
	This operation swaps the current memory in dependence of a swap signal. 
	*/	

	always@(posedge swap, posedge ssr) begin
		if( ssr )
			current_memory <= 1'b1;
		else if ( swap )
			current_memory <= !(current_memory);
	end	
	
	//This is the signal that carries out the current memory
	assign testmem = current_memory;
		
	always@(posedge ssr, posedge photon, posedge testmem) begin
		//if ssr is high, then the memories will be set to zero, to have a proper starting point.
		//The current memory is set to one, because if the first laser swap comes, the memory should be the first one.
		if( ssr ) 
		begin
			memory_one <= 23'b0;
			memory_two <= 23'b0;
			//current_memory <= 1'b1;
		end
		//else if(swap) begin
		//	current_memory <= !(current_memory);
		//end		
		else
		begin
			//When the laser is on, the memory will be swapped to the other one.
			/*if( swap )
			begin
				current_memory <= !(current_memory);
			end*/
			//If a photon comes, the respective memories will be increased.
			if( photon )
			begin
				if(testmem)
					memory_two <= memory_two + 23'b1;
				else
					memory_one <= memory_one + 23'b1;
				//if(current_memory == 1'b0)
				//	memory_one <= memory_one + 23'b1;
				//else if(current_memory == 1'b1)
				//	memory_two <= memory_two + 23'b1;
			end
		end
	end
	
	/*
	//This is another realization, without the possibility of reset with ssr.
	always@(posedge swap) begin
			current_memory = !(current_memory);
	end	
	always@(posedge photon) begin
			if(current_memory == 1'b0)
				memory_one <= memory_one + 23'b1;
			else if(current_memory == 1'b1)
				memory_two <= memory_two + 23'b1;
	end
	*/
	
	assign resetter = reset;
	
	//When the readout comes, we compare the two memories. When the first memory is higher, then a signal will be sent.
	always @(posedge readout, posedge reset) begin
		//diff = memory_one - memory_two;
		if(readout) begin
			if( memory_two < memory_one )
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