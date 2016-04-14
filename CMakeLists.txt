cmake_minimum_required(VERSION 2.8)

project(VTKmUsersGuide CXX C)

include(UseLATEX.cmake)

set(images
  images/TitleImage.png
  images/SandiaLogo.pdf
  images/DOELogo.pdf
  images/KitwareLogo.png
  images/ArrayHandleStorage.pdf
  images/CellConnectionsHexahedron.pdf
  images/CellConnectionsLine.pdf
  images/CellConnectionsPolygon.pdf
  images/CellConnectionsPyramid.pdf
  images/CellConnectionsQuadrilateral.pdf
  images/CellConnectionsTetrahedron.pdf
  images/CellConnectionsTriangle.pdf
  images/CellConnectionsVertex.pdf
  images/CellConnectionsWedge.pdf
  images/ExplicitCellConnections.pdf
  images/MCCompareCuda.png
  images/MCComparePiston.png
  images/MCCompareVTKm.png
  images/PackageHierarchy.pdf
  images/StructuredCellSet.pdf
  images/VTKmEnvironments.pdf
  images/WorkletExampleAnnotated.pdf
  )

set(input_docs
  Body.tex
  Macros.tex
  LanguageDefinitions.tex
  TitlePage.tex
  Contributors.tex
  Introduction.tex
  IO.tex
  BasicProvisions.tex
  ArrayHandle.tex
  DeviceAdapter.tex
  Timer.tex
  FancyArrayStorage.tex
  DynamicArrayHandle.tex
  DataSet.tex
  ExecutionEnvironment.tex
  Worklets.tex
  Math.tex
  AdvancedWorklets.tex
  CodingConventions.tex
  )

option(BUILD_EXAMPLES
  "When on, the examples used in the document are compiled. The examples can be run with ctest, which will report an error if they do not run correctly."
  OFF
  )

if (BUILD_EXAMPLES)
  enable_testing()
  include(CTest)
endif ()

add_subdirectory(examples)

set(add_latex_document_args
  INPUTS ${input_docs}
  IMAGES ${images}
  DEPENDS example-listings
  USE_INDEX
  )

add_latex_document(VTKmUsersGuide.tex
  ${add_latex_document_args}
  )

add_latex_document(GettingStarted.tex
  ${add_latex_document_args}
  EXCLUDE_FROM_ALL
  )

add_latex_document(Using.tex
  ${add_latex_document_args}
  EXCLUDE_FROM_ALL
  )

add_latex_document(Developing.tex
  ${add_latex_document_args}
  EXCLUDE_FROM_ALL
  )

add_latex_document(Advanced.tex
  ${add_latex_document_args}
  EXCLUDE_FROM_ALL
  )
