////
//// BEGIN-EXAMPLE UseWorkletMapCellToPoint.cxx
////
#include <vtkm/worklet/DispatcherMapTopology.h>
#include <vtkm/worklet/WorkletMapTopology.h>

#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/DataSetFieldAdd.h>
#include <vtkm/cont/DynamicArrayHandle.h>
#include <vtkm/cont/DynamicCellSet.h>
#include <vtkm/cont/Field.h>

namespace vtkm {
namespace worklet {

class AverageCellField : public vtkm::worklet::WorkletMapCellToPoint
{
public:
  typedef void ControlSignature(CellSetIn cellSet,
                                FieldInCell<> inputCellField,
                                FieldOut<> outputPointField);
  typedef void ExecutionSignature(CellCount, _2, _3);

  typedef _1 InputDomain;

  template<typename InputCellFieldType, typename OutputFieldType>
  VTKM_EXEC
  void
  operator()(vtkm::IdComponent numCells,
             const InputCellFieldType &inputCellField,
             OutputFieldType &fieldAverage) const
  {
    // TODO: This trickery with calling DoAverage with an extra fabricated type
    // is to handle when the dynamic type resolution provides combinations that
    // are incompatible. On the todo list for VTK-m is to allow you to express
    // types that are the same for different parameters of the control
    // signature. When that happens, we can get rid of this hack.
    typedef typename InputCellFieldType::ComponentType InputComponentType;
    this->DoAverage(numCells,
                    inputCellField,
                    fieldAverage,
                    vtkm::ListTagBase<InputComponentType,OutputFieldType>());
  }

private:
  template<typename InputCellFieldType, typename OutputFieldType>
  VTKM_EXEC
  void DoAverage(vtkm::IdComponent numCells,
                 const InputCellFieldType &inputCellField,
                 OutputFieldType &fieldAverage,
                 vtkm::ListTagBase<OutputFieldType,OutputFieldType>) const
  {
    fieldAverage = OutputFieldType(0);

    for (vtkm::IdComponent cellIndex = 0; cellIndex < numCells; cellIndex++)
    {
      fieldAverage = fieldAverage + inputCellField[cellIndex];
    }

    //// PAUSE-EXAMPLE
    // The following line can create a warning when converting numCells to a
    // float. However, casting it is tricky since OutputFieldType could be
    // a vector, and that would unnecessarily complicate this example. Instead,
    // just suppress the warning.
#ifdef VTKM_MSVC
#pragma warning(push)
#pragma warning(disable:4244)
#endif
    //// RESUME-EXAMPLE
    fieldAverage = fieldAverage / OutputFieldType(numCells);
    //// PAUSE-EXAMPLE
#ifdef VTKM_MSVC
#pragma warning(pop)
#endif
    //// RESUME-EXAMPLE
  }

  template<typename T1, typename T2, typename T3>
  VTKM_EXEC
  void DoAverage(vtkm::IdComponent, T1, T2, T3) const
  {
    this->RaiseError("Incompatible types for input and output.");
  }
};

}
} // namespace vtkm::worklet

VTKM_CONT
vtkm::cont::DataSet
ConvertCellFieldsToPointFields(const vtkm::cont::DataSet &inData)
{
  vtkm::cont::DataSet outData;

  // Copy parts of structure that should be passed through.
  for (vtkm::Id cellSetIndex = 0;
       cellSetIndex < inData.GetNumberOfCellSets();
       cellSetIndex++)
  {
    outData.AddCellSet(inData.GetCellSet(cellSetIndex));
  }
  for (vtkm::Id coordSysIndex = 0;
       coordSysIndex < inData.GetNumberOfCoordinateSystems();
       coordSysIndex++)
  {
    outData.AddCoordinateSystem(inData.GetCoordinateSystem(coordSysIndex));
  }

  // Copy all fields, converting cell fields to point fields.
  for (vtkm::Id fieldIndex = 0;
       fieldIndex < inData.GetNumberOfFields();
       fieldIndex++)
  {
    vtkm::cont::Field inField = inData.GetField(fieldIndex);
    if (inField.GetAssociation() == vtkm::cont::Field::ASSOC_CELL_SET)
    {
      vtkm::cont::DynamicArrayHandle inFieldData = inField.GetData();
      vtkm::cont::DynamicCellSet inCellSet =
          inData.GetCellSet(inField.GetAssocCellSet());

      vtkm::cont::DynamicArrayHandle outFieldData = inFieldData.NewInstance();
      vtkm::worklet::DispatcherMapTopology<vtkm::worklet::AverageCellField>
          dispatcher;
      dispatcher.Invoke(inCellSet, inFieldData, outFieldData);

      vtkm::cont::DataSetFieldAdd::AddCellField(outData,
                                                inField.GetName(),
                                                outFieldData,
                                                inField.GetAssocCellSet());
    }
    else
    {
      outData.AddField(inField);
    }
  }

  return outData;
}
////
//// END-EXAMPLE UseWorkletMapCellToPoint.cxx
////

#include <vtkm/cont/testing/MakeTestDataSet.h>
#include <vtkm/cont/testing/Testing.h>

namespace {

void Test()
{
  vtkm::cont::testing::MakeTestDataSet makeTestDataSet;

  std::cout << "Making test data set." << std::endl;
  vtkm::cont::DataSet inDataSet = makeTestDataSet.Make3DUniformDataSet0();

  std::cout << "Average cell data." << std::endl;
  vtkm::cont::DataSet resultDataSet = ConvertCellFieldsToPointFields(inDataSet);

  std::cout << "Checking cell data converted to points." << std::endl;
  vtkm::cont::Field convertedField = resultDataSet.GetField("cellvar");
  VTKM_TEST_ASSERT(
        convertedField.GetAssociation() == vtkm::cont::Field::ASSOC_CELL_SET,
        "Result field has wrong association.");

  const vtkm::Id numPoints = 18;
  vtkm::Float64 expectedData[numPoints] = {
    100.1, 100.15, 100.2, 100.1, 100.15, 100.2,
    100.2, 100.25, 100.3, 100.2, 100.25, 100.3,
    100.3, 100.35, 100.4, 100.3, 100.35, 100.4
  };

  vtkm::cont::ArrayHandle<vtkm::Float32> outData;
  convertedField.GetData().CopyTo(outData);
  vtkm::cont::ArrayHandle<vtkm::Float32>::PortalConstControl outPortal =
      outData.GetPortalConstControl();
  vtkm::cont::printSummary_ArrayHandle(outData, std::cout);
  std::cout << std::endl;
  VTKM_TEST_ASSERT(outPortal.GetNumberOfValues() == numPoints,
                   "Result array wrong size.");

  for (vtkm::Id pointId = 0; pointId < numPoints; pointId++)
  {
    VTKM_TEST_ASSERT(test_equal(outPortal.Get(pointId), expectedData[pointId]),
                     "Got wrong result.");
  }
}

} // anonymous namespace

int UseWorkletMapCellToPoint(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
