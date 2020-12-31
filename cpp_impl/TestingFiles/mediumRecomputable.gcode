; just a simple test
; should be able to 1/2 print time for this file

G21 ; switch to mm
G28 ; home all axis
G1 X30 Y0 Z0 E10
G1 X30 Y30 Z0 E20
G1 X0 Y30 Z0 E30
G1 X0 Y0 Z0 E40
G1 X40 Y0 Z0
G1 X70 Y0 Z0 E50
G1 X70 Y30 Z0 E60
G1 X40 Y30 Z0 E70
G1 X40 Y0 Z0 E80
