#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/DeviceAdapter.h>

#include <vtkm/cont/testing/Testing.h>

#include <algorithm>
#include <vector>

namespace {

vtkm::Scalar TestValue(vtkm::Id index)
{
  return 1 + 0.001 * index;
}

void BasicConstruction()
{
  ////
  //// BEGIN-EXAMPLE CreateArrayHandle.cxx
  ////
  vtkm::cont::ArrayHandle<vtkm::Scalar> outputArray;
  ////
  //// END-EXAMPLE CreateArrayHandle.cxx
  ////
}

void ArrayHandleFromCArray()
{
  ////
  //// BEGIN-EXAMPLE ArrayHandleFromCArray.cxx
  ////
  vtkm::Scalar dataBuffer[50];
  // Populate dataBuffer with meaningful data. Perhaps read data from a file.
  //// PAUSE-EXAMPLE
  for (vtkm::Id index = 0; index < 50; index++)
  {
    dataBuffer[index] = TestValue(index);
  }
  //// RESUME-EXAMPLE

  vtkm::cont::ArrayHandle<vtkm::Scalar> inputArray =
      vtkm::cont::make_ArrayHandle(dataBuffer, 50);
  ////
  //// END-EXAMPLE ArrayHandleFromCArray.cxx
  ////

  for (vtkm::Id index = 0; index < 50; index++)
  {
    VTKM_TEST_ASSERT(
          inputArray.GetPortalConstControl().Get(index) == TestValue(index),
          "Bad data value.");
  }
}

////
//// BEGIN-EXAMPLE ArrayOutOfScope.cxx
////
VTKM_CONT_EXPORT
vtkm::cont::ArrayHandle<vtkm::Scalar> BadDataLoad()
{
  ////
  //// BEGIN-EXAMPLE ArrayHandleFromVector.cxx
  ////
  std::vector<vtkm::Scalar> dataBuffer;
  // Populate dataBuffer with meaningful data. Perhaps read data from a file.
  //// PAUSE-EXAMPLE
  dataBuffer.resize(50);
  for (vtkm::Id index = 0; index < 50; index++)
  {
    dataBuffer[index] = TestValue(index);
  }
  //// RESUME-EXAMPLE

  vtkm::cont::ArrayHandle<vtkm::Scalar> inputArray =
      vtkm::cont::make_ArrayHandle(dataBuffer);
  ////
  //// END-EXAMPLE ArrayHandleFromVector.cxx
  ////
  //// PAUSE-EXAMPLE
  for (vtkm::Id index = 0; index < 50; index++)
  {
    VTKM_TEST_ASSERT(
          inputArray.GetPortalConstControl().Get(index) == TestValue(index),
          "Bad data value.");
  }
  //// RESUME-EXAMPLE

  return inputArray;
  // THIS IS WRONG! At this point dataBuffer goes out of scope and deletes its
  // memory. However, inputArray has a pointer to that memory, which becomes an
  // invalid pointer in the returned object. Bad things will happen when the
  // ArrayHandle is used.
}

VTKM_CONT_EXPORT
vtkm::cont::ArrayHandle<vtkm::Scalar> SafeDataLoad()
{
  std::vector<vtkm::Scalar> dataBuffer;
  // Populate dataBuffer with meaningful data. Perhaps read data from a file.
  //// PAUSE-EXAMPLE
  dataBuffer.resize(50);
  for (vtkm::Id index = 0; index < 50; index++)
  {
    dataBuffer[index] = TestValue(index);
  }
  //// RESUME-EXAMPLE

  vtkm::cont::ArrayHandle<vtkm::Scalar> tmpArray =
      vtkm::cont::make_ArrayHandle(dataBuffer);

  // This copies the data from one ArrayHandle to another (in the execution
  // environment). Although it is an extraneous copy, it is usually pretty fast
  // on a parallel device. Another option is to make sure that the buffer in
  // the std::vector never goes out of scope before all the ArrayHandle
  // references, but this extra step allows the ArrayHandle to manage its own
  // memory and ensure everything is valid.
  vtkm::cont::ArrayHandle<vtkm::Scalar> inputArray;
  vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>::Copy(
      tmpArray, inputArray);

  return inputArray;
  // This is safe.
}
////
//// END-EXAMPLE ArrayOutOfScope.cxx
////

////
//// BEGIN-EXAMPLE SimpleArrayPortal.cxx
////
class SimpleScalarArrayPortal
{
public:
  typedef vtkm::Scalar ValueType;

