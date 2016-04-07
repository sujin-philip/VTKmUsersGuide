
set(example_src
  ArrayHandle.cxx
  ArrayHandleAdapt.cxx
  ArrayHandleCast.cxx
  ArrayHandleConstant.cxx
  ArrayHandleCounting.cxx
  ArrayHandleDerived.cxx
  ArrayHandleImplicit.cxx
  ArrayHandlePermutation.cxx
  ArrayHandleTransform.cxx
  ArrayHandleZip.cxx
  CellOperations.cxx
  CellShapes.cxx
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
  find_package(VTKm REQUIRED)

  include_directories(${VTKm_INCLUDE_DIRS})

  set(test_prog ExampleTests)
  create_test_sourcelist(test_src ${test_prog}.cxx ${example_src})
  add_executable(${test_prog} ${test_src})
  target_link_libraries(${test_prog} ${VTKm_LIBRARIES})

  foreach (test ${example_src})
    get_filename_component(tname ${test} NAME_WE)
    add_test(NAME ${tname}
      COMMAND ${test_prog} ${tname}
      )
  endforeach()
endif()

include(ExtractExample.cmake)

extract_examples(created_files ${example_src})
add_custom_target(example-listings DEPENDS ${created_files})
