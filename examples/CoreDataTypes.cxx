#include <vtkm/Types.h>

#include <vtkm/testing/Testing.h>

namespace {

void CreatingVectorTypes()
{
  ////
  //// BEGIN-EXAMPLE CreatingVectorTypes.cxx
  ////
  vtkm::Vector3 A(1);                            // A is (1, 1, 1)
  A[1] = 2;                                      // A is now (1, 2, 1)
  vtkm::Vector3 B(1, 2, 3);                      // B is (1, 2, 3)
  vtkm::Vector3 C = vtkm::make_Vector3(3, 4, 5); // C is (3, 4, 5)
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
  vtkm::Vector3 A(1, 2, 3);
  vtkm::Vector3 B(4, 5, 6.5);
  vtkm::Vector3 C = A + B;                      // C is (5, 7, 9.5)
  vtkm::Vector3 D = 2 * C;                      // D is (10, 14, 19)
  vtkm::Scalar s = vtkm::dot(A, B);             // s is 33.5
  bool b1 = (A == B);                           // b1 is false
  bool b2 = (A == vtkm::make_Vector3(1, 2, 3)); // b2 is true
  ////
  //// END-EXAMPLE VectorOperations.cxx
  ////

  VTKM_TEST_ASSERT(test_equal(C, vtkm::Vector3(5, 7, 9.5)), "C is wrong");
  VTKM_TEST_ASSERT(test_equal(D, vtkm::Vector3(10, 14, 19)), "D is wrong");
  VTKM_TEST_ASSERT(test_equal(s, vtkm::Scalar(33.5)), "s is wrong");
  VTKM_TEST_ASSERT(!b1, "b1 is wrong");
  VTKM_TEST_ASSERT(b2, "b2 is wrong");
}

void TupleClass()
{
  ////
  //// BEGIN-EXAMPLE TupleClass.cxx
  ////
  vtkm::Tuple<vtkm::Scalar, 5> A(2); // A is (2, 2, 2, 2, 2)
  for (int index = 1; index < A.NUM_COMPONENTS; index++)
  {
    A[index] = A[index-1] * 1.5;
  }
  // A is now (2, 3, 4.5, 6.75, 10.125)
  ////
  //// END-EXAMPLE TupleClass.cxx
  ////

  VTKM_TEST_ASSERT(
      (A[0] == 2)
      && (A[1] == 3)
      && (A[2] == 4.5)
      && (A[3] == 6.75)
      && (A[4] == 10.125),
      "A is wrong");
}

////
//// BEGIN-EXAMPLE TuplesAndVectors.cxx
////
template<typename T, int Size>
VTKM_EXEC_CONT_EXPORT
T SumComponents(const vtkm::Tuple<T,Size> &tuple)
{
  T result = tuple[0];
  for (int componentIndex = 1; componentIndex < Size; componentIndex++)
  {
    result += tuple[componentIndex];
  }
  return result;
}

void Foo()
{
  vtkm::Id a = SumComponents(vtkm::Id3(1, 2, 3));                    // a is 6
  vtkm::Scalar b = SumComponents(vtkm::Vector4(1.5, 2.5, 3.5, 4.5)); // b is 12
  //// PAUSE-EXAMPLE
  VTKM_TEST_ASSERT(a == 6, "A is wrong");
  VTKM_TEST_ASSERT(b == 12, "B is wrong");
  //// RESUME-EXAMPLE
}
////
//// END-EXAMPLE TuplesAndVectors.cxx
////

void EquilateralTriangle()
{
  ////
  //// BEGIN-EXAMPLE EquilateralTriangle.cxx
  ////
  vtkm::Tuple<vtkm::Vector2,3> equilateralTriangle(vtkm::Vector2(0.0, 0.0),
                                                   vtkm::Vector2(1.0, 0.0),
                                                   vtkm::Vector2(0.5, 0.866));
  ////
  //// END-EXAMPLE EquilateralTriangle.cxx
  ////

  vtkm::Scalar edgeLengthSqr = 1.0;
  vtkm::Tuple<vtkm::Vector2,3> edges(
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

} // anonymous namespace

////
//// BEGIN-EXAMPLE Extent3.cxx
////
#include <vtkm/Extent.h>
#include <vtkm/Types.h>
//// PAUSE-EXAMPLE
namespace {
//// RESUME-EXAMPLE

void ExtentExample()
{
  // Make an extent that defines a grid that has 5x5x3 points and "centered"
  // at index (0,0,0).
  vtkm::Extent3 extent(vtkm::Id3(-2,-2,-1), vtkm::Id3(2,2,1));

  vtkm::Id3 minIndices = extent.Min; // Is (-2,-2,-1)
  vtkm::Id3 maxIndices = extent.Max; // Is (2,2,1)
  ////
  //// PAUSE-EXAMPLE
  VTKM_TEST_ASSERT(
      (minIndices[0] == -2) && (minIndices[1] == -2) && (minIndices[2] == -1),
      "Bad min index.");
  VTKM_TEST_ASSERT(
      (maxIndices[0] == 2) && (maxIndices[1] == 2) && (maxIndices[2] == 1),
      "Bad min index.");
  //// RESUME-EXAMPLE

  vtkm::Id3 pointDims = vtkm::ExtentPointDimensions(extent); // Returns (5,5,3)
  vtkm::Id3 cellDims = vtkm::ExtentCellDimensions(extent);   // Returns (4,4,2)
  ////
  //// PAUSE-EXAMPLE
  VTKM_TEST_ASSERT(pointDims == vtkm::Id3(5,5,3), "Wrong point dimensions");
  VTKM_TEST_ASSERT(cellDims == vtkm::Id3(4,4,2), "Wrong cell dimensions");
  //// RESUME-EXAMPLE

  vtkm::Id numPoints = vtkm::ExtentNumberOfPoints(extent); // Returns 75
  vtkm::Id numCells = vtkm::ExtentNumberOfCells(extent);   // Returns 32
  ////
  //// PAUSE-EXAMPLE
  VTKM_TEST_ASSERT(numPoints == 75, "Wrong number of points");
  VTKM_TEST_ASSERT(numCells == 32, "Wrong number of cells");
  //// RESUME-EXAMPLE

  // Returns (-1,-1,0)
  vtkm::Id3 pointIndexA = vtkm::ExtentPointFlatIndexToTopologyIndex(31, extent);

  // Returns (1,1,0)
  vtkm::Id3 cellIndexA = vtkm::ExtentCellFlatIndexToTopologyIndex(31, extent);
  ////
  //// PAUSE-EXAMPLE
  VTKM_TEST_ASSERT(pointIndexA == vtkm::Id3(-1,-1,0), "Bad conversion.");
  VTKM_TEST_ASSERT(cellIndexA == vtkm::Id3(1,1,0), "Bad conversion.");
  //// RESUME-EXAMPLE

  // Returns 33
  vtkm::Id pointIndexB =
      vtkm::ExtentPointTopologyIndexToFlatIndex(vtkm::Id3(1,-1,0), extent);

  // Returns 23
  vtkm::Id cellIndexB =
      vtkm::ExtentCellTopologyIndexToFlatIndex(vtkm::Id3(1,-1,0), extent);
  ////
  //// PAUSE-EXAMPLE
  VTKM_TEST_ASSERT(pointIndexB == 33, "Bad conversion.");
  VTKM_TEST_ASSERT(cellIndexB == 23, "Bad conversion.");
  //// RESUME-EXAMPLE

  // Returns 33
  vtkm::Id firstPoint = vtkm::ExtentFirstPointOnCell(23, extent);
  ////
  //// PAUSE-EXAMPLE
  VTKM_TEST_ASSERT(firstPoint == 33, "Got wrong point index.");
  //// RESUME-EXAMPLE
}
////
//// END-EXAMPLE Extent3.cxx
////

void Test()
{
  CreatingVectorTypes();
  VectorOperations();
  TupleClass();
  Foo();
  EquilateralTriangle();
  ExtentExample();
}

} // anonymous namespace

int CoreDataTypes(int, char *[])
{
  return vtkm::testing::Testing::Run(Test);
}
