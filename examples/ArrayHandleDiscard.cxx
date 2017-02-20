#include <vtkm/cont/ArrayHandleCounting.h>
#include <vtkm/cont/ArrayHandleDiscard.h>
#include <vtkm/cont/DeviceAdapter.h>

#include <vtkm/cont/testing/Testing.h>

namespace {

////
//// BEGIN-EXAMPLE ArrayHandleDiscard.cxx
////
template<typename InputArrayType,
         typename OutputArrayType1,
         typename OutputArrayType2>
VTKM_CONT
void DoFoo(InputArrayType input,
           OutputArrayType1 output1,
           OutputArrayType2 output2);

template<typename InputArrayType>
VTKM_CONT
inline
vtkm::cont::ArrayHandle<vtkm::FloatDefault> DoBar(InputArrayType input)
{
  VTKM_IS_ARRAY_HANDLE(InputArrayType);

  vtkm::cont::ArrayHandle<vtkm::FloatDefault> keepOutput;

  vtkm::cont::ArrayHandleDiscard<vtkm::FloatDefault> discardOutput;

  DoFoo(input, keepOutput, discardOutput);

  return keepOutput;
}
////
//// END-EXAMPLE ArrayHandleDiscard.cxx
////

template<typename InputArrayType,
         typename OutputArrayType1,
         typename OutputArrayType2>
VTKM_CONT
inline void DoFoo(InputArrayType input,
                  OutputArrayType1 output1,
                  OutputArrayType2 output2)
{
  vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>::
      Copy(input, output1);
  vtkm::cont::DeviceAdapterAlgorithm<VTKM_DEFAULT_DEVICE_ADAPTER_TAG>::
      Copy(input, output2);
}

void Test()
{
  vtkm::cont::ArrayHandleCounting<vtkm::FloatDefault> inputArray(0, 10, 10);

  vtkm::cont::ArrayHandle<vtkm::FloatDefault> outputArray =
      DoBar(inputArray);

  VTKM_TEST_ASSERT(outputArray.GetNumberOfValues() == 10, "Wrong size.");
}

} // anonymous namespace

int ArrayHandleDiscard(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
