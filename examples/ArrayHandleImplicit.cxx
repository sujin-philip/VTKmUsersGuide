////
//// BEGIN-EXAMPLE ImplicitArrayPortal.cxx
////
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/StorageImplicit.h>
#include <vtkm/cont/internal/IteratorFromArrayPortal.h>

class ArrayPortalEvenNumbers
{
public:
  typedef vtkm::Id ValueType;

  VTKM_EXEC_CONT_EXPORT
  ArrayPortalEvenNumbers() : NumberOfValues(0) {  }

  VTKM_EXEC_CONT_EXPORT
  ArrayPortalEvenNumbers(vtkm::Id numValues) : NumberOfValues(numValues) {  }

  VTKM_EXEC_CONT_EXPORT
  vtkm::Id GetNumberOfValues() const { return this->NumberOfValues; }

  VTKM_EXEC_CONT_EXPORT
  ValueType Get(vtkm::Id index) const { return 2*index; }

  typedef vtkm::cont::internal::IteratorFromArrayPortal<ArrayPortalEvenNumbers>
      IteratorType;

  VTKM_CONT_EXPORT
  IteratorType GetIteratorBegin() const
  {
    return IteratorType(*this);
  }

  VTKM_CONT_EXPORT
  IteratorType GetIteratorEnd() const
  {
    return IteratorType(*this, this->GetNumberOfValues());
  }

private:
  vtkm::Id NumberOfValues;
};
////
//// END-EXAMPLE ImplicitArrayPortal.cxx
////

////
//// BEGIN-EXAMPLE ImplicitArrayStorage.cxx
////
typedef vtkm::cont::StorageTagImplicit<ArrayPortalEvenNumbers>
    StorageTagEvenNumbers;
////
//// END-EXAMPLE ImplicitArrayStorage.cxx
////

////
//// BEGIN-EXAMPLE ImplicitArrayHandle.cxx
////
class ArrayHandleEvenNumbers
    : public vtkm::cont::ArrayHandle<
         vtkm::Id, StorageTagEvenNumbers>
{
  typedef vtkm::cont::ArrayHandle<vtkm::Id, StorageTagEvenNumbers>
      Superclass;

public:
  VTKM_CONT_EXPORT
  ArrayHandleEvenNumbers(vtkm::Id length)
    : Superclass(ArrayPortalEvenNumbers(length)) {  }
};
////
//// END-EXAMPLE ImplicitArrayHandle.cxx
////

#include <vtkm/cont/DeviceAdapter.h>

#include <vtkm/cont/testing/Testing.h>

namespace {

void Test()
{
  ArrayHandleEvenNumbers implicitArray(50);

  vtkm::cont::ArrayHandle<vtkm::Id> explicitArray;

  vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>::Copy(
        implicitArray, explicitArray);

  VTKM_TEST_ASSERT(explicitArray.GetNumberOfValues() == 50, "Wrong num vals.");
  for (vtkm::Id index = 0; index < explicitArray.GetNumberOfValues(); index++)
  {
    VTKM_TEST_ASSERT(explicitArray.GetPortalConstControl().Get(index) == 2*index,
                     "Bad array value.");
  }
}

} // anonymous namespace

int ArrayHandleImplicit(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
