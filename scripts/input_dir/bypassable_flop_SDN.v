module bypassable_flop_SDN(
input BBAR,
input DB,
input D,
input CP,
input SDN,
output Q
);

wire int_wire;

MUX2D0BWP mux_inst (
.A(DB),
.B(int_wire),
.S(BBAR),
.Y(Q)
);

DFFSR flop_inst (
.D(D),
.Q(int_wire),
.R(1'b1),
.S(SDN),
.CLK(CP)
);
endmodule

