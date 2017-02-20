#include <vtkm/cont/ArrayHandleConstant.h>

#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/ScatterUniform.h>
#include <vtkm/worklet/WorkletMapField.h>

#include <vtkm/cont/testing/Testing.h>

namespace {

////
//// BEGIN-EXAMPLE ScatterUniform.cxx
////
struct InterleaveArrays : vtkm::worklet::WorkletMapField
{
  typedef void ControlSignature(FieldIn<>, FieldIn<>, FieldOut<>);
  typedef void ExecutionSignature(_1, _2, _3, VisitIndex);
  using InputDomain = _1;

  using ScatterType = vtkm::worklet::ScatterUniform;

  VTKM_CONT
  ScatterType GetScatter() const { return vtkm::worklet::ScatterUniform(2); }

  template<typename T>
  VTKM_EXEC
  void operator()(const T &input0,
                  const T &input1,
                  T &output,
                  vtkm::IdComponent visitIndex) const
  {
    if (visitIndex == 0)
    {
      output = input0;
    }
    else // visitIndex == 1
    {
      output = input1;
    }
  }
};
////
//// END-EXAMPLE ScatterUniform.cxx
////

void Run()
{
  std::cout << "Trying scatter uniform with array interleave." << std::endl;

  static const vtkm::Id ARRAY_SIZE = 10;
  static const vtkm::Id value0 = 8;
  static const vtkm::Id value1 = 42;

  vtkm::cont::ArrayHandle<vtkm::Id> outArray;

  vtkm::worklet::DispatcherMapField<InterleaveArrays> dispatcher;
  dispatcher.Invoke(vtkm::cont::make_ArrayHandleConstant(value0, ARRAY_SIZE),
                    vtkm::cont::make_ArrayHandleConstant(value1, ARRAY_SIZE),
                    outArray);

  vtkm::cont::printSummary_ArrayHandle(outArray, std::cout);
  std::cout << std::endl;
  VTKM_TEST_ASSERT(outArray.GetNumberOfValues() == ARRAY_SIZE*2,
                   "Wrong sized array.");
  for (vtkm::Id index = 0; index < ARRAY_SIZE; index++)
  {
    vtkm::Id v0 = outArray.GetPortalConstControl().Get(2*index+0);
    VTKM_TEST_ASSERT(v0 == value0, "Bad value in array.");
    vtkm::Id v1 = outArray.GetPortalConstControl().Get(2*index+1);
    VTKM_TEST_ASSERT(v1 == value1, "Bad value in array.");
  }
}

} // anonymous namespace

int ScatterUniform(int, char*[])
{
  return vtkm::cont::testing::Testing::Run(Run);
}
