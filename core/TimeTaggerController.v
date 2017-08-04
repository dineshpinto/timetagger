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
///////////////////////////////////////////////////////////////////////////////
// top module of the TimeTagger
// it connects three modules:
// Tagger->Buffer->OpalKelly
///////////////////////////////////////////////////////////////////////////////
module TimeTaggerController #(parameter CHANNELS=8)(
	input	 wire 					clk,		// tagger base clock
	input	 wire [CHANNELS-1:0]	trig_ds_p,	// input line
	input	 wire [CHANNELS-1:0]	trig_ds_n,
   output wire [7:0]          gpio,
	
	// Opal-Kelly Host Interface - USB chip connection
	input	 wire [7:0]	 hi_in,
	output wire [1:0]	 hi_out,
	inout	 wire [15:0] hi_inout,
	output wire			 hi_muxsel,
	output wire			 i2c_sda,
	output wire			 i2c_scl,
	output wire [3:0]	 led,
	
	// SRAM connectors
	input  wire			 sdram_clk,
	output wire [3:0]  sdram_cmd,		
	output wire [1:0]  sdram_ba,
	output wire [12:0] sdram_a,
	inout  wire [15:0] sdram_d,
	output wire 		 sdram_ldqm,
	output wire			 sdram_udqm,
	output wire			 sdram_cke,
	
	//DAC connector
	output wire dac_sclk,
	output wire dac_sync,
	output wire dac_sdout
);


wire [CHANNELS-1:0] trig;
wire [CHANNELS-1:0] trig_out;
wire trig_calibration;
wire [CHANNELS*2*16-1:0] ok_deadtimes;

IBUFDS #(
	.CAPACITANCE("DONT_CARE"),
	.DIFF_TERM("TRUE"),
	.IBUF_DELAY_VALUE("0"),
	.IFD_DELAY_VALUE("AUTO"),
	.IOSTANDARD("DEFAULT")
)

lvds_input[CHANNELS-1:0] (
	.I( trig_ds_p ),
	.IB( trig_ds_n ),
	.O( trig )
);

//////////////////
// global wires //
//////////////////

wire 			tagger_enable;
wire [31:0]	tagger_data;

wire 			buffer_full;
wire			buffer_empty;
reg  [15:0]	buffer_dout;

wire			ok_clk;
wire			ok_read_enable;
wire [15:0] ok_empty_thresh;
wire [15:0] ok_enable_channel;
wire [15:0] ok_enable_calibration;
wire			ok_enable_laser_filter;

wire			dac_wr_en;
wire [31:0]	dac_din;

//==============================

wire			flipper;
wire			testmemory;
wire [15:0]	awg_data_part1;
wire [15:0]	awg_data_part2;

wire 			ssr;
wire 			photon;
wire 			swap;
wire			readout;
wire			reset_flipper;

/*
wire			flipper_reset;
wire			photons;
*/

// Shift register
// added 30.07.2015, by n.abt
wire			stream;
wire			latch_clk;
wire			shift_reset;
// wire			flipper_extension;
// wire [5:0]	count;
wire count;
// assign flipper_extension = flipper;


////////////////
// TimeTagger //
////////////////

TimeTagger #(.CHANNELS(CHANNELS*2)) tagger (
   .trig_clk(clk), 
   .trig_line(trig_out), 
   .write_full(buffer_full), 
   .write_enable(tagger_enable), 
   .write_data(tagger_data), 
   .conf_enable_channel(ok_enable_channel),
   .conf_enable_laser_filter(ok_enable_laser_filter),
   .conf_deadtimes(ok_deadtimes)
);

/////////////////
// Calibration //
/////////////////

PulsTester test_generator (
    .pulse(trig_calibration), 
    .clk_input(clk)
);

genvar i;
generate
	for(i=0; i<CHANNELS; i=i+1) begin: carry_chain
		MUXCY MUXCY_inst (
			.O (trig_out[i]), 				// Carry output signal
			.CI(trig[i]), 						// Carry input signal
			.DI(trig_calibration), 			// Data input signal
			.S(~ok_enable_calibration[i]) // MUX select, tie to '1' or LUT4 out
		);
	end
endgenerate

////////////
// Buffer //
////////////
wire [15:0] buffer_slow_data;
wire			buffer_slow_empty;
wire			buffer_out_full;
wire			buffer_out_empty;
wire [15:0] buffer_out_data;
reg 			buffer_send_empty;
reg			buffer_clear_output;