  // There is no specification for creating array portals, but they generally
  // need a constructor like this to be practical.
  VTKM_EXEC_CONT_EXPORT
  SimpleScalarArrayPortal(ValueType *array, vtkm::Id numberOfValues)
    : Array(array), NumberOfValues(numberOfValues) {  }

  VTKM_EXEC_CONT_EXPORT
  SimpleScalarArrayPortal() : Array(NULL), NumberOfValues(0) {  }

  VTKM_EXEC_CONT_EXPORT
  vtkm::Id GetNumberOfValues() const { return this->NumberOfValues; }

  VTKM_EXEC_CONT_EXPORT
  ValueType Get(vtkm::Id index) const { return this->Array[index]; }

  VTKM_EXEC_CONT_EXPORT
  void Set(vtkm::Id index, ValueType value) const {
    this->Array[index] = value;
  }

  typedef ValueType *IteratorType;

  VTKM_CONT_EXPORT
  IteratorType GetIteratorBegin() const {
    return this->Array;
  }

  VTKM_CONT_EXPORT
  IteratorType GetIteratorEnd() const {
    return this->Array + this->GetNumberOfValues();
  }

private:
  ValueType *Array;
  vtkm::Id NumberOfValues;
};
////
//// END-EXAMPLE SimpleArrayPortal.cxx
////

void ArrayHandleFromVector()
{
  BadDataLoad();
}

void CheckSafeDataLoad()
{
  vtkm::cont::ArrayHandle<vtkm::Scalar> inputArray = SafeDataLoad();
  for (vtkm::Id index = 0; index < 50; index++)
  {
    VTKM_TEST_ASSERT(
          inputArray.GetPortalConstControl().Get(index) == TestValue(index),
          "Bad data value.");
  }
}

////
//// BEGIN-EXAMPLE ControlPortals.cxx
////
template<typename T>
void SortCheckArrayHandle(vtkm::cont::ArrayHandle<T> arrayHandle)
{
  typedef typename vtkm::cont::ArrayHandle<T>::PortalControl
      PortalType;
  typedef typename vtkm::cont::ArrayHandle<T>::PortalConstControl
      PortalConstType;

  PortalType readwritePortal = arrayHandle.GetPortalControl();
  // This is actually pretty dumb. Sorting would be generally faster in
  // parallel in the execution environment using the device adapter algorithms.
  std::sort(readwritePortal.GetIteratorBegin(),
            readwritePortal.GetIteratorEnd());

  PortalConstType readPortal = arrayHandle.GetPortalConstControl();
  for (vtkm::Id index = 1; index < readPortal.GetNumberOfValues(); index++)
  {
    if (readPortal.Get(index-1) > readPortal.Get(index))
    {
      //// PAUSE-EXAMPLE
      VTKM_TEST_FAIL("Sorting is wrong!");
      //// RESUME-EXAMPLE
      std::cout << "Sorting is wrong!" << std::endl;
      break;
    }
  }
}
////
//// END-EXAMPLE ControlPortals.cxx
////

void TestControlPortalsExample()
{
  SortCheckArrayHandle(SafeDataLoad());
}

void Test()
{
  BasicConstruction();
  ArrayHandleFromCArray();
  ArrayHandleFromVector();
  CheckSafeDataLoad();
  TestControlPortalsExample();
}

} // anonymous namespace

int ArrayHandle(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
