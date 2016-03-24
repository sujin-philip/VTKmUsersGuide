#include <vtkm/Types.h>

#include <vtkm/testing/Testing.h>

namespace {

void CreatingVectorTypes()
{
  ////
  //// BEGIN-EXAMPLE CreatingVectorTypes.cxx
  ////
  vtkm::Vec<vtkm::Float32,3> A(1);                        // A is (1, 1, 1)
  A[1] = 2;                                               // A is now (1, 2, 1)
  vtkm::Vec<vtkm::Float32,3> B(1, 2, 3);                  // B is (1, 2, 3)
  vtkm::Vec<vtkm::Float32,3> C = vtkm::make_Vec(3, 4, 5); // C is (3, 4, 5)
  ////
  //// END-EXAMPLE CreatingVectorTypes.cxx
  ////

  VTKM_TEST_ASSERT((A[0] == 1) && (A[1] == 2) && (A[2] == 1),
                   "A is different than expected.");
  VTKM_TEST_ASSERT((B[0] == 1) && (B[1] == 2) && (B[2] == 3),
                   "B is different than expected.");
  VTKM_TEST_ASSERT((C[0] == 3) && (C[1] == 4) && (C[2] == 5),
                   "C is different than expected.");
}

void VectorOperations()
{
  ////
  //// BEGIN-EXAMPLE VectorOperations.cxx
  ////
  vtkm::Vec<vtkm::Float32,3> A(1, 2, 3);
  vtkm::Vec<vtkm::Float32,3> B(4, 5, 6.5);
  vtkm::Vec<vtkm::Float32,3> C = A + B;         // C is (5, 7, 9.5)
  vtkm::Vec<vtkm::Float32,3> D = 2.0f * C;      // D is (10, 14, 19)
  vtkm::Float32 s = vtkm::dot(A, B);            // s is 33.5
  bool b1 = (A == B);                           // b1 is false
  bool b2 = (A == vtkm::make_Vec(1, 2, 3));     // b2 is true
  ////
  //// END-EXAMPLE VectorOperations.cxx
  ////

  VTKM_TEST_ASSERT(test_equal(C, vtkm::Vec<vtkm::Float32,3>(5, 7, 9.5)), "C is wrong");
  VTKM_TEST_ASSERT(test_equal(D, vtkm::Vec<vtkm::Float32,3>(10, 14, 19)), "D is wrong");
  VTKM_TEST_ASSERT(test_equal(s, 33.5), "s is wrong");
  VTKM_TEST_ASSERT(!b1, "b1 is wrong");
  VTKM_TEST_ASSERT(b2, "b2 is wrong");
}

void LongerVector()
{
  ////
  //// BEGIN-EXAMPLE LongerVector.cxx
  ////
  vtkm::Vec<vtkm::Float32, 5> A(2); // A is (2, 2, 2, 2, 2)
  for (vtkm::IdComponent index = 1; index < A.NUM_COMPONENTS; index++)
  {
    A[index] = A[index-1] * 1.5;
  }
  // A is now (2, 3, 4.5, 6.75, 10.125)
  ////
  //// END-EXAMPLE LongerVector.cxx
  ////

  VTKM_TEST_ASSERT(
      (A[0] == 2)
      && (A[1] == 3)
      && (A[2] == 4.5)
      && (A[3] == 6.75)
      && (A[4] == 10.125),
      "A is wrong");
}

void EquilateralTriangle()
{
  ////
  //// BEGIN-EXAMPLE EquilateralTriangle.cxx
  ////
  vtkm::Vec<vtkm::Vec<vtkm::Float32,2>, 3> equilateralTriangle(
                                                   vtkm::make_Vec(0.0, 0.0),
                                                   vtkm::make_Vec(1.0, 0.0),
                                                   vtkm::make_Vec(0.5, 0.866));
  ////
  //// END-EXAMPLE EquilateralTriangle.cxx
  ////

  vtkm::Float32 edgeLengthSqr = 1.0;
  vtkm::Vec<vtkm::Vec<vtkm::Float32,2>,3> edges(
      equilateralTriangle[1] - equilateralTriangle[0],
      equilateralTriangle[2] - equilateralTriangle[0],
      equilateralTriangle[2] - equilateralTriangle[1]);
  VTKM_TEST_ASSERT(test_equal(vtkm::dot(edges[0],edges[0]), edgeLengthSqr),
                   "Bad edge length.");
  VTKM_TEST_ASSERT(test_equal(vtkm::dot(edges[1],edges[1]), edgeLengthSqr),
                   "Bad edge length.");
  VTKM_TEST_ASSERT(test_equal(vtkm::dot(edges[2],edges[2]), edgeLengthSqr),
                   "Bad edge length.");
}


void Test()
{
  CreatingVectorTypes();
  VectorOperations();
  LongerVector();
  EquilateralTriangle();
}

} // anonymous namespace

int CoreDataTypes(int, char *[])
{
  return vtkm::testing::Testing::Run(Test);
}
