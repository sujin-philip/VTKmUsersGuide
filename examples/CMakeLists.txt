
set(example_src
  ArrayHandle.cxx
  ArrayHandleAdapt.cxx
  ArrayHandleCast.cxx
  ArrayHandleCompositeVector.cxx
  ArrayHandleConstant.cxx
  ArrayHandleCoordinateSystems.cxx
  ArrayHandleCounting.cxx
  ArrayHandleDerived.cxx
  ArrayHandleDiscard.cxx
  ArrayHandleGroupVec.cxx
  ArrayHandleImplicit.cxx
  ArrayHandlePermutation.cxx
  ArrayHandleTransform.cxx
  ArrayHandleZip.cxx
  BasicGlut.cxx
  CellEdgesFaces.cxx
  CellOperations.cxx
  CellShapes.cxx
  ColorTables.cxx
  CoreDataTypes.cxx
  CustomDeviceAdapter.cxx
  DataSetCreation.cxx
  DeviceAdapterTag.cxx
  DeviceAdapterAlgorithms.cxx
  DynamicArrayHandle.cxx
  EnvironmentModifierMacros.cxx
  ErrorHandling.cxx
  FunctionInterface.cxx
  IO.cxx
  ListTags.cxx
  Matrix.cxx
  NewtonsMethod.cxx
  OtherGlut.cxx
  ProvidedFilters.cxx
  ScatterCounting.cxx
  ScatterUniform.cxx
  SumOfAngles.cxx
  SimpleHistogram.cxx
  Timer.cxx
  Traits.cxx
  TransferringArguments.cxx
  TriangleQuality.cxx
  UsePointElevationWorklet.cxx
  UseWorkletMapCellToPoint.cxx
  UseWorkletMapField.cxx
  UseWorkletMapPointToCell.cxx
  )

# Set up compiling and testing of examples.
if (BUILD_EXAMPLES)
  find_package(VTKm REQUIRED
    COMPONENTS Serial TBB Rendering OpenGL GLUT
    )
  include_directories(${VTKm_INCLUDE_DIRS})

  if(VTKm_VERSION_FULL MATCHES "[0-9]+.[0-9]+.[0-9]+-.*")
    # If we are in between VTK-m versions, change the version to give the
    # commit we are currently documenting.
    set(VTKm_GUIDE_VERSION ${VTKm_VERSION_FULL} PARENT_SCOPE)
  else()
    # If we are at an actual release (as far as we can tell), make sure the
    # version matches up.
    if(NOT VTKm_GUIDE_VERSION STREQUAL VTKm_VERSION)
      message(SEND_ERROR "VTKm_GUIDE_VERSION seems to be set wrong. Change in CMakeLists.txt")
    endif()
  endif()

  set(test_prog ExampleTests)
  create_test_sourcelist(test_src ${test_prog}.cxx ${example_src})
  add_executable(${test_prog} ${test_src})
  target_include_directories(${test_prog} PRIVATE ${VTKm_INCLUDE_DIRS})
  target_link_libraries(${test_prog} ${VTKm_LIBRARIES})
  target_compile_options(${test_prog} PRIVATE ${VTKm_COMPILE_OPTIONS})

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
