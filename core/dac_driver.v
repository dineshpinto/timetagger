/*
    core of TimeTagger, an OpalKelly based single photon counting library
    Copyright (C) 2012  Helmut Fedder <helmut.fedder@gmail.com>

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

`default_nettype none
`timescale 1ns / 1ps
`define log2(n)   ((n) <= (1<<0) ? 0 : (n) <= (1<<1) ? 1 :\
                   (n) <= (1<<2) ? 2 : (n) <= (1<<3) ? 3 :\
                   (n) <= (1<<4) ? 4 : (n) <= (1<<5) ? 5 :\
                   (n) <= (1<<6) ? 6 : (n) <= (1<<7) ? 7 :\
                   (n) <= (1<<8) ? 8 : (n) <= (1<<9) ? 9 :\
                   (n) <= (1<<10) ? 10 : (n) <= (1<<11) ? 11 :\
                   (n) <= (1<<12) ? 12 : (n) <= (1<<13) ? 13 :\
                   (n) <= (1<<14) ? 14 : (n) <= (1<<15) ? 15 :\
                   (n) <= (1<<16) ? 16 : (n) <= (1<<17) ? 17 :\
                   (n) <= (1<<18) ? 18 : (n) <= (1<<19) ? 19 :\
                   (n) <= (1<<20) ? 20 : (n) <= (1<<21) ? 21 :\
                   (n) <= (1<<22) ? 22 : (n) <= (1<<23) ? 23 :\
                   (n) <= (1<<24) ? 24 : (n) <= (1<<25) ? 25 :\
                   (n) <= (1<<26) ? 26 : (n) <= (1<<27) ? 27 :\
                   (n) <= (1<<28) ? 28 : (n) <= (1<<29) ? 29 :\
                   (n) <= (1<<30) ? 30 : (n) <= (1<<31) ? 31 : 32)

//////////////////////////////////////////////////////////////////////////////////
// DAC driver
//////////////////////////////////////////////////////////////////////////////////

module DacDriver #(
    parameter CLK_DIV=0, // divide clock by ( CLK_DIV + 2 )
    parameter WIDTH=16,
    parameter HOLD=5 // number of clock cycles to hold sync high before next operation
)
(
    input wire                  clk,
    input wire [WIDTH-1:0]      din,
    input wire                  wr_en,
    output reg                  sclk,
    output reg                  sync,
    output wire                 sdout
);

reg [WIDTH-1:0]         dout;
reg                     wr_en_buf;
reg [`log2(WIDTH)-1:0]  clk_ctr;
reg [`log2(WIDTH)-1:0]  state_ctr;

reg [1:0]	state;

////////////////////////////////////////////
// for simulation only, ignored by synthesis
////////////////////////////////////////////
initial begin
    sclk = 0;
    sync = 1;
    dout = 0;
    wr_en_buf   = 0;
    clk_ctr     = 0;
    state_ctr   = 0;
    state       = 0;
end
////////////////////////////////////////////
// simulation code ends here
////////////////////////////////////////////


assign sdout = dout[WIDTH-1];

always @(posedge clk) begin

if ( clk_ctr == 0 ) begin
    clk_ctr <= CLK_DIV;
    sclk <= ~sclk;
end else begin
    clk_ctr <= clk_ctr - 1;
    sclk <= sclk;
end   
    
end


// states
parameter IDLE          = 2'd0;
parameter SHIFT         = 2'd1;
parameter WAIT          = 2'd2;

always @(posedge clk ) begin

if ( clk_ctr == 0 && sclk == 0 ) begin // rising edge of divided clock

    if ( wr_en ) begin // we have just missed wr_en, so store it for next clock
        wr_en_buf <= 1;
    end else begin
    	wr_en_buf <= 0; // we will take care about a set wr_en_buf during this "clock tick", so clear it now 
    end

    case ( state )
    	IDLE: begin // wait for a wr_en
    	    dout <= din;
    	    state_ctr <= WIDTH-1;
    		if ( wr_en_buf ) begin
    		    sync <= 0;
    		    state <= SHIFT;
    		end else begin
    		    sync <= 1;
    		    state <= IDLE;
    		end
    	end
    	SHIFT: begin // left shift dout n-times
    	    if ( state_ctr == 0 ) begin
    	        sync <= 1;
    	        dout <= din;
    	        state_ctr <= HOLD-1;
    	        state <= WAIT;
    	    end else begin
    	    	sync <= 0;
                dout <= dout << 1;
//              dout <= {dout[14:0], 1'b0};
    	        state_ctr <= state_ctr - 1;
    	        state <= SHIFT;
    	    end
    	end
    	WAIT: begin // hold sync high for 5 clock cycles
                sync <= 1;
                dout <= din;
    	    if ( state_ctr == 0 ) begin
    	    	state_ctr <= WIDTH-1;
    		state <= IDLE;
    	    end else begin
    	    	state_ctr <= state_ctr - 1;
    	        state <= WAIT;
    	    end
    	end
    endcase
        
end else begin // if this was not a clock edge of the divided clock, store any wr_en
    if ( wr_en ) begin
        wr_en_buf <= 1;
    end
end

end

endmodule

