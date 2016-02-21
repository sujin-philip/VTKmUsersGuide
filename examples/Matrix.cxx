#include <vtkm/Matrix.h>

#include <vtkm/testing/Testing.h>

namespace {

VTKM_CONT_EXPORT
void BuildMatrix()
{
  std::cout << "Building matrix containing " << std::endl
            << "|  0  1  2 |" << std::endl
            << "| 10 11 12 |" << std::endl;

  ////
  //// BEGIN-EXAMPLE BuildMatrix.cxx
  ////
  vtkm::Matrix<vtkm::Float32, 2, 3> matrix;

  // Using parenthesis notation.
  matrix(0,0) = 0.0f;
  matrix(0,1) = 1.0f;
  matrix(0,2) = 2.0f;

  // Using bracket notation.
  matrix[1][0] = 10.0f;
  matrix[1][1] = 11.0f;
  matrix[1][2] = 12.0f;
  ////
  //// END-EXAMPLE BuildMatrix.cxx
  ////

  vtkm::Vec<vtkm::Float32,2> termVec(1.0f, 0.1f);
  vtkm::Vec<vtkm::Float32,3> multVec = vtkm::MatrixMultiply(termVec, matrix);
//  std::cout << multVec << std::endl;
  VTKM_TEST_ASSERT(test_equal(multVec, vtkm::make_Vec(1.0, 2.1, 3.2)),
                   "Unexpected product.");
}

void Run()
{
  BuildMatrix();
}

} // anonymous namespace

int Matrix(int, char*[])
{
  return vtkm::testing::Testing::Run(Run);
}
