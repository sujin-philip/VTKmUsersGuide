////
//// BEGIN-EXAMPLE DerivedArrayPortal.cxx
////
#include <vtkm/cont/ArrayContainerControlImplicit.h>
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayPortal.h>
#include <vtkm/cont/Assert.h>
#include <vtkm/cont/internal/IteratorFromArrayPortal.h>

template<typename P1, typename P2>
class ArrayPortalConcatenate
{
public:
  typedef P1 PortalType1;
  typedef P2 PortalType2;
  typedef typename PortalType1::ValueType ValueType;

  VTKM_EXEC_CONT_EXPORT
  ArrayPortalConcatenate() : Portal1(), Portal2() {  }

  VTKM_EXEC_CONT_EXPORT
  ArrayPortalConcatenate(const PortalType1 &portal1, const PortalType2 portal2)
    : Portal1(portal1), Portal2(portal2) {  }

  /// Copy constructor for any other ArrayPortalConcatenate with a portal type
  /// that can be copied to this portal type. This allows us to do any type
  /// casting that the portals do (like the non-const to const cast).
  template<typename OtherP1, typename OtherP2>
  VTKM_EXEC_CONT_EXPORT
  ArrayPortalConcatenate(const ArrayPortalConcatenate<OtherP1,OtherP2> &src)
    : Portal1(src.GetPortal1()), Portal2(src.GetPortal2()) {  }

  VTKM_EXEC_CONT_EXPORT
  vtkm::Id GetNumberOfValues() const {
    return
        this->Portal1.GetNumberOfValues() + this->Portal2.GetNumberOfValues();
  }

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

  typedef vtkm::cont::internal::IteratorFromArrayPortal<
      ArrayPortalConcatenate<PortalType1,PortalType2> > IteratorType;

  VTKM_EXEC_CONT_EXPORT
  IteratorType GetIteratorBegin() const {
    return IteratorType(*this);
  }

  VTKM_EXEC_CONT_EXPORT
  IteratorType GetIteratorEnd() const {
    return IteratorType(*this, this->GetNumberOfValues());
  }

  VTKM_EXEC_CONSTANT_EXPORT
  const PortalType1 &GetPortal1() const { return this->Portal1; }
  VTKM_EXEC_CONSTANT_EXPORT
  const PortalType2 &GetPortal2() const { return this->Portal2; }

private:
  PortalType1 Portal1;
  PortalType2 Portal2;
};
////
//// END-EXAMPLE DerivedArrayPortal.cxx
////

////
//// BEGIN-EXAMPLE DerivedArrayContainer.cxx
////
template<typename ArrayHandleType1, typename ArrayHandleType2>
struct ArrayContainerControlTagConcatenate {  };

