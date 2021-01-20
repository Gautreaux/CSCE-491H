; just a simple test
; should be able to 1/2 print time for this file (8 depth)
; but the way you reach that 1/2 is a little more complicated
; this shape is 4 30mm squares in a checkerboard style pattern

G21 ; switch to mm
G28 ; home all axis
G1 X30 Y60 Z0 E10
G1 X30 Y90 Z0 E20
G1 X60 Y90 Z0 E30
G1 X60 Y60 Z0 E40
G1 X60 Y30 Z0 E50
G1 X90 Y30 Z0 E60
G1 X90 Y60 Z0 E70
G1 X90 Y90 Z0 E80
G1 X120 Y90 Z0 E90
G1 X120 Y60 Z0 E100
G1 X120 Y30 Z0 E110
G1 X150 Y30 Z0 E120
G1 X150 Y60 Z0 E130
G1 X120 Y60 Z0 E140
G1 X90 Y60 Z0 E150
G1 X60 Y60 Z0 E160
G1 X30 Y60 Z0 E170
