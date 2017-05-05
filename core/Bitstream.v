`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: University of Stuttgart
// Engineer: Nikolas Abt
// 
// Create Date:    29.07.2015
// Design Name: 
// Module Name:    Bitstream 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description:	module to put out the bitstream to the shift register in dependence
//						of the result from the flipper.
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module Bitstream (
	//inout wire				flip,		//this is the result from the comparison of the two memories.
	input wire				clock,	//external clock, which drives the output from the FPGA to the shift register and the shift_clock of the register
	input wire [15:0]		data_low,	//this contains the highest five bits of the jump adress.
	input wire [15:0] 	data_high,	//this contains the lower thirteen bits of the jump adress.
	input wire				photons,	//incoming photons, needed for ExtensionSingleShot
	input wire				swap,	//this is the laser signal
	input wire				ssr,	//trigger for the ssr measurements
	//input wire				ssr,	//trigger for the ssr measurements
	input wire				readout,	//this signal reads out the memories and leads to high or low on flipper
	output wire				bitstream_out,	//this is the connection to the shift register where the 32 bit are sent.
	output wire				latch_clock,	//the latch_clock latches the full 32bits to the output of the register if they are complete
	output wire 			shift_reg_reset,	//this resets the shift register to make it ready for the next jump adresses
	output wire				shift_clock
	//output wire				testmemory,
	//input wire				clock_reset
	);

	wire flipper;

	reg half_clock;
	reg quarter_clock;
	reg quaver_clock;


	reg [4:0] small_counter;
	reg [15:0] big_counter;
	reg [31:0] fast_counter;
	wire [3:0] select;
	assign select = small_counter[3:0];
	
	reg latch;
	reg shift_reset;

	wire	bitstream_low;
	wire	bitstream_high;
		
	reg	bitstream;
	
	reg reset; // for reset of memories and onset (or not depending on memories) of flipper
	reg testmem;
	reg resetter;
	
	//route out signals
	assign shift_reg_reset = shift_reset;
	assign latch_clock = latch;
	assign shift_clock = half_clock;
	
	ExtensionSingleShot Left_Right_SSR (
		.photon( photons ),
		.swap( swap ),
		.ssr( ssr ),
		.readout( readout ),
		.reset( reset ),
		.flip( flipper )
	);

	//Multiplexer for the selection of bits in data_one and data_two
	MUX #( .N(5) ) multiplexer_low (
		.data( data_low ),
		.select( select ),
		.bitstream( bitstream_low )
	);
	MUX #( .N(5) ) multiplexer_high (
		.data( data_high ),
		.select( select ),
		.bitstream( bitstream_high )
	);
	
	reg [3:0] count;	
	
	always@(posedge clock, posedge readout) 
	begin
		if(readout==1'b1)
		begin
			fast_counter = 32'b0;
			count = 4'b0;
			half_clock = 1'b0;
			quarter_clock = 1'b0;
			quaver_clock = 1'b0;
			reset = 1'b0;
		end
		else begin
			fast_counter <= fast_counter + 1;
			count <= count + 1;
			if(count%1==1'b0) begin
				half_clock <= ~half_clock;
			end
			if(count%2==1'b0) begin
				quarter_clock <= ~quarter_clock;
			end
			if(count%4==1'b0) begin
				quaver_clock <= ~quaver_clock;
			end			
			if(big_counter==16'b0000000001111111)
			begin
				reset = 1'b1;
			end
			if(big_counter==16'b1111111111111111)
			begin
				reset = 1'b0;
			end
			
			if(fast_counter==32'b1111100)
			begin
				latch = 1'b1;
			end
			if(fast_counter==32'b1111101)
			begin
				latch = 1'b0;
			end
			if(fast_counter==32'b1111110)
			begin
				shift_reset = 1'b1;
			end
			if(fast_counter==32'b1111111)
			begin
				shift_reset = 1'b0;
			end
			
			if(fast_counter==32'b11111100)
			begin
				latch = 1'b1;
			end
			if(fast_counter==32'b11111101)
			begin
				latch = 1'b0;
			end
			if(fast_counter==32'b11111110)
			begin
				shift_reset = 1'b1;
			end
			if(fast_counter==32'b11111111)
			begin
				shift_reset = 1'b0;
			end
			
			
		end
	end
	
		
	always@(posedge quaver_clock) 
	begin
		if(~flipper) 
		begin 
			small_counter = 5'b11111;
			bitstream = 1'b0;
			shift_reset = 1'b0;
			latch = 1'b0;
		end
		else if(flipper) 
		begin
			small_counter = small_counter + 5'b1;
			if(small_counter<5'b10000)
			begin
				bitstream = bitstream_low;
			end
			else if(small_counter>=5'b10000)
			begin
				bitstream = bitstream_high;
			end
		end
	end
		
	always@(posedge half_clock) begin
		if(~flipper)
		begin
			big_counter = 16'b1111111111111111; // for quarter clock
		end
		else if(flipper) 
		begin
			big_counter = big_counter + 16'b1; 
		end
		else begin 
			bitstream = 1'b0;
		end
	end
	
	assign bitstream_out = bitstream;

endmodule
