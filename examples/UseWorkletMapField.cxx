#include <vtkm/exec/ExecutionWholeArray.h>

////
//// BEGIN-EXAMPLE UseWorkletMapField.cxx
////
#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/WorkletMapField.h>

#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/DynamicArrayHandle.h>

#include <vtkm/VectorAnalysis.h>

//// PAUSE-EXAMPLE
namespace {
//// RESUME-EXAMPLE

class Magnitude : public vtkm::worklet::WorkletMapField
{
public:
  //// BEGIN-EXAMPLE ControlSignature.cxx
  typedef void ControlSignature(FieldIn<VecAll> inputVectors,
                                FieldOut<Scalar> outputMagnitudes);
  //// END-EXAMPLE ControlSignature.cxx
  //// BEGIN-EXAMPLE ExecutionSignature.cxx
  typedef _2 ExecutionSignature(_1);
  //// END-EXAMPLE ExecutionSignature.cxx

  //// BEGIN-EXAMPLE InputDomain.cxx
  typedef _1 InputDomain;
  //// END-EXAMPLE InputDomain.cxx

  //// BEGIN-EXAMPLE WorkletOperator.cxx
  template<typename T, vtkm::IdComponent Size>
  VTKM_EXEC_EXPORT
  T operator()(const vtkm::Vec<T,Size> &inVector) const
  {
  //// END-EXAMPLE WorkletOperator.cxx
    return vtkm::Magnitude(inVector);
  }
};

VTKM_CONT_EXPORT
vtkm::cont::DynamicArrayHandle
InvokeMagnitude(vtkm::cont::DynamicArrayHandle input)
{
  vtkm::cont::ArrayHandle<vtkm::Float64> output;

  vtkm::worklet::DispatcherMapField<Magnitude> dispatcher;
  dispatcher.Invoke(input, output);

  return vtkm::cont::DynamicArrayHandle(output);
}
////
//// END-EXAMPLE UseWorkletMapField.cxx
////

////
//// BEGIN-EXAMPLE RandomArrayAccess.cxx
////
struct ReverseArrayCopy : vtkm::worklet::WorkletMapField
{
  typedef void ControlSignature(FieldIn<> inputArray,
                                ExecObject outputArray);
  typedef void ExecutionSignature(_1, _2, WorkIndex);
  typedef _1 InputDomain;

  template<typename InputType, typename OutputArrayPortalType>
  VTKM_EXEC_EXPORT
  void operator()(const InputType &inputValue,
                  const OutputArrayPortalType &outputArrayPortal,
                  vtkm::Id workIndex) const
  {
    vtkm::Id outIndex = outputArrayPortal.GetNumberOfValues() - workIndex - 1;
    if (outIndex >= 0)
    {
      outputArrayPortal.Set(outIndex, inputValue);
    }
    else
    {
      this->RaiseError("Output array not sized correctly.");
    }
  }
};

template<typename T, typename Storage>
VTKM_CONT_EXPORT
vtkm::cont::ArrayHandle<T>
InvokeReverseArrayCopy(const vtkm::cont::ArrayHandle<T,Storage> &inArray)
{
  vtkm::cont::ArrayHandle<T> outArray;
  vtkm::exec::ExecutionWholeArray<T>
      outArrayExecObject(outArray, inArray.GetNumberOfValues());

  vtkm::worklet::DispatcherMapField<ReverseArrayCopy> dispatcher;
  dispatcher.Invoke(inArray, outArrayExecObject);

  return outArray;
}
////
//// END-EXAMPLE RandomArrayAccess.cxx
////

} // anonymous namespace

#include <vtkm/cont/testing/Testing.h>

namespace {

void Test()
{
  static const vtkm::Id ARRAY_SIZE = 10;
  typedef vtkm::Vec<vtkm::FloatDefault,3> Vec3;
  Vec3 inputBuffer[ARRAY_SIZE];

  for (vtkm::Id index = 0; index < ARRAY_SIZE; index++)
  {
    inputBuffer[index] = TestValue(index, Vec3());
  }

  vtkm::cont::ArrayHandle<Vec3> inputArray =
      vtkm::cont::make_ArrayHandle(inputBuffer, ARRAY_SIZE);

  vtkm::cont::DynamicArrayHandle outputDynamicArray =
      InvokeMagnitude(inputArray);
  vtkm::cont::ArrayHandle<vtkm::Float64> outputArray =
      outputDynamicArray.CastToArrayHandle(vtkm::Float64(),
                                           VTKM_DEFAULT_STORAGE_TAG());

  VTKM_TEST_ASSERT(outputArray.GetNumberOfValues() == ARRAY_SIZE,
                   "Bad output array size.");
  for (vtkm::Id index = 0; index < ARRAY_SIZE; index++)
  {
    Vec3 testValue = TestValue(index, Vec3());
    vtkm::Float64 expectedValue = sqrt(vtkm::dot(testValue,testValue));
    vtkm::Float64 gotValue = outputArray.GetPortalConstControl().Get(index);
    VTKM_TEST_ASSERT(test_equal(expectedValue, gotValue), "Got bad value.");
  }

  vtkm::cont::ArrayHandle<Vec3> outputArray2 =
      InvokeReverseArrayCopy(inputArray);
  VTKM_TEST_ASSERT(outputArray2.GetNumberOfValues() == ARRAY_SIZE,
                   "Bad output array size.");
  for (vtkm::Id index = 0; index < ARRAY_SIZE; index++)
  {
    Vec3 expectedValue = TestValue(ARRAY_SIZE - index - 1, Vec3());
    Vec3 gotValue = outputArray2.GetPortalConstControl().Get(index);
    VTKM_TEST_ASSERT(test_equal(expectedValue, gotValue), "Got bad value.");
  }
}

} // anonymous namespace

int UseWorkletMapField(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