/*
// small buffer in both clk domain, and convert 32bit to 16bit
OutputBufferSmall output_buffer_small (
  .wr_clk(clk), // input wr_clk
  .rd_clk(ok_clk), // input rd_clk
  .din(tagger_data), // input [31 : 0] din
  .wr_en(tagger_enable), // input wr_en
  .rd_en(!buffer_out_full && !buffer_slow_empty), // input rd_en
  .dout(buffer_slow_data), // output [15 : 0] dout
  .full(), // output full
  .almost_full(buffer_full), // output almost_full
  .empty(buffer_slow_empty) // output empty
);

// big buffer in slow usb clk domain
OutputBuffer output_buffer (
  .clk(ok_clk), // input clk
  .din(buffer_slow_data), // input [15 : 0] din
  .wr_en(!buffer_out_full && !buffer_slow_empty), // input wr_en
	// input rd_en
  .rd_en(ok_read_enable && !buffer_out_empty && !buffer_send_empty), 
  .prog_empty_thresh(ok_empty_thresh), // input [13 : 0] prog_empty_thresh
  .dout(buffer_out_data), // output [15 : 0] dout
  .full(buffer_out_full), // output full
  .almost_full(), // output almost_full
  .empty(buffer_out_empty), // output empty
  .prog_empty(buffer_empty) // output prog_empty
);

assign sdram_cmd = 0;
assign sdram_ba = 0;
assign sdram_a = 0;
assign led = 0;
*/

assign sdram_cke = 1'b1;
assign sdram_ldqm = 1'b0;
assign sdram_udqm = 1'b0;

RamFifo output_buffer (
    .wr_clk(clk), 
    .ram_clk(sdram_clk), 
    .rd_clk(ok_clk), 
    .din(tagger_data), 
    .wr_en(tagger_enable), 
    .rd_en(ok_read_enable && !buffer_out_empty && !buffer_send_empty), 
    .prog_empty_thresh(ok_empty_thresh), 
    .dout(buffer_out_data), 
    .full(), 
    .almost_full(buffer_full), 
    .empty(buffer_out_empty), 
    .prog_empty(buffer_empty), 
    .sdram_cmd(sdram_cmd), 
    .sdram_ba(sdram_ba), 
    .sdram_a(sdram_a), 
    .sdram_d(sdram_d),
	 .led(led)
 );


// if empty, send zeros
always @(*) begin
	if(buffer_clear_output)
		buffer_dout <= 16'h0000;
	else
		buffer_dout <= buffer_out_data;
end

// to be sure, that only multiples of two counts of zeros are send
always @(posedge ok_clk) begin
	if(buffer_send_empty) begin
		buffer_send_empty <= !ok_read_enable;
		buffer_clear_output <= 1;
	end else begin
		buffer_send_empty <= ok_read_enable && buffer_out_empty;
		buffer_clear_output <= ok_read_enable && buffer_out_empty;
	end
end

/////////////////////
// Opal Kelly Host //
/////////////////////
assign hi_muxsel = 1'b0;
assign i2c_sda = 1'bz;
assign i2c_scl = 1'bz;

// Opal Kelly Host Interface internal connections
parameter N = 1; // count of connectors to ok2x
//parameter N = 3; //mod by n.abt, one okWireOut is needed twice
wire [30:0]			ok1;
wire [16:0]			ok2;
wire [17*N-1:0]		ok2x;

// Opal Kelly Host
okHost okHI	(
	.hi_in( hi_in ),
	.hi_out( hi_out ),
	.hi_inout( hi_inout ),
	.ti_clk( ok_clk ),
	.ok1( ok1 ),
	.ok2( ok2 )
);

// Opal Kelly output multiplexer
// N = 3: 1 x okBTPipeOut pipe, 2 x okWireOut
okWireOR #(.N(N)) okOr ( ok2, ok2x );        

