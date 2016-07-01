
set(example_src
  ArrayHandle.cxx
  ArrayHandleAdapt.cxx
  ArrayHandleCast.cxx
  ArrayHandleCompositeVector.cxx
  ArrayHandleConstant.cxx
  ArrayHandleCoordinateSystems.cxx
  ArrayHandleCounting.cxx
  ArrayHandleDerived.cxx
  ArrayHandleGroupVec.cxx
  ArrayHandleImplicit.cxx
  ArrayHandlePermutation.cxx
  ArrayHandleTransform.cxx
  ArrayHandleZip.cxx
  BasicGlut.cxx
  CellOperations.cxx
  CellShapes.cxx
  ColorTables.cxx
  CoreDataTypes.cxx
  CustomDeviceAdapter.cxx
  DataSetCreation.cxx
  DeviceAdapterTag.cxx
  DeviceAdapterAlgorithms.cxx
  DynamicArrayHandle.cxx
  ErrorHandling.cxx
  ExportMacro.cxx
  FunctionInterface.cxx
  IO.cxx
  ListTags.cxx
  Matrix.cxx
  NewtonsMethod.cxx
  OtherGlut.cxx
  ProvidedFilters.cxx
  ScatterCounting.cxx
  ScatterUniform.cxx
  Timer.cxx
  Traits.cxx
  TransferringArguments.cxx
  TriangleQuality.cxx
  UseWorkletMapCellToPoint.cxx
  UseWorkletMapField.cxx
  UseWorkletMapPointToCell.cxx
  )

# Set up compiling and testing of examples.
if (BUILD_EXAMPLES)
  find_package(VTKm REQUIRED
    COMPONENTS Serial TBB
    )
  include_directories(${VTKm_INCLUDE_DIRS})

  find_package(OpenGL REQUIRED)
  find_package(GLUT REQUIRED)
  include_directories(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR})

  set(test_prog ExampleTests)
  create_test_sourcelist(test_src ${test_prog}.cxx ${example_src})
  add_executable(${test_prog} ${test_src})
  target_link_libraries(${test_prog}
    ${VTKm_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    )

  foreach (test ${example_src})
    get_filename_component(tname ${test} NAME_WE)
    add_test(NAME ${tname}
      COMMAND ${test_prog} ${tname} --no-interaction
      )
  endforeach()

  if(NOT WIN32)
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E
        create_symlink ${CMAKE_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data
      )
  else()
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E
        copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR}/data
      )
  endif()
endif()

include(ExtractExample.cmake)

extract_examples(created_files ${example_src})
add_custom_target(example-listings DEPENDS ${created_files})
