////
//// BEGIN-EXAMPLE UseWorkletMapField.cxx
////
#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/WorkletMapField.h>

#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/DynamicArrayHandle.h>

// TODO: Use VTK-m math when available.
#include <math.h>

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
  vtkm::Float64 operator()(const vtkm::Vec<T,Size> &inVector) const
  {
  //// END-EXAMPLE WorkletOperator.cxx
    T magnitudeSquared = vtkm::dot(inVector, inVector);
    // TODO: Use VTK-m math when available.
    return sqrt(vtkm::Float64(magnitudeSquared));
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
}

} // anonymous namespace

int UseWorkletMapField(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
