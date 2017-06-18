// The following method for declaring an implicit array still works, but
// it is much easier to use the ArrayHandleImplict class.
////
//// BEGIN-EXAMPLE ImplicitArrayPortal.cxx
////
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/StorageImplicit.h>

class ArrayPortalEvenNumbers
{
public:
  typedef vtkm::Id ValueType;

  VTKM_EXEC_CONT
  ArrayPortalEvenNumbers() : NumberOfValues(0) {  }

  VTKM_EXEC_CONT
  ArrayPortalEvenNumbers(vtkm::Id numValues) : NumberOfValues(numValues) {  }

  VTKM_EXEC_CONT
  vtkm::Id GetNumberOfValues() const { return this->NumberOfValues; }

  VTKM_EXEC_CONT
  ValueType Get(vtkm::Id index) const { return 2*index; }

private:
  vtkm::Id NumberOfValues;
};
////
//// END-EXAMPLE ImplicitArrayPortal.cxx
////

// More of the example no longer being used.
////
//// BEGIN-EXAMPLE ImplicitArrayStorage.cxx
////
typedef vtkm::cont::StorageTagImplicit<ArrayPortalEvenNumbers>
    StorageTagEvenNumbers;
////
//// END-EXAMPLE ImplicitArrayStorage.cxx
////

// More of the example no longer being used.
////
//// BEGIN-EXAMPLE ImplicitArrayHandle.cxx
////
class ArrayHandleEvenNumbers
    : public vtkm::cont::ArrayHandle<vtkm::Id, StorageTagEvenNumbers>
{
public:
  VTKM_ARRAY_HANDLE_SUBCLASS_NT(
      ArrayHandleEvenNumbers,
      (vtkm::cont::ArrayHandle<vtkm::Id,StorageTagEvenNumbers>));

  VTKM_CONT
  ArrayHandleEvenNumbers(vtkm::Id length)
    : Superclass(ArrayPortalEvenNumbers(length)) {  }
};
////
//// END-EXAMPLE ImplicitArrayHandle.cxx
////

////
//// BEGIN-EXAMPLE ImplicitArrayFunctor.cxx
////
struct DoubleIndexFunctor
{
  VTKM_EXEC_CONT
  vtkm::Id operator()(vtkm::Id index) const
  {
    return 2*index;
  }
};
////
//// END-EXAMPLE ImplicitArrayFunctor.cxx
////

////
//// BEGIN-EXAMPLE ImplicitArrayHandle2.cxx
////
#include <vtkm/cont/ArrayHandleImplicit.h>

class ArrayHandleDoubleIndex
    : public vtkm::cont::ArrayHandleImplicit<DoubleIndexFunctor>
{
public:
  VTKM_ARRAY_HANDLE_SUBCLASS_NT(
      ArrayHandleDoubleIndex,
      (vtkm::cont::ArrayHandleImplicit<DoubleIndexFunctor>));

  VTKM_CONT
  ArrayHandleDoubleIndex(vtkm::Id numberOfValues)
    : Superclass(DoubleIndexFunctor(), numberOfValues) {  }
};
////
//// END-EXAMPLE ImplicitArrayHandle2.cxx
////

#include <vtkm/cont/DeviceAdapter.h>

#include <vtkm/cont/testing/Testing.h>

namespace {

void Test()
{
  ////
  //// BEGIN-EXAMPLE DeclareImplicitArray.cxx
  ////
  vtkm::cont::ArrayHandleImplicit<DoubleIndexFunctor>
      implicitArray(DoubleIndexFunctor(), 50);
  ////
  //// END-EXAMPLE DeclareImplicitArray.cxx
  ////

  ArrayHandleDoubleIndex implicitArray2(50);
  ArrayHandleEvenNumbers implicitArray3(50);

  vtkm::cont::ArrayHandle<vtkm::Id> explicitArray;
  vtkm::cont::ArrayHandle<vtkm::Id> explicitArray2;
  vtkm::cont::ArrayHandle<vtkm::Id> explicitArray3;

  vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>::Copy(
        implicitArray, explicitArray);
  vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>::Copy(
        implicitArray2, explicitArray2);
  vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>::Copy(
        implicitArray3, explicitArray3);

  VTKM_TEST_ASSERT(explicitArray.GetNumberOfValues() == 50, "Wrong num vals.");
  VTKM_TEST_ASSERT(explicitArray2.GetNumberOfValues() == 50, "Wrong num vals.");
  VTKM_TEST_ASSERT(explicitArray3.GetNumberOfValues() == 50, "Wrong num vals.");
  for (vtkm::Id index = 0; index < explicitArray.GetNumberOfValues(); index++)
  {
    VTKM_TEST_ASSERT(explicitArray.GetPortalConstControl().Get(index) == 2*index,
                     "Bad array value.");
    VTKM_TEST_ASSERT(explicitArray2.GetPortalConstControl().Get(index) == 2*index,
                     "Bad array value.");
    VTKM_TEST_ASSERT(explicitArray3.GetPortalConstControl().Get(index) == 2*index,
                     "Bad array value.");
  }

  // Just an example of using make_ArrayHandleImplicit
  ////
  //// BEGIN-EXAMPLE MakeArrayHandleImplicit.cxx
  ////
  vtkm::cont::make_ArrayHandleImplicit(DoubleIndexFunctor(), 50);
  ////
  //// END-EXAMPLE MakeArrayHandleImplicit.cxx
  ////
}

} // anonymous namespace

int ArrayHandleImplicit(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