namespace vtkm {
namespace cont {
namespace internal {

template<typename T, typename Container1, typename Container2>
class ArrayContainerControl<
    T,
    ArrayContainerControlTagConcatenate<
      vtkm::cont::ArrayHandle<T, Container1>,
      vtkm::cont::ArrayHandle<T, Container2> > >
{
  typedef vtkm::cont::ArrayHandle<T, Container1> ArrayHandleType1;
  typedef vtkm::cont::ArrayHandle<T, Container2> ArrayHandleType2;

public:
  typedef T ValueType;

  typedef ArrayPortalConcatenate<
      typename ArrayHandleType1::PortalControl,
      typename ArrayHandleType2::PortalControl> PortalType;
  typedef ArrayPortalConcatenate<
      typename ArrayHandleType1::PortalConstControl,
      typename ArrayHandleType2::PortalConstControl> PortalConstType;

  VTKM_CONT_EXPORT
  ArrayContainerControl() : Valid(false) {  }

  VTKM_CONT_EXPORT
  ArrayContainerControl(const ArrayHandleType1 array1,
                        const ArrayHandleType2 array2)
    : Array1(array1), Array2(array2), Valid(true) {  }

  VTKM_CONT_EXPORT
  PortalType GetPortal() {
    VTKM_ASSERT_CONT(this->Valid);
    return PortalType(this->Array1.GetPortalControl(),
                      this->Array2.GetPortalControl());
  }

  VTKM_CONT_EXPORT
  PortalConstType GetPortalConst() const {
    VTKM_ASSERT_CONT(this->Valid);
    return PortalConstType(this->Array1.GetPortalConstControl(),
                           this->Array2.GetPortalConstControl());
  }

  VTKM_CONT_EXPORT
  vtkm::Id GetNumberOfValues() const {
    VTKM_ASSERT_CONT(this->Valid);
    return this->Array1.GetNumberOfValues() + this->Array2.GetNumberOfValues();
  }

  VTKM_CONT_EXPORT
  void Allocate(vtkm::Id numberOfValues) {
    VTKM_ASSERT_CONT(this->Valid);
    // This implementation of allocate, which allocates the same amount in both
    // arrays, is arbitrary. It could, for example, leave the size of Array1
    // alone and change the size of Array2. Or, probably most likely, it could
    // simply throw an error and state that this operation is invalid.
    vtkm::Id half = numberOfValues/2;
    this->Array1.PrepareForOutput(numberOfValues-half);
    this->Array2.PrepareForOutput(half);
  }

  VTKM_CONT_EXPORT
  void Shrink(vtkm::Id numberOfValues) {
    VTKM_ASSERT_CONT(this->Valid);
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
    VTKM_ASSERT_CONT(this->Valid);
    this->Array1.ReleaseResources();
    this->Array2.ReleaseResources();
  }

  // Requried for later use in ArrayTransfer class.
  VTKM_CONT_EXPORT
  const ArrayHandleType1 &GetArray1() const {
    VTKM_ASSERT_CONT(this->Valid);
    return this->Array1;
  }
  VTKM_CONT_EXPORT
  const ArrayHandleType2 &GetArray2() const {
    VTKM_ASSERT_CONT(this->Valid);
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
//// END-EXAMPLE DerivedArrayContainer.cxx
////

////
//// BEGIN-EXAMPLE ArrayTransferPrototype.cxx
////
namespace vtkm {
namespace cont {
namespace internal {

template<typename T,typename ArrayContainerControlTag,typename DeviceAdapterTag>
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
    ArrayContainerControlTagConcatenate<ArrayHandleType1,ArrayHandleType2>,
    Device>
{
public:
  typedef typename ArrayHandleType1::ValueType ValueType;

private:
  typedef ArrayContainerControlTagConcatenate<ArrayHandleType1,ArrayHandleType2>
      ContainerTag;
  typedef vtkm::cont::internal::ArrayContainerControl<ValueType,ContainerTag>
      ContainerType;

public:
  typedef typename ContainerType::PortalType PortalControl;
  typedef typename ContainerType::PortalConstType PortalConstControl;

  typedef ArrayPortalConcatenate<
      typename ArrayHandleType1::template ExecutionTypes<Device>::Portal,
      typename ArrayHandleType2::template ExecutionTypes<Device>::Portal>
    PortalExecution;
  typedef ArrayPortalConcatenate<
      typename ArrayHandleType1::template ExecutionTypes<Device>::PortalConst,
      typename ArrayHandleType2::template ExecutionTypes<Device>::PortalConst>
    PortalConstExecution;

  VTKM_CONT_EXPORT
  ArrayTransfer()
    : ArraysValid(false),
      ExecutionPortalConstValid(false),
      ExecutionPortalValid(false)
  {  }

  VTKM_CONT_EXPORT
  vtkm::Id GetNumberOfValues() const {
    VTKM_ASSERT_CONT(this->ArraysValid);
    return this->Array1.GetNumberOfValues() + this->Array2.GetNumberOfValues();
  }

  VTKM_CONT_EXPORT
  void LoadDataForInput(PortalConstControl vtkmNotUsed(portal)) {
    throw vtkm::cont::ErrorControlInternal(
          "Wrong version of LoadDataForInput called. "
          "ArrayHandle must be in bad state.");
  }

  VTKM_CONT_EXPORT
  void LoadDataForInput(const ContainerType &container) {
    this->Array1 = container.GetArray1();
    this->Array2 = container.GetArray2();
    this->ArraysValid = true;

    this->ExecutionPortalConst =
        PortalConstExecution(this->Array1.PrepareForInput(Device()),
                             this->Array2.PrepareForInput(Device()));
    this->ExecutionPortalConstValid = true;
    this->ExecutionPortalValid = false;
  }

  VTKM_CONT_EXPORT
  void LoadDataForInPlace(ContainerType &container) {
    this->Array1 = container.GetArray1();
    this->Array2 = container.GetArray2();
    this->ArraysValid = true;

    this->ExecutionPortal =
        PortalExecution(this->Array1.PrepareForInPlace(Device()),
                        this->Array2.PrepareForInPlace(Device()));
    this->ExecutionPortalConst = this->ExecutionPortal;
    this->ExecutionPortalConstValid = true;
    this->ExecutionPortalValid = true;
  }

  VTKM_CONT_EXPORT
  void AllocateArrayForOutput(ContainerType &container, vtkm::Id numberOfValues)
  {
    this->Array1 = container.GetArray1();
    this->Array2 = container.GetArray2();
    this->ArraysValid = true;

    // This implementation of allocate, which allocates the same amount in both
    // arrays, is arbitrary. It could, for example, leave the size of Array1
    // alone and change the size of Array2. Or, probably most likely, it could
    // simply throw an error and state that this operation is invalid.
    vtkm::Id half = numberOfValues/2;
    this->ExecutionPortal =
        PortalExecution(
          this->Array1.PrepareForOutput(numberOfValues-half, Device()),
          this->Array2.PrepareForOutput(half, Device()));
    this->ExecutionPortalConst = this->ExecutionPortal;
    this->ExecutionPortalConstValid = true;
    this->ExecutionPortalValid = true;
  }

  VTKM_CONT_EXPORT
  void RetrieveOutputData(ContainerType &vtkmNotUsed(container)) const {
    // Implementation of this method should be unnecessary. The internal
    // array handles should automatically retrieve the output data as
    // necessary.
  }

  VTKM_CONT_EXPORT
  void Shrink(vtkm::Id numberOfValues) {
    VTKM_ASSERT_CONT(this->ArraysValid);
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
  PortalExecution GetPortalExecution() {
    VTKM_ASSERT_CONT(this->ExecutionPortalValid);
    return this->ExecutionPortal;
  }

  VTKM_CONT_EXPORT
  PortalConstExecution GetPortalConstExecution() const {
    VTKM_ASSERT_CONT(this->ExecutionPortalConstValid);
    return this->ExecutionPortalConst;
  }

  VTKM_CONT_EXPORT
  void ReleaseResources() {
    VTKM_ASSERT_CONT(this->ArraysValid);
    this->Array1.ReleaseResourcesExecution();
    this->Array2.ReleaseResourcesExecution();
    this->ExecutionPortalValid = false;
    this->ExecutionPortalConstValid = false;
  }

private:
  ArrayHandleType1 Array1;
  ArrayHandleType2 Array2;
  bool ArraysValid;
  PortalConstExecution ExecutionPortalConst;
  bool ExecutionPortalConstValid;
  PortalExecution ExecutionPortal;
  bool ExecutionPortalValid;
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
        ArrayContainerControlTagConcatenate<ArrayHandleType1,ArrayHandleType2> >
{
  typedef ArrayContainerControlTagConcatenate<ArrayHandleType1,ArrayHandleType2>
      ContainerTag;
  typedef vtkm::cont::ArrayHandle<
        typename ArrayHandleType1::ValueType, ContainerTag>
      Superclass;
  typedef typename Superclass::ValueType ValueType;
  typedef vtkm::cont::internal::ArrayContainerControl<ValueType,ContainerTag>
      ContainerType;

public:
  ArrayHandleConcatenate(const ArrayHandleType1 &array1,
                         const ArrayHandleType2 &array2)
    : Superclass(ContainerType(array1, array2)) {  }
};
////
//// END-EXAMPLE DerivedArrayHandle.cxx
////

#include <vtkm/cont/ArrayHandleCounting.h>
#include <vtkm/cont/DeviceAdapter.h>

#include <vtkm/cont/testing/Testing.h>

namespace {

void Test()
{
  const vtkm::Id HALF_ARRAY_SIZE = 25;
  const vtkm::Id ARRAY_SIZE = 2*HALF_ARRAY_SIZE;
  vtkm::cont::ArrayHandleCounting<vtkm::Id> inputArray(0, ARRAY_SIZE);

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
}

} // anonymous namespace

int ArrayHandleDerived(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
