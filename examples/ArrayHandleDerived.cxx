////
//// BEGIN-EXAMPLE DerivedArrayPortal.cxx
////
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayPortal.h>

template<typename P1, typename P2>
class ArrayPortalConcatenate
{
public:
  typedef P1 PortalType1;
  typedef P2 PortalType2;
  typedef typename PortalType1::ValueType ValueType;

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC_CONT_EXPORT
  ArrayPortalConcatenate() : Portal1(), Portal2() {  }

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC_CONT_EXPORT
  ArrayPortalConcatenate(const PortalType1 &portal1, const PortalType2 portal2)
    : Portal1(portal1), Portal2(portal2) {  }

  /// Copy constructor for any other ArrayPortalConcatenate with a portal type
  /// that can be copied to this portal type. This allows us to do any type
  /// casting that the portals do (like the non-const to const cast).
  VTKM_SUPPRESS_EXEC_WARNINGS
  template<typename OtherP1, typename OtherP2>
  VTKM_EXEC_CONT_EXPORT
  ArrayPortalConcatenate(const ArrayPortalConcatenate<OtherP1,OtherP2> &src)
    : Portal1(src.GetPortal1()), Portal2(src.GetPortal2()) {  }

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC_CONT_EXPORT
  vtkm::Id GetNumberOfValues() const {
    return
        this->Portal1.GetNumberOfValues() + this->Portal2.GetNumberOfValues();
  }

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC_CONT_EXPORT
  ValueType Get(vtkm::Id index) const {
    if (index < this->Portal1.GetNumberOfValues())
    {
      return this->Portal1.Get(index);
    }
    else
    {
      return this->Portal2.Get(index - this->Portal1.GetNumberOfValues());
    }
  }

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC_CONT_EXPORT
  void Set(vtkm::Id index, const ValueType &value) const {
    if (index < this->Portal1.GetNumberOfValues())
    {
      this->Portal1.Set(index, value);
    }
    else
    {
      this->Portal2.Set(index - this->Portal1.GetNumberOfValues(), value);
    }
  }

  VTKM_EXEC_CONT_EXPORT
  const PortalType1 &GetPortal1() const { return this->Portal1; }
  VTKM_EXEC_CONT_EXPORT
  const PortalType2 &GetPortal2() const { return this->Portal2; }

private:
  PortalType1 Portal1;
  PortalType2 Portal2;
};
////
//// END-EXAMPLE DerivedArrayPortal.cxx
////

////
//// BEGIN-EXAMPLE DerivedArrayStorage.cxx
////
template<typename ArrayHandleType1, typename ArrayHandleType2>
struct StorageTagConcatenate {  };

namespace vtkm {
namespace cont {
namespace internal {

template<typename ArrayHandleType1, typename ArrayHandleType2>
class Storage<
    typename ArrayHandleType1::ValueType,
    StorageTagConcatenate<ArrayHandleType1, ArrayHandleType2> >
{
public:
  typedef typename ArrayHandleType1::ValueType ValueType;

  typedef ArrayPortalConcatenate<
      typename ArrayHandleType1::PortalControl,
      typename ArrayHandleType2::PortalControl> PortalType;
  typedef ArrayPortalConcatenate<
      typename ArrayHandleType1::PortalConstControl,
      typename ArrayHandleType2::PortalConstControl> PortalConstType;

  VTKM_CONT_EXPORT
  Storage() : Valid(false) {  }

  VTKM_CONT_EXPORT
  Storage(const ArrayHandleType1 array1, const ArrayHandleType2 array2)
    : Array1(array1), Array2(array2), Valid(true) {  }

  VTKM_CONT_EXPORT
  PortalType GetPortal() {
    VTKM_ASSERT(this->Valid);
    return PortalType(this->Array1.GetPortalControl(),
                      this->Array2.GetPortalControl());
  }

  VTKM_CONT_EXPORT
  PortalConstType GetPortalConst() const {
    VTKM_ASSERT(this->Valid);
    return PortalConstType(this->Array1.GetPortalConstControl(),
                           this->Array2.GetPortalConstControl());
  }

  VTKM_CONT_EXPORT
  vtkm::Id GetNumberOfValues() const {
    VTKM_ASSERT(this->Valid);
    return this->Array1.GetNumberOfValues() + this->Array2.GetNumberOfValues();
  }

  VTKM_CONT_EXPORT
  void Allocate(vtkm::Id numberOfValues) {
    VTKM_ASSERT(this->Valid);
    // This implementation of allocate, which allocates the same amount in both
    // arrays, is arbitrary. It could, for example, leave the size of Array1
    // alone and change the size of Array2. Or, probably most likely, it could
    // simply throw an error and state that this operation is invalid.
    vtkm::Id half = numberOfValues/2;
    this->Array1.Allocate(numberOfValues-half);
    this->Array2.Allocate(half);
  }

  VTKM_CONT_EXPORT
  void Shrink(vtkm::Id numberOfValues) {
    VTKM_ASSERT(this->Valid);
    if (numberOfValues < this->Array1.GetNumberOfValues())
    {
      this->Array1.Shrink(numberOfValues);
      this->Array2.Shrink(0);
    }
    else
    {
      this->Array2.Shrink(numberOfValues - this->Array1.GetNumberOfValues());
    }
  }

  VTKM_CONT_EXPORT
  void ReleaseResources() {
    VTKM_ASSERT(this->Valid);
    this->Array1.ReleaseResources();
    this->Array2.ReleaseResources();
  }

  // Requried for later use in ArrayTransfer class.
  VTKM_CONT_EXPORT
  const ArrayHandleType1 &GetArray1() const {
    VTKM_ASSERT(this->Valid);
    return this->Array1;
  }
  VTKM_CONT_EXPORT
  const ArrayHandleType2 &GetArray2() const {
    VTKM_ASSERT(this->Valid);
    return this->Array2;
  }

private:
  ArrayHandleType1 Array1;
  ArrayHandleType2 Array2;
  bool Valid;
};

}
}
} // namespace vtkm::cont::internal
////
//// END-EXAMPLE DerivedArrayStorage.cxx
////

