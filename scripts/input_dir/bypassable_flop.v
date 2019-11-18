module bypassable_flop(
input BBAR,
input DB,
input D,
input CP,
output Q
);

wire int_wire;

MUX2X1 mux_inst (
.A(DB),
.B(int_wire),
.S(BBAR),
.Y(Q)
);

DFFPOSX1 flop_inst (
.D(D),
.Q(int_wire),
.CLK(CP)
);
endmodule

