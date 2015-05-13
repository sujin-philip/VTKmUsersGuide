#include <vtkm/cont/ArrayHandlePermutation.h>

#include <vtkm/cont/testing/Testing.h>

namespace {

template<typename ArrayHandleType>
void CheckArray1(const ArrayHandleType array)
{
  VTKM_TEST_ASSERT(array.GetNumberOfValues() == 3,
                   "Permuted array has wrong size.");

  typename ArrayHandleType::PortalConstControl portal =
      array.GetPortalConstControl();
  VTKM_TEST_ASSERT(portal.GetNumberOfValues() == 3,
                   "Permuted portal has wrong size.");

  VTKM_TEST_ASSERT(test_equal(portal.Get(0), 0.3),
                   "Permuted array has wrong value.");
  VTKM_TEST_ASSERT(test_equal(portal.Get(1), 0.0),
                   "Permuted array has wrong value.");
  VTKM_TEST_ASSERT(test_equal(portal.Get(2), 0.1),
                   "Permuted array has wrong value.");
}

template<typename ArrayHandleType>
void CheckArray2(const ArrayHandleType array)
{
  VTKM_TEST_ASSERT(array.GetNumberOfValues() == 5,
                   "Permuted array has wrong size.");

  typename ArrayHandleType::PortalConstControl portal =
      array.GetPortalConstControl();
  VTKM_TEST_ASSERT(portal.GetNumberOfValues() == 5,
                   "Permuted portal has wrong size.");

  VTKM_TEST_ASSERT(test_equal(portal.Get(0), 0.1),
                   "Permuted array has wrong value.");
  VTKM_TEST_ASSERT(test_equal(portal.Get(1), 0.2),
                   "Permuted array has wrong value.");
  VTKM_TEST_ASSERT(test_equal(portal.Get(2), 0.2),
                   "Permuted array has wrong value.");
  VTKM_TEST_ASSERT(test_equal(portal.Get(3), 0.3),
                   "Permuted array has wrong value.");
  VTKM_TEST_ASSERT(test_equal(portal.Get(4), 0.0),
                   "Permuted array has wrong value.");
}

void Test()
{
  ////
  //// BEGIN-EXAMPLE ArrayHandlePermutation.cxx
  ////
  typedef vtkm::cont::ArrayHandle<vtkm::Id> IdArrayType;
  typedef IdArrayType::PortalControl IdPortalType;

  typedef vtkm::cont::ArrayHandle<vtkm::Float32> ValueArrayType;
  typedef ValueArrayType::PortalControl ValuePortalType;

  // Create array with values [0.0, 0.1, 0.2, 0.3]
  ValueArrayType valueArray;
  valueArray.Allocate(4);
  ValuePortalType valuePortal = valueArray.GetPortalControl();
  valuePortal.Set(0, 0.0);
  valuePortal.Set(1, 0.1);
  valuePortal.Set(2, 0.2);
  valuePortal.Set(3, 0.3);

  // Use ArrayHandlePermutation to make an array = [0.3, 0.0, 0.1].
  IdArrayType idArray1;
  idArray1.Allocate(3);
  IdPortalType idPortal1 = idArray1.GetPortalControl();
  idPortal1.Set(0, 3);
  idPortal1.Set(1, 0);
  idPortal1.Set(2, 1);
  vtkm::cont::ArrayHandlePermutation<IdArrayType,ValueArrayType>
      permutedArray1(idArray1, valueArray);
  //// PAUSE-EXAMPLE
  CheckArray1(permutedArray1);
  //// RESUME-EXAMPLE

  // Use ArrayHandlePermutation to make an array = [0.1, 0.2, 0.2, 0.3, 0.0]
  IdArrayType idArray2;
  idArray2.Allocate(5);
  IdPortalType idPortal2 = idArray2.GetPortalControl();
  idPortal2.Set(0, 1);
  idPortal2.Set(1, 2);
  idPortal2.Set(2, 2);
  idPortal2.Set(3, 3);
  idPortal2.Set(4, 0);
  vtkm::cont::ArrayHandlePermutation<IdArrayType,ValueArrayType>
      permutedArray2(idArray2, valueArray);
  //// PAUSE-EXAMPLE
  CheckArray2(permutedArray2);
  //// RESUME-EXAMPLE
  ////
  //// END-EXAMPLE ArrayHandlePermutation.cxx
  ////

  IdArrayType idArray = idArray2;
  CheckArray2(
  ////
  //// BEGIN-EXAMPLE MakeArrayHandlePermutation.cxx
  ////
  vtkm::cont::make_ArrayHandlePermutation(idArray,valueArray)
  ////
  //// END-EXAMPLE MakeArrayHandlePermutation.cxx
  ////
  );
}

} // anonymous namespace

int ArrayHandlePermutation(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
