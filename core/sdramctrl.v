//------------------------------------------------------------------------
// sdramctrl.v
//
// This is a simple SDRAM controller that provides fullpage read and 
// write capability.  Autorefresh cycles are added to each page access
// to guarantee that enough refresh cycles are completed for the memory
// to stay fresh.
//
// During idle time, autorefresh cycles are also performed.
//
// IMPORTANT NOTE: This controller is provided free of charge from
// Opal Kelly Incorporated.  This controller comes with NO GUARANTEES
// of any kind (including any warranty of the suitability of a particular
// purpose).
//------------------------------------------------------------------------
// tabstop 3
// Copyright (c) 2005-2007 Opal Kelly Incorporated
// $Rev: 318 $ $Date: 2007-08-31 16:03:04 -0700 (Fri, 31 Aug 2007) $
//------------------------------------------------------------------------

`default_nettype none
`timescale 1ns / 1ps
module sdramctrl(
	input  wire        clk,
	input  wire        clk_read,
	input  wire        reset,

	input  wire        cmd_pagewrite,
	input  wire        cmd_pageread,
	output reg         cmd_ack,
	output reg         cmd_done,
	input  wire [14:0] rowaddr_in,
	input  wire [15:0] fifo_din,
	output reg  [15:0] fifo_dout,
	output reg         fifo_write,
	output reg         fifo_read,

	output reg  [3:0]  sdram_cmd,		// cs_n, ras_n, cas_n, we_n}
	output reg  [1:0]  sdram_ba,
	output reg  [12:0] sdram_a,
	inout  wire [15:0] sdram_d
	);

// synthesis attribute iob sdram_ba is "true";
// synthesis attribute iob sdram_a is "true";
// synthesis attribute iob sdram_dout is "true";
// synthesis attribute iob sdram_cmd is "true";
// synthesis attribute iob sdram_dir is "true";
// synthesis attribute iob fifo_dout is "true";


// Refresh cycle.  8192 AUTO REFRESH commands must be delivered every
// 64ms.  Distributing these means that one must be issued every 
// 7.81us.  At a 100MHz clock, that's 781 cycles.
parameter REFRESH_CYCLE = 10'd750;

// Default mode: 
// Burst length = Full page
// Burst type = Sequential
// CAS latency = 2 (for Micron -7E devices)
// Operating mode = Standard
// Write burst mode = Programmed burst length
//parameter MODE_DEFAULT = 13'b0000000100111;
parameter MODE_DEFAULT = 13'b0000000110111; //CAS=3

// Delay timings.  Most of these are specified in ns on the Micron 
// datasheet.  They are converted to clock cycles here for a
// 100 MHz clock frequency.
parameter CNT_tRP   = 4'd1;
parameter CNT_tRFC  = 4'd9;
parameter CNT_tMRD  = 4'd1;
parameter CNT_tWR   = 4'd4;
parameter CNT_tCAS  = 4'd2;
parameter CNT_tINIT = 16'd17500;
parameter CMD_INHIBIT         = 4'b1000,
          CMD_NOP             = 4'b0111,
          CMD_ACTIVE          = 4'b0011,
          CMD_READ            = 4'b0101,
          CMD_WRITE           = 4'b0100,
          CMD_BURSTTERMINATE  = 4'b0110,
          CMD_PRECHARGE       = 4'b0010,
          CMD_AUTOREFRESH     = 4'b0001,
          CMD_LOADMODE        = 4'b0000;

reg [15:0] sdram_dout;
reg        sdram_dir;
assign sdram_d = (sdram_dir==1'b0) ? (sdram_dout) : (16'bz);

// Initialization counter
reg [15:0] cINIT;

// Counter for various delay timings.
reg [3:0] cWAIT;

// Transaction counter.
reg [8:0] cTX;

// Refresh timer.
reg [9:0] cREFRESH_TIMER;
reg [4:0] cREFRESH_COUNT;

// Location addressed by memory transactions.
reg [14:0] rowaddr;
reg        cmd_refresh;


