set(BEGIN_EXAMPLE_REGEX "////[ ]*BEGIN-EXAMPLE")
set(END_EXAMPLE_REGEX "////[ ]*END-EXAMPLE")

get_filename_component(ExtractExample_script ${CMAKE_CURRENT_LIST_FILE} ABSOLUTE)

function(add_example_targets src_file created_files_var)
  set(created_files)
  get_filename_component(full_src_file ${src_file} ABSOLUTE)

  file(STRINGS ${src_file} src_lines)

  foreach(line IN LISTS src_lines)
    if("${line}" MATCHES "${BEGIN_EXAMPLE_REGEX}")
      string(REGEX MATCH "[^ ]+[ ]*$" out_name "${line}")
      set(full_out_name ${CMAKE_CURRENT_BINARY_DIR}/${out_name})
      add_custom_command(OUTPUT ${full_out_name}
        COMMAND ${CMAKE_COMMAND}
          -D DO_EXTRACT=ON
          -D INPUT_FILE=${full_src_file}
	  -D EXAMPLE_NAME=${out_name}
          -D OUTPUT_FILE=${full_out_name}
          -P ${ExtractExample_script}
        COMMENT "Creating example ${out_name}"
        )
      set(created_files ${created_files} ${full_out_name})
    endif()
  endforeach()

  set(${created_files_var} ${created_files} PARENT_SCOPE)
endfunction()

if(DO_EXTRACT)
  # If we get this flag, then INPUT_FILE, EXAMPLE_NAME, and OUTPUT_FILE
  # should also be set.  Here we parse the file for the desired script
  file(STRINGS ${INPUT_FILE} src_lines)

  set(inexample)
  set(example_out)
  foreach(line IN LISTS src_lines)
    if(inexample)
      if("${line}" MATCHES "${END_EXAMPLE_REGEX}[ ]*${EXAMPLE_NAME}")
	set(inexample)
      endif()
    endif()

    if(inexample)
      if(NOT "${line}" MATCHES "////")
	set(example_out "${example_out}${line}
")
      endif()
    else()
      if("${line}" MATCHES "${BEGIN_EXAMPLE_REGEX}[ ]*${EXAMPLE_NAME}")
	set(inexample ON)
      endif()
    endif()
  endforeach()

  file(WRITE ${OUTPUT_FILE} "${example_out}")
endif()  
