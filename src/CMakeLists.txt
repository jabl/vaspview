include_directories (.)

aux_source_directory (. VVSRC)

add_executable(vaspview ${VVSRC})

target_link_libraries (vaspview ${GLLIBS} m ) 
