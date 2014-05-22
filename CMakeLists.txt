cmake_minimum_required(VERSION 2.8)

project(VTKmUsersGuide NONE)

include(UseLATEX.cmake)

set(images
  SANDimages/DOEbwlogo.pdf
  SANDimages/snllineblk.pdf
  SANDimages/SANDbackground.png
  )

set(input_docs
  Distribution.tex
  )

add_latex_document(VTKmUsersGuide.tex
  INPUTS ${input_docs}
  IMAGES ${images}
  USE_INDEX
  )