set(BEGIN_EXAMPLE_REGEX "////[ ]*BEGIN-EXAMPLE")
set(END_EXAMPLE_REGEX "////[ ]*END-EXAMPLE")

function(extract_from_file extracted_files_var src_file)
  set(extracted_files)

  latex_get_output_path(output_dir)

  file(STRINGS ${src_file} src_lines)

  set(active_files)

  foreach(line IN LISTS src_lines)
    if("${line}" MATCHES "////")
      if("${line}" MATCHES "${BEGIN_EXAMPLE_REGEX}")
        string(REGEX MATCH "[^ ]+[ ]*$" out_name "${line}")
        message(STATUS "Generating ${out_name}")
        list(APPEND active_files ${out_name})
        # Remove files to begin appending.
        file(REMOVE ${output_dir}/listing_${out_name})
        list(APPEND extracted_files ${output_dir}/listing_${out_name})
      elseif("${line}" MATCHES "${END_EXAMPLE_REGEX}")
        string(REGEX MATCH "[^ ]+[ ]*$" out_name "${line}")
        list(FIND active_files "${out_name}" out_index)
        if(${out_index} LESS 0)
          message(WARNING "Name mismatch extracting from ${src_file}. ${out_name} never started.")
        else()
          list(REMOVE_AT active_files ${out_index})
        endif()
      endif()
    else()
      foreach(file IN LISTS active_files)
        file(APPEND ${output_dir}/listing_${file} "${line}\n")
      endforeach()
    endif()
  endforeach()

  foreach(unended_file IN LISTS active_files)
    message(WARNING "Out file ${unended_file} started but never ended.")
  endforeach()

  set(${extracted_files_var} ${extracted_files})
endfunction()

function(extract_examples created_files_var)
  set(created_files)

  foreach(src_file ${ARGN})
    extract_from_file(extracted_files ${src_file})
    list(APPEND created_files ${extracted_files})
  endforeach()

  set(${created_files_var} ${created_files} PARENT_SCOPE)
endfunction()
