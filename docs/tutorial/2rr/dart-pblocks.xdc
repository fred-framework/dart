# User Generated miscellaneous constraints


set_property HD.RECONFIGURABLE true [get_cells dart_i/acc_0/inst]
create_pblock pblock_slot_0
add_cells_to_pblock [get_pblocks pblock_slot_0] [get_cells -quiet [list dart_i/acc_0/inst]]
resize_pblock [get_pblocks pblock_slot_0] -add {SLICE_X7Y120:SLICE_X10Y419}
resize_pblock [get_pblocks pblock_slot_0] -add {RAMB18_X1Y48:RAMB18_X1Y167}
resize_pblock [get_pblocks pblock_slot_0] -add {RAMB36_X1Y24:RAMB36_X1Y83}
resize_pblock [get_pblocks pblock_slot_0] -add {DSP48E2_X1Y48:DSP48E2_X1Y167}
#set_property RESET_AFTER_RECONFIG true [get_pblocks pblock_slot_0]
set_property SNAPPING_MODE ON [get_pblocks pblock_slot_0]


set_property HD.RECONFIGURABLE true [get_cells dart_i/acc_1/inst]
create_pblock pblock_slot_1
add_cells_to_pblock [get_pblocks pblock_slot_1] [get_cells -quiet [list dart_i/acc_1/inst]]
#resize_pblock [get_pblocks pblock_slot_1] -add {SLICE_X0Y180:SLICE_X2Y419}
#resize_pblock [get_pblocks pblock_slot_1] -add {RAMB18_X0Y72:RAMB18_X0Y167}
#resize_pblock [get_pblocks pblock_slot_1] -add {RAMB36_X0Y36:RAMB36_X0Y83}

#resize_pblock [get_pblocks pblock_slot_0] -add {SLICE_X3Y180:SLICE_X8Y419}
#resize_pblock [get_pblocks pblock_slot_0] -add {RAMB18_X0Y72:RAMB18_X1Y167}
#resize_pblock [get_pblocks pblock_slot_0] -add {RAMB36_X0Y36:RAMB36_X1Y83}
#resize_pblock [get_pblocks pblock_slot_0] -add {DSP48E2_X0Y72:DSP48E2_X0Y167}

resize_pblock pblock_slot_1 -add {SLICE_X1Y180:SLICE_X4Y419 DSP48E2_X0Y72:DSP48E2_X0Y167 RAMB18_X0Y72:RAMB18_X0Y167 RAMB36_X0Y36:RAMB36_X0Y83} -remove {SLICE_X2Y180:SLICE_X4Y419 DSP48E2_X0Y72:DSP48E2_X0Y167 RAMB18_X0Y72:RAMB18_X0Y167 RAMB36_X0Y36:RAMB36_X0Y83} -locs keep_all
#set_property RESET_AFTER_RECONFIG true [get_pblocks pblock_slot_1]
set_property SNAPPING_MODE ON [get_pblocks pblock_slot_1]


set_property SEVERITY {Warning} [get_drc_checks NSTD-1]
set_property SEVERITY {Warning} [get_drc_checks UCIO-1]
