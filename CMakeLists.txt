cmake_minimum_required(VERSION 2.8)

project(VTKmUsersGuide CXX C)

include(UseLATEX.cmake)

set(images
  images/TitleImage.png
  images/SandiaLogo.pdf
  images/DOELogo.pdf
  images/KitwareLogo.png
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
  images/StructuredCellSet.pdf
  images/WorkletExampleAnnotated.pdf
  )

set(input_docs
  Macros.tex
  LanguageDefinitions.tex
  TitlePage.tex
  Contributors.tex
  Introduction.tex
  BasicProvisions.tex
  ControlEnvironment.tex
  ExecutionEnvironment.tex
  Worklets.tex
  AdvancedWorklets.tex
  CodingConventions.tex
  Distribution.tex
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

add_latex_document(VTKmUsersGuide.tex
  INPUTS ${input_docs}
  IMAGES ${images}
  DEPENDS example-listings
  USE_INDEX
  )
