
# PlanAhead Launch Script for Post-Synthesis floorplanning, created by Project Navigator

create_project -name core -dir "/home/dinesh/HiWi/legacy/TimeTagger/timetagger/core/planAhead_run_1" -part xc3s1200eft256-4
set_property design_mode GateLvl [get_property srcset [current_run -impl]]
set_property edif_top_file "/home/dinesh/HiWi/legacy/TimeTagger/timetagger/core/TimeTaggerController.ngc" [ get_property srcset [ current_run ] ]
add_files -norecurse { {/home/dinesh/HiWi/legacy/TimeTagger/timetagger/core} {FrontPanelHDL} {ipcore_dir} }
add_files [list {ipcore_dir/RamFifo_Output.ncf}] -fileset [get_property constrset [current_run]]
add_files [list {ipcore_dir/PurifierBuffer.ncf}] -fileset [get_property constrset [current_run]]
add_files [list {ipcore_dir/RamFifo_Input.ncf}] -fileset [get_property constrset [current_run]]
set_property target_constrs_file "TimeTaggerController.ucf" [current_fileset -constrset]
add_files [list {TimeTaggerController.ucf}] -fileset [get_property constrset [current_run]]
link_design
