`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: University of Stuttgart
// Engineer: Friedemann Reinhard
// 
// Create Date:    20:21:08 01/13/2010 
// Design Name: RAMTag
// Module Name:    ApdProtection 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: module to protect APDs by switching off the laser if the countrate 
// exceeds a certain threshold. 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module ApdProtection(
    input photon,
    input laser,
    input reset,
	 input clk,
    output aomout
    );

	reg [24:0] count;
	reg [24:0] clk_divide;
	reg alarm;
	reg ResetPhotons;
	wire FilteredLaser;
		
	always @(posedge clk) 
		begin 
			clk_divide <= clk_divide +1;
			if(~alarm)
				alarm <= count > 25'h0030000;//25'h0038000
			else
				if(reset)
					alarm <= 0;
		end
	
	always @(posedge photon) 
		begin
			if(ResetPhotons) 
				count <= 25'h0000;
			else
				count <= count	+1;
		end	
	
	always @(posedge clk_divide[22]) 
		begin
			if(ResetPhotons) 
				ResetPhotons = 0;
			else
				ResetPhotons = 1;
		end
		
	assign aomout = laser & (~alarm); //(laser | ForceOn) //laser & ^alarm;
	
endmodule
