`timescale 1ns / 1ps
///////////////////////////////////////////////////////////////////////////////
// Company: University of Stuttgart
// Engineer: Nikolas Abt
// 
// Create Date:    29.07.2015
// Design Name: 
// Module Name:    Bitstream 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description:	module to put out the select_bitstream_part to the shift register in 
//						dependence of the result from the 
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
///////////////////////////////////////////////////////////////////////////////

/*
Bitstream
=========
flip					: Result of comparison of two memories
clock 				: External clock driving output from FPGA 
data_part1			: Highest 5 bits of jump address 
data_part2			: Lower 13 bits of jump address
reset_singleshot	: Resets flipper in ExtensionSingleShot, when all 32 bits are sent
select_bitstream_part			: Streams data_part1/data_part2 to the shift reg.
shift_reg_reset		: Reset the shift_register
latch_clock			: Latch the 16 bit to the output of the shift register
count_it				: Test lowest bit of counter is switching between 0 & 1 
						  with clock (testing only)
*/

module Bitstream #(parameter delta=1) (
	input wire				flip,		
	input wire				clock,	
	input wire [15:0]		data_part1, 
	input wire [15:0]    data_part2,	
	output wire				reset_singleshot,	
	output wire				select_bitstream_part,	
	output wire				latch_clock,	
	output wire 			shift_reg_reset,	
//	output wire	[5:0]		count_it		
	output wire count_it
	);

	reg [4:0]	counter;
	wire [3:0] 	select;
	
	// Taking only 4 bits (out of 5) from counter (1 bit of overflow)
	assign select = counter[3:0];
	assign count_it = counter[4];
	// Only for use with shift register
	reg selector;
	reg latch;
	reg shift_reset;
	
	wire stream_part1;
	wire stream_part2;		
	
	reg	select_stream_part;
	reg	flip_it;

	reg [5:0] temp_count; 
	initial temp_count = 6'b0;


	// Assign all wires which need to be carried out
	assign select_bitstream_part = select_stream_part;
	assign latch_clock = latch;
	assign shift_reg_reset = shift_reset;
	assign reset_singleshot = flip_it;

	// Routes bitstream
	// select is 4 bits (out of 5) from counter
	// ?? select_stream_part is never passed to MUX	
	MUX multiplexer_one (
		.data(data_part1),
		.select(select),
		.bitstream(stream_part1)
	);

	MUX multiplexer_two (
		.data(data_part2),
		.select(select),
		.bitstream(stream_part2)
	);
		
	always @(posedge clock) begin
		if (flip) begin
			// Increase the counter with each clock to load the next entry in data_part1/data_part2
			counter = counter + 5'b1; 
			if (counter <= 5'b01111)
				select_stream_part = 1'b0;
			else 
				select_stream_part = 1'b1;
		end
		else if (!flip) begin
			counter = 5'b11111; //set the counter to maximum (so that first value is 5'b00000) that 
			// matches the first and the second information
			select_stream_part = 1'b0;
			latch = 1'b0; //latch is zero, that means nothing is written on the output of the shift register
			shift_reset = 1'b0; //the shift register is not resetted. happened when counter==5'b11111
			flip_it = 1'b0; //flip was set to zero before when counter=5'b11111
			selector = 1'b0; //selector is zero to start again with first 16bit
			temp_count = 32'b0;
		end
	end
	

	/*always @(negedge clock or posedge clock) begin
		if (flip) begin
			temp_count = temp_count + 6'b1;
			if (temp_count == 6'b011111) begin
				latch = 1'b1;
				shift_reset = 1'b1;
			end
			else if (temp_count == 6'b100000) begin
				latch = 1'b0;
				shift_reset = 1'b0;
			end
		end
		else ;
	end
	*/
endmodule

/*
	always@(posedge clock) begin
		if(flip) begin
			// Increase the counter with each clock to load the next entry in data_part1/two
			counter = counter + 5'b1; 
			case(counter) inside
				5'b00000,5'b00001,5'b00010,5'b00011,5'b00100,
				5'b00101,5'b00110,5'b00111,5'b01000,5'b01001,
				5'b01010,5'b01011,5'b01100,5'b01101,5'b01110:	
					begin
						select_stream_part = stream_part1;
					end
				5'b01111:	
					begin
						select_stream_part = stream_part1;
						//latch = 1'b1;
						//shift_reset = 1'b1;
					end
				5'b10000:	
					begin
						select_stream_part = stream_part2;
						//latch = 1'b0;
						//latch = 1'b0;
						//shift_reset = 1'b0;
					end
				5'b10001,5'b10010,5'b10011,5'b10100,5'b10101,
				5'b10110,5'b10111,5'b11000,5'b11001,5'b11010,
				5'b11011,5'b11100,5'b11101,5'b11110:	
					begin
						select_stream_part = stream_part2;
					end
				5'b11111:	
					begin
						select_stream_part = stream_part2;
						//latch = 1'b1;
						//shift_reset = 1'b1;
					end
			endcase
		end
		else if(~flip) begin
			counter = 5'b11111; //set the counter to maximum (so that first value is 5'b00000) that 
			// matches the first and the second information
			select_stream_part = 1'b0;
			latch = 1'b0; //latch is zero, that means nothing is written on the output of the shift register
			shift_reset = 1'b0; //the shift register is not resetted. happened when counter==5'b11111
			flip_it = 1'b0; //flip was set to zero before when counter=5'b11111
			selector = 1'b0; //selector is zero to start again with first 16bit
			temp_count = 32'b0;
		end
		else begin //if nothing fits always send zero.
			select_stream_part = 1'b0;
		end
	end
*/


	/*
	always@(posedge clock) begin
		if(flip) begin
			counter = counter + 5'b00001; //increase the counter with each clock. An increased higher counter means the next entry in data_part1/two
			if(counter < 5'b01101) begin
				//selector = 1'b0;	//as long as the first 16bit are not send, stream data_part1
				select_stream_part = stream_part1;
			end
			else if(counter == 5'b01101) begin
				select_stream_part = stream_part2;
			end
			else if(counter == 5'b01110) begin
				select_stream_part = stream_part2;
			end
			else if(counter==5'b01111) begin
				latch = 1'b1;
				shift_reset = 1'b1;
				//selector = 1'b1;	//if we reach the 17th value, we want to stream data_part2, select is automatically 0000
			end
			else if(counter > 5'b01111) begin
				select_stream_part = stream_part2;
				latch = 1'b0;
				shift_reset = 1'b0;
			end
			//else if(counter==5'b10001) begin
			//	latch = 1'b0;
			//	shift_reset = 1'b0;
			//end
			else if(counter==5'b11111) begin //when the last entry of data_part2 is reached, the latch signal should be send, 
														//then the reset signal, flip needs to be at first set to one, but then again 
														//needs to be zero, that it does not disturb ExtensionSingleShot. Also the selector 
														//is set to zero for the next round
				latch = 1'b1;
				shift_reset = 1'b1;
				//selector = 1'b0; //maybe shift to if(~flip)
				flip_it = 1'b1; //
				//THEY NEED ALL TO BE SET TO ZERO AFTERWARDS!!!! Should happen when ~flip
			end
			//case(selector) //select which data set to stream
			//	1'b0: select_stream_part = stream_part1;
			//	1'b1: select_stream_part = stream_part2;
			//	default: select_stream_part = 1'b0;
			//endcase
		end
		else if(~flip) begin
			counter = 5'b11111; //set the counter to maximum (so that first value is 5'b00000) that matches the first and the second information
			select_stream_part = 1'b0;
			latch = 1'b0; //latch is zero, that means nothing is written on the output of the shift register
			shift_reset = 1'b0; //the shift register is not resetted. happened when counter==5'b11111
			flip_it = 1'b0; //flip was set to zero before when counter=5'b11111
			selector = 1'b0; //selector is zero to start again with first 16bit
		end
		else begin //if nothing fits always send zero.
			select_stream_part = 1'b0;
		end
	end
	*/