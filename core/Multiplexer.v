`timescale 1ns / 1ps
///////////////////////////////////////////////////////////////////////////////
// Company: University of Stuttgart
// Engineer: Nikolas Abt
// 
// Create Date:    29.07.2015
// Design Name: 
// Module Name:    Multiplexer 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 	This is a multiplexer. The input wire data carries a 16bit 
// 					sequence which corresponds to an adress from the AWG. Using 
// 					select, the corresponding value of data[i] is routed to the 
// 					output.
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
///////////////////////////////////////////////////////////////////////////////
module MUX (
	input wire [15:0]	data,
	input wire [3:0]	select,
	output wire			bitstream
	);
	
	reg temp_bitstream;
	assign bitstream = temp_bitstream;
		
	/*
	The first bit has always to be the Data_Select which determines which
	internal registers are filled from the AWG, here 1.
	The next bits are the ones 18...13 from the sequencer index.
	A Load signal latches the preset data into the internal registers.
	This Data_Select is now 0 and determines the next 13 bits for the AWG.
	The next bits are the ones 12...0 for the internal registers.
	The last Load signal latches the data and since Data_Select is 0, the
	loaded sequencer index is passed to the sequencer for execution.
	This could be either done in one call, but also in two calls. 
	Better in two calls.
	*/
	
	// Whenever data or select changes the loop will execute (may switch to select only)
	always @* begin
		case(select)
			//begin with data[1] for correct series in bitstream
			4'b0000: temp_bitstream <= data[1]; 
			4'b0001: temp_bitstream <= data[2];
			4'b0010: temp_bitstream <= data[3];
			4'b0011: temp_bitstream <= data[4];
			4'b0100: temp_bitstream <= data[5];
			4'b0101: temp_bitstream <= data[6];
			4'b0110: temp_bitstream <= data[7];
			4'b0111: temp_bitstream <= data[8];
			4'b1000: temp_bitstream <= data[9];
			4'b1001: temp_bitstream <= data[10];
			4'b1010: temp_bitstream <= data[11];
			4'b1011: temp_bitstream <= data[12];
			4'b1100: temp_bitstream <= data[13];
			4'b1101: temp_bitstream <= data[14];
			4'b1110: temp_bitstream <= data[15];
			4'b1111: temp_bitstream <= data[0];
		endcase
	end
	
endmodule 