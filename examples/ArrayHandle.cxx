#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayPortalToIterators.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/StorageBasic.h>

#include <vtkm/exec/FunctorBase.h>

#include <vtkm/cont/testing/Testing.h>

#include <algorithm>
#include <vector>

namespace {

vtkm::Float32 TestValue(vtkm::Id index)
{
  return 1 + 0.001 * index;
}

void CheckArrayValues(const vtkm::cont::ArrayHandle<vtkm::Float32> &array,
                      vtkm::Float32 factor = 1)
{
  // So far all the examples are using 50 entries. Could change.
  VTKM_TEST_ASSERT(array.GetNumberOfValues() == 50, "Wrong number of values");

  for (vtkm::Id index = 0; index < array.GetNumberOfValues(); index++)
  {
    VTKM_TEST_ASSERT(
          array.GetPortalConstControl().Get(index) == TestValue(index)*factor,
          "Bad data value.");
  }
}

void BasicConstruction()
{
  ////
  //// BEGIN-EXAMPLE CreateArrayHandle.cxx
  ////
vtkm::cont::ArrayHandle<vtkm::Float32> outputArray;
  ////
  //// END-EXAMPLE CreateArrayHandle.cxx
  ////

  ////
  //// BEGIN-EXAMPLE ArrayHandleStorageParameter.cxx
  ////
vtkm::cont::ArrayHandle<vtkm::Float32,vtkm::cont::StorageTagBasic> arrayHandle;
  ////
  //// END-EXAMPLE ArrayHandleStorageParameter.cxx
  ////
}

void ArrayHandleFromCArray()
{
  ////
  //// BEGIN-EXAMPLE ArrayHandleFromCArray.cxx
  ////
  vtkm::Float32 dataBuffer[50];
  // Populate dataBuffer with meaningful data. Perhaps read data from a file.
  //// PAUSE-EXAMPLE
  for (vtkm::Id index = 0; index < 50; index++)
  {
    dataBuffer[index] = TestValue(index);
  }
  //// RESUME-EXAMPLE

  vtkm::cont::ArrayHandle<vtkm::Float32> inputArray =
      vtkm::cont::make_ArrayHandle(dataBuffer, 50);
  ////
  //// END-EXAMPLE ArrayHandleFromCArray.cxx
  ////

  CheckArrayValues(inputArray);
}

vtkm::Float32 GetValueForArray(vtkm::Id index)
{
  return TestValue(index);
}

void AllocateAndFillArrayHandle()
{
  ////
  //// BEGIN-EXAMPLE ArrayHandlePopulate.cxx
  ////
  ////
  //// BEGIN-EXAMPLE ArrayHandleAllocate.cxx
  ////
  vtkm::cont::ArrayHandle<vtkm::Float32> arrayHandle;

  const vtkm::Id ARRAY_SIZE = 50;
  arrayHandle.Allocate(ARRAY_SIZE);
  ////
  //// END-EXAMPLE ArrayHandleAllocate.cxx
  ////

  typedef vtkm::cont::ArrayHandle<vtkm::Float32>::PortalControl PortalType;
  PortalType portal = arrayHandle.GetPortalControl();

  for (vtkm::Id index = 0; index < ARRAY_SIZE; index++)
  {
    portal.Set(index, GetValueForArray(index));
  }
  ////
  //// END-EXAMPLE ArrayHandlePopulate.cxx
  ////

  CheckArrayValues(arrayHandle);
}

////
//// BEGIN-EXAMPLE ArrayOutOfScope.cxx
////
VTKM_CONT_EXPORT
vtkm::cont::ArrayHandle<vtkm::Float32> BadDataLoad()
{
  ////
  //// BEGIN-EXAMPLE ArrayHandleFromVector.cxx
  ////
  std::vector<vtkm::Float32> dataBuffer;
  // Populate dataBuffer with meaningful data. Perhaps read data from a file.
  //// PAUSE-EXAMPLE
  dataBuffer.resize(50);
  for (vtkm::Id index = 0; index < 50; index++)
  {
    dataBuffer[index] = TestValue(index);
  }
  //// RESUME-EXAMPLE

  vtkm::cont::ArrayHandle<vtkm::Float32> inputArray =
      vtkm::cont::make_ArrayHandle(dataBuffer);
  ////
  //// END-EXAMPLE ArrayHandleFromVector.cxx
  ////
  //// PAUSE-EXAMPLE
  CheckArrayValues(inputArray);
  //// RESUME-EXAMPLE

  return inputArray;
  // THIS IS WRONG! At this point dataBuffer goes out of scope and deletes its
  // memory. However, inputArray has a pointer to that memory, which becomes an
  // invalid pointer in the returned object. Bad things will happen when the
  // ArrayHandle is used.
}

VTKM_CONT_EXPORT
vtkm::cont::ArrayHandle<vtkm::Float32> SafeDataLoad()
{
  std::vector<vtkm::Float32> dataBuffer;
  // Populate dataBuffer with meaningful data. Perhaps read data from a file.
  //// PAUSE-EXAMPLE
  dataBuffer.resize(50);
  for (vtkm::Id index = 0; index < 50; index++)
  {
    dataBuffer[index] = TestValue(index);
  }
  //// RESUME-EXAMPLE

  vtkm::cont::ArrayHandle<vtkm::Float32> tmpArray =
      vtkm::cont::make_ArrayHandle(dataBuffer);

  // This copies the data from one ArrayHandle to another (in the execution
  // environment). Although it is an extraneous copy, it is usually pretty fast
  // on a parallel device. Another option is to make sure that the buffer in
  // the std::vector never goes out of scope before all the ArrayHandle
  // references, but this extra step allows the ArrayHandle to manage its own
  // memory and ensure everything is valid.
  vtkm::cont::ArrayHandle<vtkm::Float32> inputArray;
  vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>::Copy(
      tmpArray, inputArray);

  return inputArray;
  // This is safe.
}
////
//// END-EXAMPLE ArrayOutOfScope.cxx
////

void ArrayHandleFromVector()
{
  BadDataLoad();
}

void CheckSafeDataLoad()
{
  vtkm::cont::ArrayHandle<vtkm::Float32> inputArray = SafeDataLoad();
  CheckArrayValues(inputArray);
}

////
//// BEGIN-EXAMPLE SimpleArrayPortal.cxx
////
template<typename T>
class SimpleScalarArrayPortal
{
public:
  typedef T ValueType;

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

private:
  ValueType *Array;
  vtkm::Id NumberOfValues;
};
////
//// END-EXAMPLE SimpleArrayPortal.cxx
////

////
//// BEGIN-EXAMPLE ArrayPortalToIterators.cxx
////
template<typename PortalType>
VTKM_CONT_EXPORT
std::vector<typename PortalType::ValueType>
CopyArrayPortalToVector(const PortalType &portal)
{
  typedef typename PortalType::ValueType ValueType;
  std::vector<ValueType> result(portal.GetNumberOfValues());

  vtkm::cont::ArrayPortalToIterators<PortalType> iterators(portal);

  std::copy(iterators.GetBegin(), iterators.GetEnd(), result.begin());

  return result;
}
////
//// END-EXAMPLE ArrayPortalToIterators.cxx
////

void TestArrayPortalVectors()
{
  vtkm::cont::ArrayHandle<vtkm::Float32> inputArray = SafeDataLoad();
  std::vector<vtkm::Float32> buffer =
      CopyArrayPortalToVector(inputArray.GetPortalConstControl());

  VTKM_TEST_ASSERT(buffer.size() == inputArray.GetNumberOfValues(),
                   "Vector was sized wrong.");

  for (vtkm::Id index = 0; index < inputArray.GetNumberOfValues(); index++)
  {
    VTKM_TEST_ASSERT(buffer[index] == TestValue(index), "Bad data value.");
  }

  SimpleScalarArrayPortal<vtkm::Float32> portal(&buffer.at(0), buffer.size());

  ////
  //// BEGIN-EXAMPLE ArrayPortalToIteratorBeginEnd.cxx
  ////
  std::vector<vtkm::Float32> myContainer(portal.GetNumberOfValues());

  std::copy(vtkm::cont::ArrayPortalToIteratorBegin(portal),
            vtkm::cont::ArrayPortalToIteratorEnd(portal),
            myContainer.begin());
  ////
  //// END-EXAMPLE ArrayPortalToIteratorBeginEnd.cxx
  ////

  for (vtkm::Id index = 0; index < inputArray.GetNumberOfValues(); index++)
  {
    VTKM_TEST_ASSERT(myContainer[index] == TestValue(index), "Bad data value.");
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
  std::sort(vtkm::cont::ArrayPortalToIteratorBegin(readwritePortal),
            vtkm::cont::ArrayPortalToIteratorEnd(readwritePortal));

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

////
//// BEGIN-EXAMPLE ExecutionPortals.cxx
////
template<typename T, typename Device>
struct DoubleFunctor : public vtkm::exec::FunctorBase
{
  typedef typename vtkm::cont::ArrayHandle<T>::
      template ExecutionTypes<Device>::PortalConst InputPortalType;
  typedef typename vtkm::cont::ArrayHandle<T>::
      template ExecutionTypes<Device>::Portal OutputPortalType;

  VTKM_CONT_EXPORT
  DoubleFunctor(InputPortalType inputPortal, OutputPortalType outputPortal)
    : InputPortal(inputPortal), OutputPortal(outputPortal) {  }

  VTKM_EXEC_EXPORT
  void operator()(vtkm::Id index) const {
    this->OutputPortal.Set(index, 2*this->InputPortal.Get(index));
  }

  InputPortalType InputPortal;
  OutputPortalType OutputPortal;
};

template<typename T, typename Device>
void DoubleArray(vtkm::cont::ArrayHandle<T> inputArray,
                 vtkm::cont::ArrayHandle<T> outputArray,
                 Device)
{
  vtkm::Id numValues = inputArray.GetNumberOfValues();

  DoubleFunctor<T, Device> functor(
        inputArray.PrepareForInput(Device()),
        outputArray.PrepareForOutput(numValues, Device()));

  vtkm::cont::DeviceAdapterAlgorithm<Device>::Schedule(functor, numValues);
}
////
//// END-EXAMPLE ExecutionPortals.cxx
////

void TestExecutionPortalsExample()
{
  vtkm::cont::ArrayHandle<vtkm::Float32> inputArray = SafeDataLoad();
  CheckArrayValues(inputArray);
  vtkm::cont::ArrayHandle<vtkm::Float32> outputArray;
  DoubleArray(inputArray, outputArray, VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
  CheckArrayValues(outputArray, 2);
}

void Test()
{
  BasicConstruction();
  ArrayHandleFromCArray();
  ArrayHandleFromVector();
  AllocateAndFillArrayHandle();
  CheckSafeDataLoad();
  TestArrayPortalVectors();
  TestControlPortalsExample();
  TestExecutionPortalsExample();
}

} // anonymous namespace

int ArrayHandle(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
