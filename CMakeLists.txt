cmake_minimum_required(VERSION 2.8)

project(VTKmUsersGuide CXX C)

include(UseLATEX.cmake)

set(images
  images/TitleImage.png
  images/SandiaLogo.pdf
  images/DOELogo.pdf
  images/KitwareLogo.png
  images/CMakeGUIBlank.png
  images/CMakeGUI.png
  images/ArrayHandleStorage.pdf
  images/BasicGlut.png
  images/CameraViewRange2D.pdf
  images/CameraPositionOrientation.pdf
  images/CameraMovement.pdf
  images/CellConnectionsHexahedron.pdf
  images/CellConnectionsLine.pdf
  images/CellConnectionsPolygon.pdf
  images/CellConnectionsPyramid.pdf
  images/CellConnectionsQuadrilateral.pdf
  images/CellConnectionsTetrahedron.pdf
  images/CellConnectionsTriangle.pdf
  images/CellConnectionsVertex.pdf
  images/CellConnectionsWedge.pdf
  images/CellConstituents.pdf
  images/Dragon01.pdf
  images/Dragon02.pdf
  images/Dragon03.pdf
  images/Dragon04.pdf
  images/Dragon12.pdf
  images/ExplicitCellConnections.pdf
  images/Hilbert01.pdf
  images/Hilbert02.pdf
  images/Hilbert03.pdf
  images/Hilbert06.pdf
  images/Koch1.pdf
  images/Koch2.pdf
  images/Koch5.pdf
  images/KochApply.pdf
  images/KochParametric.pdf
  images/MCCompareCuda.png
  images/MCComparePiston.png
  images/MCCompareVTKm.png
  images/PackageHierarchy.pdf
  images/PointIncidentAngles.pdf
  images/QuadraticType2_1.pdf
  images/QuadraticType2_2.pdf
  images/QuadraticType2_4.pdf
  images/StructuredCellSet.pdf
  images/Tree01.pdf
  images/Tree02.pdf
  images/Tree08.pdf
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
  BuildAndInstall.tex
  IO.tex
  ProvidedFilters.tex
  Rendering.tex
  BasicProvisions.tex
  ArrayHandle.tex
  DeviceAdapter.tex
  Timer.tex
  FancyArrayStorage.tex
  DynamicArrayHandle.tex
  DataSet.tex
  Policies.tex
  Worklets.tex
  CreatingFilters.tex
  Math.tex
  WorkingWithCells.tex
  VirtualObjects.tex
  ImplementingDeviceAdapters.tex
  OpenGLInteroperability.tex
  FunctionInterface.tex
  WorkletArguments.tex
  NewWorkletTypes.tex
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

set(VTKm_GUIDE_VERSION 1.1)

add_subdirectory(examples)

set(add_latex_document_args
  INPUTS ${input_docs}
  CONFIGURE TitlePage.tex
  IMAGES ${images}
  IMAGE_DIRS images/ColorTables
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