////
//// BEGIN-EXAMPLE ArrayTransferPrototype.cxx
////
namespace vtkm {
namespace cont {
namespace internal {

template<typename T,typename StorageTag,typename DeviceAdapterTag>
class ArrayTransfer;

}
}
} //namespace vtkm::cont::internal
////
//// END-EXAMPLE ArrayTransferPrototype.cxx
////

////
//// BEGIN-EXAMPLE DerivedArrayTransfer.cxx
////
namespace vtkm {
namespace cont {
namespace internal {

template<typename ArrayHandleType1,
         typename ArrayHandleType2,
         typename Device>
class ArrayTransfer<
    typename ArrayHandleType1::ValueType,
    StorageTagConcatenate<ArrayHandleType1,ArrayHandleType2>,
    Device>
{
public:
  typedef typename ArrayHandleType1::ValueType ValueType;

private:
  typedef StorageTagConcatenate<ArrayHandleType1,ArrayHandleType2>
      StorageTag;
  typedef vtkm::cont::internal::Storage<ValueType,StorageTag>
      StorageType;

public:
  typedef typename StorageType::PortalType PortalControl;
  typedef typename StorageType::PortalConstType PortalConstControl;

  typedef ArrayPortalConcatenate<
      typename ArrayHandleType1::template ExecutionTypes<Device>::Portal,
      typename ArrayHandleType2::template ExecutionTypes<Device>::Portal>
    PortalExecution;
  typedef ArrayPortalConcatenate<
      typename ArrayHandleType1::template ExecutionTypes<Device>::PortalConst,
      typename ArrayHandleType2::template ExecutionTypes<Device>::PortalConst>
    PortalConstExecution;

  VTKM_CONT_EXPORT
  ArrayTransfer(StorageType *storage)
    : Array1(storage->GetArray1()), Array2(storage->GetArray2())
  {  }

  VTKM_CONT_EXPORT
  vtkm::Id GetNumberOfValues() const {
    return this->Array1.GetNumberOfValues() + this->Array2.GetNumberOfValues();
  }

  VTKM_CONT_EXPORT
  PortalConstExecution PrepareForInput(bool vtkmNotUsed(updateData)) {
    return PortalConstExecution(this->Array1.PrepareForInput(Device()),
                                this->Array2.PrepareForInput(Device()));
  }

  VTKM_CONT_EXPORT
  PortalExecution PrepareForInPlace(bool vtkmNotUsed(updateData)) {
    return PortalExecution(this->Array1.PrepareForInPlace(Device()),
                           this->Array2.PrepareForInPlace(Device()));
  }

  VTKM_CONT_EXPORT
  PortalExecution PrepareForOutput(vtkm::Id numberOfValues)
  {
    // This implementation of allocate, which allocates the same amount in both
    // arrays, is arbitrary. It could, for example, leave the size of Array1
    // alone and change the size of Array2. Or, probably most likely, it could
    // simply throw an error and state that this operation is invalid.
    vtkm::Id half = numberOfValues/2;
    return PortalExecution(
          this->Array1.PrepareForOutput(numberOfValues-half, Device()),
          this->Array2.PrepareForOutput(half, Device()));
  }

  VTKM_CONT_EXPORT
  void RetrieveOutputData(StorageType *vtkmNotUsed(storage)) const {
    // Implementation of this method should be unnecessary. The internal
    // array handles should automatically retrieve the output data as
    // necessary.
  }

  VTKM_CONT_EXPORT
  void Shrink(vtkm::Id numberOfValues) {
    if (numberOfValues < this->Array1.GetNumberOfValues())
    {
      this->Array1.Shrink(numberOfValues);
      this->Array2.Shrink(0);
    }
    else
    {
      this->Array2.Shrink(numberOfValues - this->Array1.GetNumberOfValues());
    }
  }

  VTKM_CONT_EXPORT
  void ReleaseResources() {
    this->Array1.ReleaseResourcesExecution();
    this->Array2.ReleaseResourcesExecution();
  }

private:
  ArrayHandleType1 Array1;
  ArrayHandleType2 Array2;
};

}
}
} // namespace vtkm::cont::internal
////
//// END-EXAMPLE DerivedArrayTransfer.cxx
////