parameter [5:0]
          s_idle         = 6'd0,
          s_reset        = 6'd1,
          s_reset2       = 6'd2,
          s_reset3       = 6'd3,
          s_reset4       = 6'd4,
          s_reset5       = 6'd5,
          s_reset6       = 6'd6,
          
          s_loadmode     = 6'd7,
          s_loadmode2    = 6'd8,
          
          s_blockwrite   = 6'd9,
          s_blockwrite1  = 6'd10,
          s_blockwrite2  = 6'd11,
          s_blockwrite3  = 6'd12,
          s_blockwrite4  = 6'd13,
          s_blockwrite5  = 6'd14,
          s_blockwrite6  = 6'd15,
          s_blockwrite7  = 6'd16,
          s_blockwrite8  = 6'd17,
          
          s_blockread    = 6'd18,
          s_blockread1   = 6'd19,
          s_blockread2   = 6'd20,
          s_blockread3   = 6'd21,
          s_blockread4   = 6'd22,
          s_blockread5   = 6'd23,
          s_blockread6   = 6'd24,
          s_blockread7   = 6'd25,
          s_blockread8   = 6'd26,
          
          s_autorefresh  = 6'd27,
          s_autorefresh1 = 6'd28,
          s_autorefresh2 = 6'd29,
          s_autorefresh3 = 6'd30,
          
          s_init         = 6'd31,
          s_init2        = 6'd32;
reg [5:0] state;
always @(posedge clk_read) begin
	fifo_dout <= sdram_d;
