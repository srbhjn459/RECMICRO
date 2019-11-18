module netlist_linear(IN1, IN2, IN3, IN4, CLOCK, OUT );
  input IN1,IN2,IN3,IN4,CLOCK;
  output OUT;
  wire w0,w1,w2,w3,w4,w5,w6,w7,w8,w9;
  XOR2X1 U1 (.A(IN1), .B(IN2), .Y(w1));
  XOR2X1 U2 (.A(IN3), .B(IN4), .Y(w0));
  DFFPOSX1 FF1 (.D(w1), .CLK(CLOCK), .Q(w3));
  DFFPOSX1 FF2 (.D(w0), .CLK(CLOCK), .Q(w2));
  INVX1 U3 (.A(w2), .Y(w4));
  NOR2X1 U4 (.A(w3), .B(w4), .Y(w6));
  INVX1 U5 (.A(w3), .Y(w5));
  DFFPOSX1 FF3 (.D(w5), .CLK(CLOCK), .Q(w7));
  DFFPOSX1 FF4 (.D(w6), .CLK(CLOCK), .Q(w8));
  XOR2X1 U6 (.A(w7), .B(w8), .Y(w9));
  DFFPOSX1 FF5 (.D(w9), .CLK(CLOCK), .Q(OUT));
endmodule
