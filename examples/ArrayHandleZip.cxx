#include <vtkm/cont/ArrayHandleZip.h>

#include <vtkm/cont/testing/Testing.h>

namespace {

template<typename ArrayHandleType>
void CheckArray(const ArrayHandleType array)
{
  VTKM_TEST_ASSERT(array.GetNumberOfValues() == 3,
                   "Permuted array has wrong size.");

  typename ArrayHandleType::PortalConstControl portal =
      array.GetPortalConstControl();
  VTKM_TEST_ASSERT(portal.GetNumberOfValues() == 3,
                   "Permuted portal has wrong size.");

  typedef vtkm::Pair<vtkm::Id, vtkm::Float32> PairType;

  VTKM_TEST_ASSERT(test_equal(portal.Get(0), PairType(3, 0.0)),
                   "Zipped array has wrong value.");
  VTKM_TEST_ASSERT(test_equal(portal.Get(1), PairType(0, 0.1)),
                   "Zipped array has wrong value.");
  VTKM_TEST_ASSERT(test_equal(portal.Get(2), PairType(1, 0.2)),
                   "Zipped array has wrong value.");
}

void Test()
{
  ////
  //// BEGIN-EXAMPLE ArrayHandleZip.cxx
  ////
  typedef vtkm::cont::ArrayHandle<vtkm::Id> ArrayType1;
  typedef ArrayType1::PortalControl PortalType1;

  typedef vtkm::cont::ArrayHandle<vtkm::Float32> ArrayType2;
  typedef ArrayType2::PortalControl PortalType2;

  // Create an array of vtkm::Id with values [3, 0, 1]
  ArrayType1 array1;
  array1.Allocate(3);
  PortalType1 portal1 = array1.GetPortalControl();
  portal1.Set(0, 3);
  portal1.Set(1, 0);
  portal1.Set(2, 1);

  // Create a second array of vtkm::Float32 with values [0.0, 0.1, 0.2]
  ArrayType2 array2;
  array2.Allocate(3);
  PortalType2 portal2 = array2.GetPortalControl();
  portal2.Set(0, 0.0);
  portal2.Set(1, 0.1);
  portal2.Set(2, 0.2);

  // Zip the two arrays together to create an array of
  // vtkm::Pair<vtkm::Id, vtkm::Float32> with values [(3,0.0), (0,0.1), (1,0.2)]
  vtkm::cont::ArrayHandleZip<ArrayType1,ArrayType2> zipArray(array1, array2);
  ////
  //// END-EXAMPLE ArrayHandleZip.cxx
  ////

  CheckArray(zipArray);

  CheckArray(
  ////
  //// BEGIN-EXAMPLE MakeArrayHandleZip.cxx
  ////
  vtkm::cont::make_ArrayHandleZip(array1,array2)
  ////
  //// END-EXAMPLE MakeArrayHandleZip.cxx
  ////
  );
}

} // anonymous namespace

int ArrayHandleZip(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