end
always @(posedge clk) begin
	if (reset == 1'b1) begin
		state <= s_init;
		fifo_read <= 1'b0;
		fifo_write <= 1'b0;
		cmd_ack <= 1'b0;
		cmd_done <= 1'b0;
		cmd_refresh <= 1'b0;
		cREFRESH_COUNT <= 1;
		cREFRESH_TIMER <= REFRESH_CYCLE;
		cTX <= 0;
		cWAIT <= 0;
		rowaddr <= 0;
		
		sdram_cmd <= CMD_INHIBIT;
		sdram_ba <= 0;
		sdram_a <= 0;
		sdram_dir <= 0;
	end else begin
		fifo_read <= 1'b0;
		fifo_write <= 1'b0;
		sdram_dout <= fifo_din;
		sdram_dir <= 1'b0;
		cmd_done <= 1'b0;
		cmd_ack <= 1'b0;
		cmd_refresh <= 1'b0;

		// Keep the refresh counter going until it expires.
		if (cREFRESH_TIMER == 0) begin
			cmd_refresh <= 1'b1;
		end else begin
			cREFRESH_TIMER <= cREFRESH_TIMER - 1;
		end
		
		case (state)
			s_idle: begin
				sdram_cmd <= CMD_INHIBIT;
				state <= s_idle;

				// When the refresh timer expires, perform an auto refresh.
				if (cmd_refresh == 1'b1) begin
					cREFRESH_COUNT <= 5'd1;
					cmd_ack <= 1'b0;
					state <= s_autorefresh;
				end else if (cmd_pagewrite == 1'b1) begin
					cmd_ack <= 1'b1;
					rowaddr <= rowaddr_in;
					state <= s_blockwrite;
				end else if (cmd_pageread == 1'b1) begin
					cmd_ack <= 1'b1;
					rowaddr <= rowaddr_in;
					state <= s_blockread;
				end
			end


			// --- INIT ----------------------------------------------------
			s_init: begin
				sdram_cmd <= CMD_INHIBIT;
				sdram_a <= 13'b0010000000000;
				sdram_ba <= 2'b00;
				cINIT <= CNT_tINIT;
				state <= s_init2;
			end
			
			// Wait to satisfy tINIT (>100us).
			s_init2: begin
				cINIT <= cINIT - 1;
				if (cINIT == 0)
					state <= s_reset;
				else
					state <= s_init2;
			end
			
			// --- RESET ----------------------------------------------------
			// Send PRECHARGE to all banks.
			s_reset: begin
				sdram_cmd <= CMD_PRECHARGE;
				sdram_a <= 13'b0010000000000;
				sdram_ba <= 2'b00;
				cWAIT <= CNT_tRP;
				state <= s_reset2;
			end
			
			// Wait to satisfy tRP.
			s_reset2: begin
				sdram_cmd <= CMD_NOP;
				cWAIT <= cWAIT - 1;
				if (cWAIT == 0)
					state <= s_reset3;
				else
					state <= s_reset2;
			end
			
			// Send AUTO REFRESH.
			s_reset3: begin
				sdram_cmd <= CMD_AUTOREFRESH;
				cWAIT <= CNT_tRFC;
				state <= s_reset4;
			end

			// Wait to satisfy tRFC.
			s_reset4: begin
				sdram_cmd <= CMD_NOP;
				cWAIT <= cWAIT - 1;
				if (cWAIT == 0)
					state <= s_reset5;
				else
					state <= s_reset4;
			end

			// Send AUTO REFRESH.
			s_reset5: begin
				sdram_cmd <= CMD_AUTOREFRESH;
				cREFRESH_TIMER <= REFRESH_CYCLE;
				cWAIT <= CNT_tRFC;
				state <= s_reset6;
			end
			
			// Wait to satisfy tRFC.
			s_reset6: begin
				sdram_cmd <= CMD_NOP;
				cWAIT <= cWAIT - 1;
				if (cWAIT == 0)
					state <= s_loadmode;
				else
					state <= s_reset6;
			end

			// Send the LOAD MODE command.
			s_loadmode: begin
				sdram_cmd <= CMD_LOADMODE;
				sdram_a <= MODE_DEFAULT;
				cWAIT <= CNT_tMRD;
				state <= s_loadmode2;
			end
			
			// Wait to satisfy tMRD.
			s_loadmode2: begin
				sdram_cmd <= CMD_NOP;
				cWAIT <= cWAIT - 1;
				if (cWAIT == 0)
					state <= s_idle;
				else
					state <= s_loadmode2;
			end

			// --- AUTO REFRESH ---------------------------------------------
			s_autorefresh: begin
				sdram_cmd <= CMD_PRECHARGE;
				sdram_a <= 13'b0010000000000;
				sdram_ba <= 2'b00;
				cWAIT <= CNT_tRP;
				state <= s_autorefresh1;
			end
			
			// Wait to satisfy tRP.
			s_autorefresh1: begin
				sdram_cmd <= CMD_NOP;
				cWAIT <= cWAIT - 1;
				if (cWAIT == 0)
					state <= s_autorefresh2;
				else
					state <= s_autorefresh1;
			end

			// Send AUTO REFRESH.
			s_autorefresh2: begin
				sdram_cmd <= CMD_AUTOREFRESH;
				cREFRESH_COUNT <= cREFRESH_COUNT - 1;
				cREFRESH_TIMER <= REFRESH_CYCLE;
				cWAIT <= CNT_tRFC;
				state <= s_autorefresh3;
			end
			
			// Wait to satisfy tRFC.
			s_autorefresh3: begin
				sdram_cmd <= CMD_NOP;
				cWAIT <= cWAIT - 1;
				if (cWAIT == 0) begin
					if (cREFRESH_COUNT == 0)
						state <= s_idle;
					else
						state <= s_autorefresh2;
				end else begin
					state <= s_autorefresh3;
				end
			end
			
			
			// --- BLOCK WRITE ----------------------------------------------
			
			// Send the ACTIVE command
			s_blockwrite: begin
				cTX <= 9'd511;
				sdram_cmd <= CMD_ACTIVE;
				sdram_ba <= rowaddr[14:13];
				sdram_a <= rowaddr[12:0];
				fifo_read <= 1'b1;
				state <= s_blockwrite2;
			end
			
			// Send NOP to satisfy tRCD.
			s_blockwrite2: begin
				sdram_cmd <= CMD_NOP;
				fifo_read <= 1'b1;
				state <= s_blockwrite3;
			end

			// Send a WRITE with AUTO PRECHARGE
			// First FIFO data is available here.
			s_blockwrite3: begin
				sdram_cmd <= CMD_WRITE;
				sdram_ba <= rowaddr[14:13];
				sdram_a <= {4'b0010, 9'd0};
				fifo_read <= 1'b1;
				cTX <= cTX - 1;
				state <= s_blockwrite4;
			end

			// Send NOP until the burst is complete.
			s_blockwrite4: begin
				sdram_cmd <= CMD_NOP;
				fifo_read <= 1'b1;
				cTX <= cTX - 1;
				if (cTX == 2)
					state <= s_blockwrite5;
				else
					state <= s_blockwrite4;
			end
			
			// Second to last WRITE in the burst.
			s_blockwrite5: begin
				sdram_cmd <= CMD_NOP;
				state <= s_blockwrite6;
			end

			// Last WRITE in the burst.
			s_blockwrite6: begin
				sdram_cmd <= CMD_NOP;
				state <= s_blockwrite7;
			end

			// Terminate the full-page burst.
			s_blockwrite7: begin
				sdram_cmd <= CMD_BURSTTERMINATE;
				state <= s_blockwrite8;
			end

			// Wait to satisfy tWR.
			// Perform an AUTO REFRESH when we finish.
			s_blockwrite8: begin
				sdram_cmd <= CMD_NOP;
				cREFRESH_COUNT <= 5'd1;
				state <= s_autorefresh;
				cmd_done <= 1'b1;
			end

			// --- BLOCK READ -----------------------------------------------
			
			// Send the ACTIVE command
			s_blockread: begin
				cTX <= 9'd511;
				sdram_cmd <= CMD_ACTIVE;
				sdram_ba <= rowaddr[14:13];
				sdram_a <= rowaddr[12:0];
				sdram_dir <= 1'b1;
				state <= s_blockread2;
			end
			
			// Send NOP to satisfy tRCD.
			s_blockread2: begin
				sdram_cmd <= CMD_NOP;
				sdram_dir <= 1'b1;
				state <= s_blockread3;
			end
			
			// Send a READ with AUTO PRECHARGE
			s_blockread3: begin
				sdram_cmd <= CMD_READ;
				sdram_ba <= rowaddr[14:13];
				sdram_a <= {4'b0010, 9'd0};
				sdram_dir <= 1'b1;
				cTX <= 9'd511;
				cWAIT <= CNT_tCAS;
				state <= s_blockread4;
			end
			
			// CAS wait
			s_blockread4: begin
				sdram_cmd <= CMD_NOP;
				sdram_dir <= 1'b1;
				cWAIT <= cWAIT - 1;
				if (cWAIT == 0) begin
					state <= s_blockread5;
				end else begin
					state <= s_blockread4;
				end
			end

			// First read is available here
			s_blockread5: begin
				sdram_cmd <= CMD_NOP;
				sdram_dir <= 1'b1;
				fifo_write <= 1'b1;
				cTX <= cTX - 1;
				if (cTX == 9'd3)
					state <= s_blockread6;
				else
					state <= s_blockread5;
			end
			
			// Send BURST TERMINATE.
			s_blockread6: begin
				sdram_cmd <= CMD_BURSTTERMINATE;
				sdram_dir <= 1'b1;
				fifo_write <= 1'b1;
				cTX <= cTX - 1;
				state <= s_blockread7;
			end
			
			// Send NOP.  Second to last read available here.
			s_blockread7: begin
				sdram_cmd <= CMD_NOP;
				sdram_dir <= 1'b1;
				fifo_write <= 1'b1;
				state <= s_blockread8;
			end
			
			// Send NOP.  Last read available here.
			// Run an AUTO REFRESH cycle after the block read.
			s_blockread8: begin
				sdram_cmd <= CMD_NOP;
				sdram_dir <= 1'b1;
				fifo_write <= 1'b1;
				cmd_done <= 1'b1;
				cREFRESH_COUNT <= 5'd1;
				state <= s_autorefresh;
			end
			
		endcase
	end
end

endmodule
