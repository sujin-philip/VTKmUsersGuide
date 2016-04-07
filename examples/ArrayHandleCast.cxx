#include <vtkm/cont/ArrayHandleCast.h>

#include <vector>

#include <vtkm/cont/testing/Testing.h>

namespace {

template<typename OriginalType, typename ArrayHandleType>
void CheckArray(const ArrayHandleType array)
{
  vtkm::Id length = array.GetNumberOfValues();

  typename ArrayHandleType::PortalConstControl portal =
      array.GetPortalConstControl();
  VTKM_TEST_ASSERT(portal.GetNumberOfValues() == length,
                   "Portal has wrong size.");

  for (vtkm::Id index = 0; index < length; index++)
  {
    VTKM_TEST_ASSERT(test_equal(portal.Get(index),
                                TestValue(index, OriginalType())),
                     "Array has wrong value.");
    VTKM_TEST_ASSERT(
          !test_equal(portal.Get(index),
                      TestValue(index, typename ArrayHandleType::ValueType())),
          "Array has wrong value.");
  }
}

////
//// BEGIN-EXAMPLE ArrayHandleCast.cxx
////
template<typename T>
VTKM_CONT_EXPORT
void Foo(const std::vector<T> &inputData)
{
  vtkm::cont::ArrayHandle<T> originalArray =
      vtkm::cont::make_ArrayHandle(inputData);

  vtkm::cont::ArrayHandleCast<vtkm::Float64, vtkm::cont::ArrayHandle<T> >
      castArray(originalArray);
  ////
  //// END-EXAMPLE ArrayHandleCast.cxx
  ////
  CheckArray<T>(castArray);

  CheckArray<T>(
  ////
  //// BEGIN-EXAMPLE MakeArrayHandleCast.cxx
  ////
  vtkm::cont::make_ArrayHandleCast<vtkm::Float64>(originalArray)
  ////
  //// END-EXAMPLE MakeArrayHandleCast.cxx
  ////
        );
}

void Test()
{
  const std::size_t ARRAY_SIZE = 50;
  std::vector<vtkm::Id> inputData(ARRAY_SIZE);
  for (std::size_t index = 0; index < ARRAY_SIZE; index++)
  {
    inputData[index] = TestValue(vtkm::Id(index), vtkm::Id());
  }

  Foo(inputData);
}

} // anonymous namespace

int ArrayHandleCast(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
