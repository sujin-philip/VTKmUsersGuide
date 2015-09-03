
set(example_src
  ArrayHandle.cxx
  ArrayHandleAdapt.cxx
  ArrayHandleDerived.cxx
  ArrayHandleImplicit.cxx
  ArrayHandlePermutation.cxx
  ArrayHandleTransform.cxx
  ArrayHandleZip.cxx
  CoreDataTypes.cxx
  DeviceAdapterTag.cxx
  DeviceAdapterAlgorithms.cxx
  DynamicArrayHandle.cxx
  ErrorHandling.cxx
  ExportMacro.cxx
  FunctionInterface.cxx
  ListTags.cxx
  Timer.cxx
  Traits.cxx
  TransferringArguments.cxx
  UseWorkletMapField.cxx
  )

# Set up compiling and testing of examples.
if (BUILD_EXAMPLES)
  find_package(VTKm REQUIRED)

  # This should later be encapsulated in VTKmUseSerial
  find_package(BoostHeaders REQUIRED)
  include_directories(${Boost_INCLUDE_DIRS} ${VTKm_INCLUDE_DIRS})

  set(test_prog ExampleTests)
  create_test_sourcelist(test_src ${test_prog}.cxx ${example_src})
  add_executable(${test_prog} ${test_src})

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