// Opal Kelly block throttled output pipe
okBTPipeOut okPipe ( 	
	.ok1( ok1 ),
	.ok2( ok2x[ 0*17 +: 17 ] ),
	.ep_addr( 8'ha0 ),
	.ep_datain( buffer_dout ),    // get data from fifo
	.ep_read( ok_read_enable ),  	// request data
	.ep_blockstrobe( ),
	.ep_ready( !buffer_empty )    // fifo initalizes data transmission
);

// fifo controllable full thresshold
okWireIn ep01 (
	.ok1(ok1), 
	.ep_addr(8'h01),
	.ep_dataout( ok_empty_thresh )
);

okWireIn ep02 (
	.ok1(ok1), 
	.ep_addr(8'h02),
	.ep_dataout( ok_enable_channel )
);

// DAC data word
okWireIn ep03 (
	  .ok1(ok1), 
	  .ep_addr(8'h03),
	  .ep_dataout( dac_din[15:0] )
);
okWireIn ep04 (
	  .ok1(ok1), 
	  .ep_addr(8'h04),
	  .ep_dataout( dac_din[31:16] )
);

// enable laser filter
okWireIn ep05 (
	.ok1(ok1), 
	.ep_addr(8'h05),
	.ep_dataout( ok_enable_laser_filter )
);

// enable calibration
okWireIn ep06 (
        .ok1(ok1), 
        .ep_addr(8'h06),
        .ep_dataout( ok_enable_calibration )
);

// deadtime
generate
	for(i=0; i<CHANNELS*2; i=i+1) begin: deadtimes_loop
		okWireIn deadtime_wire_in (
        .ok1(ok1), 
        .ep_addr(16 + i),
        .ep_dataout( ok_deadtimes[15+16*i:16*i] )
		);
	end
endgenerate

// DAC write trigger
okTriggerIn ep40 (
	.ok1( ok1 ),
	.ep_clk( ok_clk ),
	.ep_addr( 8'h40 ),
	.ep_trigger( dac_wr_en )
);

/*
    DAC Driver for the DAC 7568 / 8168 / 8568
*/

DacDriver #(
    .CLK_DIV(0), // divide clock by two so we have 24 MHz (must be <50 MHz)
    .WIDTH(32),
    .HOLD(3)     // minimum sync high time is 80 ns, we use 125 ns
)
dac_driver (
	.clk(ok_clk), // 48 MHz clock
	.din(dac_din),
	.wr_en(dac_wr_en),
	.sclk(dac_sclk),
	.sync(dac_sync),
	.sdout(dac_sdout)
);

//===================== SSR =======================
//added 17.06.2015 by n.abt

// not needed when flipper_reset is used, but with this flipper can be set to 
// zero and one set the flipper by hand
/*okWireIn FlipWireIn (
	.ok1( ok1 ),
	.ep_addr( 8'h07 ),
	.ep_dataout( flipper )
);*/

//if flipper_reset trigger comes, the flipper wire is set to zero, as well as 
// the memories that count
/*okWireIn FlipResetWireIn (
	.ok1( ok1 ),
	.ep_addr( 8'h09 ),
	.ep_dataout( flipper_reset )
);*/

//read the state of flipper out
/*okWireOut FlipWireOut (
	.ok1( ok1 ),
	.ok2( ok2x[ 1*17 +: 17 ] ),
	.ep_addr( 8'h25 ),
	.ep_datain( flipper )
);*/

//read the state of flipper_reset out
/*okWireOut FlipResetWireOut (
	.ok1( ok1 ),
	.ok2( ok2x[ 2*17 +: 17 ] ),
	.ep_addr( 8'h26 ),
	.ep_datain( flipper_reset )
);*/

//added 30.07.2015, serves for communication with AWG
//provide the data which needs to sent to AWG as an adress in dynamic control. 

okWireIn AWGDataOneWireIn (
	.ok1( ok1 ),
	.ep_addr( 8'h07 ),
	.ep_dataout( awg_data_part1 )
);

okWireIn AWGDataTwoWireIn (
	.ok1( ok1 ),
	.ep_addr( 8'h08 ),
	.ep_dataout( awg_data_part2 )
);


// Null signal for useless outputs
(* keep="soft" *)
wire null_wire;

// Triggers are inverted by default
assign ssr 				= ~trig[0];
assign photon 			= ~trig[1];
assign reset_flipper 	= ~trig[2];
assign readout 		= ~trig[3];
assign swap 			= ~trig[4];

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

ExtensionSingleShot Flipping (
	.ssr(ssr), 
	.readout(readout),
	.swap(swap), 
	.photon(photon), 
	.reset(reset_flipper), 
	.flip(flipper), 	
	.testmem(testmemory) 
);

/*
Bitstream
=========
flip					: Result of comparison of two memories
clock 				: External clock driving output from FPGA 
data_one				: Highest 5 bits of jump address 
data_two				: Lower 13 bits of jump address
reset_ss				: Resets flipper in ExtensionSingleShot, when all 32 bits are sent
bitstream			: Streams data_one/data_two to the shift reg.
shift_reg_reset	: Reset the shift_register
latch_clock			: Latch the 16 bit to the output of the shift register
count_it				: Test lowest bit of counter is switching between 0 & 1 
						  with clock (testing only)
*/
/*
Bitstream BitstreamtoShiftRegister (
	.flip(flipper),
	.clock(ok_clk),
	.data_part1(awg_data_part1),
	.data_part2(awg_data_part2),
	.reset_singleshot(reset_flipper),
	.select_bitstream_part(stream),
	.shift_reg_reset(shift_reset),
	.count_it(count)
);
*/
//////////////////
// gpio         //
//////////////////

// All trig signals are inverted by default
assign gpio[0] = flipper;
//assign gpio[1] = null_wire;
//assign gpio[2] = stream;
//assign gpio[3] = count;

endmodule
