#include <vtkm/Range.h>
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

void UsingRange()
{
  ////
  //// BEGIN-EXAMPLE UsingRange.cxx
  ////
  vtkm::Range range;                      // default constructor is empty range
  bool b1 = range.IsNonEmpty();           // b1 is false

  range.Include(0.5);                     // range now is [0.5 .. 0.5]
  bool b2 = range.IsNonEmpty();           // b2 is true
  bool b3 = range.Contains(0.5);          // b3 is true
  bool b4 = range.Contains(0.6);          // b4 is false

  range.Include(2.0);                     // range is now [0.5 .. 2]
  bool b5 = range.Contains(0.5);          // b3 is true
  bool b6 = range.Contains(0.6);          // b4 is true

  range.Include(vtkm::Range(-1, 1));      // range is now [-1 .. 2]
  //// PAUSE-EXAMPLE
  VTKM_TEST_ASSERT(test_equal(range, vtkm::Range(-1,2)), "Bad range");
  //// RESUME-EXAMPLE

  range.Include(vtkm::Range(3, 4));       // range is now [-1 .. 4]
  //// PAUSE-EXAMPLE
  VTKM_TEST_ASSERT(test_equal(range, vtkm::Range(-1,4)), "Bad range");
  //// RESUME-EXAMPLE

  vtkm::Float64 lower = range.Min;        // lower is -1
  vtkm::Float64 upper = range.Max;        // upper is 4
  vtkm::Float64 length = range.Length();  // length is 5
  vtkm::Float64 center = range.Center();  // center is 1.5
  ////
  //// END-EXAMPLE UsingRange.cxx
  ////

  VTKM_TEST_ASSERT(!b1, "Bad non empty.");
  VTKM_TEST_ASSERT(b2,  "Bad non empty.");
  VTKM_TEST_ASSERT(b3,  "Bad contains.");
  VTKM_TEST_ASSERT(!b4, "Bad contains.");
  VTKM_TEST_ASSERT(b5,  "Bad contains.");
  VTKM_TEST_ASSERT(b6,  "Bad contains.");

  VTKM_TEST_ASSERT(test_equal(lower, -1), "Bad lower");
  VTKM_TEST_ASSERT(test_equal(upper, 4), "Bad upper");
  VTKM_TEST_ASSERT(test_equal(length, 5), "Bad length");
  VTKM_TEST_ASSERT(test_equal(center, 1.5), "Bad center");
}

void UsingBounds()
{
  ////
  //// BEGIN-EXAMPLE UsingBounds.cxx
  ////
  vtkm::Bounds bounds;                      // default constructor makes empty
  bool b1 = bounds.IsNonEmpty();            // b1 is false

  bounds.Include(vtkm::make_Vec(0.5, 2.0, 0.0));    // bounds contains only
                                                    // the point [0.5, 2, 0]
  bool b2 = bounds.IsNonEmpty();                            // b2 is true
  bool b3 = bounds.Contains(vtkm::make_Vec(0.5, 2.0, 0.0)); // b3 is true
  bool b4 = bounds.Contains(vtkm::make_Vec(1, 1, 1));       // b4 is false
  bool b5 = bounds.Contains(vtkm::make_Vec(0, 0, 0));       // b5 is false

  bounds.Include(vtkm::make_Vec(4, -1, 2)); // bounds is region [0.5 .. 4] in X,
                                            //                  [-1 .. 2] in Y,
                                            //              and [0 .. 2] in Z
  //// PAUSE-EXAMPLE
  VTKM_TEST_ASSERT(test_equal(bounds, vtkm::Bounds(0.5, 4, -1, 2, 0, 2)), "");
  //// RESUME-EXAMPLE
  bool b6 = bounds.Contains(vtkm::make_Vec(0.5, 2.0, 0.0)); // b6 is true
  bool b7 = bounds.Contains(vtkm::make_Vec(1, 1, 1));       // b7 is true
  bool b8 = bounds.Contains(vtkm::make_Vec(0, 0, 0));       // b8 is false

  vtkm::Bounds otherBounds(vtkm::make_Vec(0, 0, 0), vtkm::make_Vec(3, 3, 3));
                                // otherBounds is region [0 .. 3] in X, Y, and Z
  bounds.Include(otherBounds);  // bounds is now region [0 .. 4] in X,
                                //                      [-1 .. 3] in Y,
                                //                  and [0 .. 3] in Z
  //// PAUSE-EXAMPLE
  VTKM_TEST_ASSERT(test_equal(bounds, vtkm::Bounds(0, 4, -1, 3, 0, 3)), "");
  //// RESUME-EXAMPLE

  vtkm::Vec<vtkm::Float64,3> lower(bounds.X.Min, bounds.Y.Min, bounds.Z.Min);
                                                        // lower is [0, -1, 0]
  vtkm::Vec<vtkm::Float64,3> upper(bounds.X.Max, bounds.Y.Max, bounds.Z.Max);
                                                        // upper is [4, 3, 3]

  vtkm::Vec<vtkm::Float64,3> center = bounds.Center();  // center is [2, 1, 1.5]
  ////
  //// END-EXAMPLE UsingBounds.cxx
  ////

  VTKM_TEST_ASSERT(!b1, "Bad non empty.");
  VTKM_TEST_ASSERT(b2,  "Bad non empty.");
  VTKM_TEST_ASSERT(b3,  "Bad contains.");
  VTKM_TEST_ASSERT(!b4, "Bad contains.");
  VTKM_TEST_ASSERT(!b5,  "Bad contains.");
  VTKM_TEST_ASSERT(b6,  "Bad contains.");
  VTKM_TEST_ASSERT(b7,  "Bad contains.");
  VTKM_TEST_ASSERT(!b8,  "Bad contains.");
  VTKM_TEST_ASSERT(test_equal(lower, vtkm::make_Vec(0, -1, 0)), "");
  VTKM_TEST_ASSERT(test_equal(upper, vtkm::make_Vec(4, 3, 3)), "");
  VTKM_TEST_ASSERT(test_equal(center, vtkm::make_Vec(2.0, 1.0, 1.5)), "");
}


void Test()
{
  CreatingVectorTypes();
  VectorOperations();
  LongerVector();
  EquilateralTriangle();
  UsingRange();
  UsingBounds();
}

} // anonymous namespace

int CoreDataTypes(int, char *[])
{
  return vtkm::testing::Testing::Run(Test);
}
