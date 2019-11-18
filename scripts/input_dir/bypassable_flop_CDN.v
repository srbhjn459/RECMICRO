module bypassable_flop_CDN(
input BBAR,
input DB,
input D,
input CP,
input CDN,
output Q
);

wire int_wire;

MUX2X1 mux_inst (
.A(DB),
.B(int_wire),
.S(BBAR),
.Y(Q)
);


DFFSR flop_inst (
.D(D),
.Q(int_wire),
.R(CDN),
.S(1'b1),
.CLK(CP)
);
endmodule