////
//// BEGIN-EXAMPLE DerivedArrayHandle.cxx
////
template<typename ArrayHandleType1, typename ArrayHandleType2>
class ArrayHandleConcatenate
    : public vtkm::cont::ArrayHandle<
        typename ArrayHandleType1::ValueType,
        StorageTagConcatenate<ArrayHandleType1,ArrayHandleType2> >
{
public:
  VTKM_ARRAY_HANDLE_SUBCLASS(
      ArrayHandleConcatenate,
      (ArrayHandleConcatenate<ArrayHandleType1,ArrayHandleType2>),
      (vtkm::cont::ArrayHandle<
         typename ArrayHandleType1::ValueType,
         StorageTagConcatenate<ArrayHandleType1,ArrayHandleType2> >));

private:
  typedef vtkm::cont::internal::Storage<ValueType,StorageTag> StorageType;

public:
  VTKM_CONT_EXPORT
  ArrayHandleConcatenate(const ArrayHandleType1 &array1,
                         const ArrayHandleType2 &array2)
    : Superclass(StorageType(array1, array2)) {  }
};
////
//// END-EXAMPLE DerivedArrayHandle.cxx
////

#include <vtkm/cont/ArrayHandleIndex.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/DynamicArrayHandle.h>

#include <vtkm/cont/testing/Testing.h>

namespace {

void Test()
{
  const vtkm::Id HALF_ARRAY_SIZE = 25;
  const vtkm::Id ARRAY_SIZE = 2*HALF_ARRAY_SIZE;
  vtkm::cont::ArrayHandleIndex inputArray(ARRAY_SIZE);

  typedef vtkm::cont::ArrayHandle<vtkm::Id> BaseArrayType;
  BaseArrayType array1;
  BaseArrayType array2;

  ArrayHandleConcatenate<BaseArrayType,BaseArrayType>
      concatArray(array1, array2);

  vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>::Copy(
        inputArray, concatArray);

  VTKM_TEST_ASSERT(array1.GetNumberOfValues() == HALF_ARRAY_SIZE,"Wrong size.");
  VTKM_TEST_ASSERT(array2.GetNumberOfValues() == HALF_ARRAY_SIZE,"Wrong size.");
  for (vtkm::Id index = 0; index < HALF_ARRAY_SIZE; index++)
  {
    VTKM_TEST_ASSERT(array1.GetPortalConstControl().Get(index) == index,
                     "Wrong value.");
    VTKM_TEST_ASSERT(
          array2.GetPortalConstControl().Get(index) == index+HALF_ARRAY_SIZE,
          "Wrong value.");
  }

  ArrayHandleConcatenate<BaseArrayType,BaseArrayType>
      switchedArray(array2, array1);
  BaseArrayType targetArray;

  vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>::Copy(
        switchedArray, targetArray);
  for (vtkm::Id index = 0; index < HALF_ARRAY_SIZE; index++)
  {
    VTKM_TEST_ASSERT(targetArray.GetPortalConstControl().Get(index)
                       == index+HALF_ARRAY_SIZE,
                     "Wrong value.");
  }
  for (vtkm::Id index = HALF_ARRAY_SIZE; index < ARRAY_SIZE; index++)
  {
    VTKM_TEST_ASSERT(targetArray.GetPortalConstControl().Get(index)
                       == index-HALF_ARRAY_SIZE,
                     "Wrong value.");
  }

  // Check all PrepareFor* methods.
  concatArray.ReleaseResourcesExecution();
  concatArray.PrepareForInput(VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
  concatArray.PrepareForInPlace(VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
  concatArray.PrepareForOutput(ARRAY_SIZE+1, VTKM_DEFAULT_DEVICE_ADAPTER_TAG());

  // Make sure that the array can go into and out of a dynamic array handle.
  vtkm::cont::DynamicArrayHandle dynamicArray = concatArray;

  ArrayHandleConcatenate<BaseArrayType,BaseArrayType> concatArrayShallowCopy;
  VTKM_TEST_ASSERT(concatArray != concatArrayShallowCopy, "Huh?");
  dynamicArray.CopyTo(concatArrayShallowCopy);
  VTKM_TEST_ASSERT(concatArray == concatArrayShallowCopy,
                   "Did not get array out of dynamic.");
}

} // anonymous namespace

int ArrayHandleDerived(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
