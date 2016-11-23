////
//// BEGIN-EXAMPLE UseWorkletMapField.cxx
////
#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/WorkletMapField.h>

#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/DynamicArrayHandle.h>

#include <vtkm/VectorAnalysis.h>

namespace vtkm {
namespace worklet {

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
  VTKM_EXEC
  T operator()(const vtkm::Vec<T,Size> &inVector) const
  {
  //// END-EXAMPLE WorkletOperator.cxx
    return vtkm::Magnitude(inVector);
  }
};

}
} // namespace vtkm::worklet

VTKM_CONT
vtkm::cont::DynamicArrayHandle
InvokeMagnitude(vtkm::cont::DynamicArrayHandle input)
{
  vtkm::cont::ArrayHandle<vtkm::FloatDefault> output;

  vtkm::worklet::DispatcherMapField<vtkm::worklet::Magnitude> dispatcher;
  dispatcher.Invoke(input, output);

  return vtkm::cont::DynamicArrayHandle(output);
}
////
//// END-EXAMPLE UseWorkletMapField.cxx
////

#include <vtkm/filter/FilterField.h>

////
//// BEGIN-EXAMPLE UseFilterField.cxx
////
namespace vtkm {
namespace filter {

class FieldMagnitude : public vtkm::filter::FilterField<FieldMagnitude>
{
public:
  VTKM_CONT
  FieldMagnitude();

  template<typename ArrayHandleType, typename Policy, typename DeviceAdapter>
  VTKM_CONT
  vtkm::filter::ResultField
  DoExecute(const vtkm::cont::DataSet &inDataSet,
            const ArrayHandleType &inField,
            const vtkm::filter::FieldMetadata &fieldMetadata,
            vtkm::filter::PolicyBase<Policy>,
            DeviceAdapter);
};

template<>
class FilterTraits<vtkm::filter::FieldMagnitude>
{
public:
  struct InputFieldTypeList :
      vtkm::ListTagBase<vtkm::Vec<vtkm::Float32,2>,
                        vtkm::Vec<vtkm::Float64,2>,
                        vtkm::Vec<vtkm::Float32,3>,
                        vtkm::Vec<vtkm::Float64,3>,
                        vtkm::Vec<vtkm::Float32,4>,
                        vtkm::Vec<vtkm::Float64,4> >
  {  };
};

}
} // namespace vtkm::filter
////
//// END-EXAMPLE UseFilterField.cxx
////

////
//// BEGIN-EXAMPLE FilterFieldImpl.cxx
////
namespace vtkm {
namespace filter {

VTKM_CONT
FieldMagnitude::FieldMagnitude()
{
  this->SetOutputFieldName("");
}

template<typename ArrayHandleType, typename Policy, typename DeviceAdapter>
VTKM_CONT
vtkm::filter::ResultField
FieldMagnitude::DoExecute(const vtkm::cont::DataSet &inDataSet,
                          const ArrayHandleType &inField,
                          const vtkm::filter::FieldMetadata &fieldMetadata,
                          vtkm::filter::PolicyBase<Policy>,
                          DeviceAdapter)
{
  VTKM_IS_ARRAY_HANDLE(ArrayHandleType);
  VTKM_IS_DEVICE_ADAPTER_TAG(DeviceAdapter);

  using ComponentType = typename ArrayHandleType::ValueType::ComponentType;
  vtkm::cont::ArrayHandle<ComponentType> outField;

  vtkm::worklet::DispatcherMapField<vtkm::worklet::Magnitude, DeviceAdapter>
      dispatcher;
  dispatcher.Invoke(inField, outField);

  std::string outFieldName = this->GetOutputFieldName();
  if (outFieldName == "")
  {
    outFieldName = fieldMetadata.GetName() + "_magnitude";
  }

  return vtkm::filter::ResultField(inDataSet,
                                   outField,
                                   outFieldName,
                                   fieldMetadata.GetAssociation(),
                                   fieldMetadata.GetCellSetName());
}

}
} // namespace vtkm::filter
////
//// END-EXAMPLE FilterFieldImpl.cxx
////

////
//// BEGIN-EXAMPLE RandomArrayAccess.cxx
////
namespace vtkm {
namespace worklet {

struct ReverseArrayCopy : vtkm::worklet::WorkletMapField
{
  typedef void ControlSignature(FieldIn<> inputArray,
                                WholeArrayOut<> outputArray);
  typedef void ExecutionSignature(_1, _2, WorkIndex);
  typedef _1 InputDomain;

  template<typename InputType, typename OutputArrayPortalType>
  VTKM_EXEC
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

}
} // namespace vtkm::worklet

template<typename T, typename Storage>
VTKM_CONT
vtkm::cont::ArrayHandle<T>
InvokeReverseArrayCopy(const vtkm::cont::ArrayHandle<T,Storage> &inArray)
{
  vtkm::cont::ArrayHandle<T> outArray;
  outArray.Allocate(inArray.GetNumberOfValues());

  vtkm::worklet::DispatcherMapField<vtkm::worklet::ReverseArrayCopy> dispatcher;
  dispatcher.Invoke(inArray, outArray);

  return outArray;
}
////
//// END-EXAMPLE RandomArrayAccess.cxx
////

#include <vtkm/cont/DataSetFieldAdd.h>
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
  vtkm::cont::ArrayHandle<vtkm::FloatDefault> outputArray;
  outputDynamicArray.CopyTo(outputArray);

  VTKM_TEST_ASSERT(outputArray.GetNumberOfValues() == ARRAY_SIZE,
                   "Bad output array size.");
  for (vtkm::Id index = 0; index < ARRAY_SIZE; index++)
  {
    Vec3 testValue = TestValue(index, Vec3());
    vtkm::Float64 expectedValue = sqrt(vtkm::dot(testValue,testValue));
    vtkm::Float64 gotValue = outputArray.GetPortalConstControl().Get(index);
    VTKM_TEST_ASSERT(test_equal(expectedValue, gotValue), "Got bad value.");
  }
  outputArray.ReleaseResources();

  vtkm::cont::DataSet inputDataSet;
  vtkm::cont::CellSetStructured<1> cellSet("1D_mesh");
  cellSet.SetPointDimensions(ARRAY_SIZE);
  inputDataSet.AddCellSet(cellSet);
  vtkm::cont::DataSetFieldAdd::AddPointField(
        inputDataSet, "test_values", inputArray);

  vtkm::filter::FieldMagnitude fieldMagFilter;
  vtkm::filter::ResultField magResult =
      fieldMagFilter.Execute(inputDataSet, "test_values");
  magResult.FieldAs(outputArray);

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
