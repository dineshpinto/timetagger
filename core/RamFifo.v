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
// virtual fifo
// build up of tho xilinx fifos and a sdram interface
// input_fifo -> sdram -> output_fifo
//////////////////////////////////////////////////////////////////////////////////
module RamFifo(
	input  wire 			wr_clk,
	input  wire				ram_clk,
	input  wire 			rd_clk,
	input  wire [31 : 0] din,
	input  wire				wr_en,
	input  wire				rd_en,
	input  wire [10 : 0] prog_empty_thresh,
	output wire [15 : 0] dout,
	output wire				full,
	output wire				almost_full,
	output wire				empty,
	output wire				prog_empty,
	output wire [3:0]  	sdram_cmd,		
	output wire [1:0]  	sdram_ba,
	output wire [12:0] 	sdram_a,
	inout  wire [15:0] 	sdram_d,
	output wire [3:0]		led
);

///////////////
// Variables //
///////////////

reg  [02:0] state;
reg  [14:0] write_pos;
reg  [14:0] read_pos;

// states
parameter [2:0]
	s_init   = 3'd0,
	s_empty  = 3'd1,
	s_idle   = 3'd2,
	s_full   = 3'd3,
	s_read1  = 3'd4,
	s_read2  = 3'd5,
	s_write1 = 3'd6,
	s_write2 = 3'd7;
	
/////////
// DCM //	
/////////

wire dcm_locked;
wire dcm_clk_180;
wire dcm_clk;

assign dcm_clk = ~dcm_clk_180;

dcm_sys ram_dcm (
    .CLKIN_IN(ram_clk), 
    .RST_IN(0), 
    .CLKIN_IBUFG_OUT(), 
    .CLK0_OUT(dcm_clk_180), 
    .LOCKED_OUT(dcm_locked)
    );
	 

///////////
// Fifos //
///////////

wire ififo_rd_en;
wire [15:0] ififo_dout;
wire ififo_empty;
wire ififo_prog_empty;

wire ofifo_wr_en;
wire [15:0] ofifo_din;
wire ofifo_full;
wire ofifo_almost_full;
wire ofifo_prog_full;

RamFifo_Input input_fifo (
  .rst(!dcm_locked),
  .wr_clk(wr_clk), // input wr_clk
  .rd_clk(dcm_clk), // input rd_clk
  .din(din), // input [31 : 0] din
  .wr_en(wr_en), // input wr_en
  .rd_en(ififo_rd_en), // input rd_en
  .dout(ififo_dout), // output [15 : 0] dout
  .full(full), // output full
  .almost_full(almost_full), // output almost_full
  .empty(ififo_empty), // output empty
  .prog_empty(ififo_prog_empty) // output prog_empty
);

RamFifo_Output output_fifo (
  .wr_clk(dcm_clk), // input wr_clk
  .rd_clk(rd_clk), // input rd_clk
  .din(ofifo_din), // input [15 : 0] din
  .wr_en(ofifo_wr_en), // input wr_en
  .rd_en(rd_en), // input rd_en
  .prog_empty_thresh(prog_empty_thresh), // input [10 : 0] prog_empty_thresh
  .dout(dout), // output [15 : 0] dout
  .full(ofifo_full), // output full
  .almost_full(ofifo_almost_full), // output almost_full
  .empty(empty), // output empty
  .prog_full(ofifo_prog_full), // output prog_full
  .prog_empty(prog_empty) // output prog_empty
);


//////////////////////
// SDRAM Controller //
//////////////////////

reg  cmd_pagewrite;
reg  cmd_pageread;
wire cmd_ack;
wire cmd_done;
reg  [14:0] ram_rowaddr;


sdramctrl ramctrl (
    .clk(dcm_clk), 
    .clk_read(dcm_clk), 
    .reset(~dcm_locked), 
    .cmd_pagewrite(cmd_pagewrite), 
    .cmd_pageread(cmd_pageread), 
    .cmd_ack(cmd_ack), 
    .cmd_done(cmd_done), 
    .rowaddr_in(ram_rowaddr), 
    .fifo_din(ififo_dout), 
    .fifo_dout(ofifo_din), 
    .fifo_write(ofifo_wr_en), 
    .fifo_read(ififo_rd_en), 
    .sdram_cmd(sdram_cmd), 
    .sdram_ba(sdram_ba), 
    .sdram_a(sdram_a), 
    .sdram_d(sdram_d)
    );


////////////
// States //
////////////
assign led[3:0] = ~{1'b0, state};

always @(posedge dcm_clk) begin
	cmd_pagewrite <= 0;
	cmd_pageread  <= 0;
	ram_rowaddr   <= 0;
	
	if(!dcm_locked) begin
		state <= s_init;
	end else begin	
		case (state)
			s_init: begin
				write_pos <= 0;
				read_pos <= 0;
				if(dcm_locked)
					state <= s_empty;
				else
					state <= s_init;
			end
			
			s_empty: begin
				if(ififo_prog_empty)
					state <= s_empty;
				else
					state <= s_write1;
			end
			
			s_idle: begin
				if(ififo_prog_empty) begin
					if(ofifo_prog_full)
						state <= s_idle;
					else
						state <= s_read1;
				end else
					state <= s_write1;
			end
			
			s_full: begin
				if(ofifo_prog_full)
					state <= s_full;
				else
					state <= s_read1;
			
			end
			
			s_read1: begin
				cmd_pageread  <= 1;
				ram_rowaddr   <= read_pos;
				if(cmd_ack) begin
					state <= s_read2;
					read_pos <= read_pos + 1;
				end else begin
					state <= s_read1;
				end
			end
			
			s_read2: begin
				if(cmd_done) begin
					if(read_pos == write_pos)
						state <= s_empty;
					else
						state <= s_idle;
				end else begin
					state <= s_read2;
				end
			end
			
			s_write1: begin
				cmd_pagewrite <= 1;
				ram_rowaddr   <= write_pos;
				if(cmd_ack) begin
					state <= s_write2;
					write_pos <= write_pos + 1;
				end else begin
					state <= s_write1;
				end
			
			end
			
			s_write2: begin
				if(cmd_done) begin
					if(read_pos == write_pos)
						state <= s_full;
					else
						state <= s_idle;
				end else begin
					state <= s_write2;
				end
			
			end
		endcase
	end
end

endmodule
