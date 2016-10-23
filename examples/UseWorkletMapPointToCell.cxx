////
//// BEGIN-EXAMPLE UseWorkletMapPointToCell.cxx
////
#include <vtkm/worklet/DispatcherMapTopology.h>
#include <vtkm/worklet/WorkletMapTopology.h>

#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/DataSetFieldAdd.h>

#include <vtkm/exec/CellInterpolate.h>
#include <vtkm/exec/ParametricCoordinates.h>

namespace vtkm {
namespace worklet {

class CellCenter : public vtkm::worklet::WorkletMapPointToCell
{
public:
  typedef void ControlSignature(CellSetIn cellSet,
                                FieldInPoint<> inputPointField,
                                FieldOut<> outputCellField);
  typedef _3 ExecutionSignature(_1, PointCount, _2);

  typedef _1 InputDomain;

  template<typename CellShape,
           typename InputPointFieldType>
  VTKM_EXEC_EXPORT
  typename InputPointFieldType::ComponentType
  operator()(CellShape shape,
             vtkm::IdComponent numPoints,
             const InputPointFieldType &inputPointField) const
  {
    vtkm::Vec<vtkm::FloatDefault,3> parametricCenter =
        vtkm::exec::ParametricCoordinatesCenter(numPoints, shape, *this);
    return vtkm::exec::CellInterpolate(inputPointField,
                                       parametricCenter,
                                       shape,
                                       *this);
  }
};

}
} // namespace vtkm::worklet

VTKM_CONT_EXPORT
void FindCellCenters(vtkm::cont::DataSet &dataSet)
{
  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault,3> > cellCentersArray;

  vtkm::worklet::DispatcherMapTopology<vtkm::worklet::CellCenter> dispatcher;
  dispatcher.Invoke(dataSet.GetCellSet(),
                    dataSet.GetCoordinateSystem().GetData(),
                    cellCentersArray);

  vtkm::cont::DataSetFieldAdd dataSetFieldAdd;
  dataSetFieldAdd.AddCellField(dataSet, "cell_center", cellCentersArray);
}
////
//// END-EXAMPLE UseWorkletMapPointToCell.cxx
////

#include <vtkm/filter/FilterCell.h>

////
//// BEGIN-EXAMPLE UseFilterCell.cxx
////
namespace vtkm {
namespace filter {

class CellCenters : public vtkm::filter::FilterCell<CellCenters>
{
public:
  VTKM_CONT_EXPORT
  CellCenters();

  template<typename ArrayHandleType, typename Policy, typename DeviceAdapter>
  VTKM_CONT_EXPORT
  vtkm::filter::ResultField
  DoExecute(const vtkm::cont::DataSet &inDataSet,
            const ArrayHandleType &inField,
            const vtkm::filter::FieldMetadata &FieldMetadata,
            vtkm::filter::PolicyBase<Policy>,
            DeviceAdapter);
};

}
} // namespace vtkm::filter
////
//// END-EXAMPLE UseFilterCell.cxx
////

////
//// BEGIN-EXAMPLE FilterCellImpl.cxx
////
namespace vtkm {
namespace filter {

VTKM_CONT_EXPORT
CellCenters::CellCenters()
{
  this->SetOutputFieldName("");
}

template<typename ArrayHandleType, typename Policy, typename DeviceAdapter>
VTKM_CONT_EXPORT
vtkm::filter::ResultField
CellCenters::DoExecute(const vtkm::cont::DataSet &inDataSet,
                       const ArrayHandleType &inField,
                       const vtkm::filter::FieldMetadata &fieldMetadata,
                       vtkm::filter::PolicyBase<Policy>,
                       DeviceAdapter)
{
  VTKM_IS_ARRAY_HANDLE(ArrayHandleType);
  VTKM_IS_DEVICE_ADAPTER_TAG(DeviceAdapter);

  if (!fieldMetadata.IsPointField())
  {
    throw vtkm::cont::ErrorControlBadType(
          "Cell Centers filter operates on point data.");
  }

  vtkm::cont::DynamicCellSet cellSet =
      inDataSet.GetCellSet(this->GetActiveCellSetIndex());

  using ValueType = typename ArrayHandleType::ValueType;
  vtkm::cont::ArrayHandle<ValueType> outField;

  vtkm::worklet::DispatcherMapTopology<vtkm::worklet::CellCenter, DeviceAdapter>
      dispatcher;

  dispatcher.Invoke(vtkm::filter::ApplyPolicy(cellSet, Policy()),
                    inField,
                    outField);

  std::string outFieldName = this->GetOutputFieldName();
  if (outFieldName == "")
  {
    outFieldName = fieldMetadata.GetName() + "_center";
  }

  return vtkm::filter::ResultField(inDataSet,
                                   outField,
                                   outFieldName,
                                   vtkm::cont::Field::ASSOC_CELL_SET,
                                   cellSet.GetName());
}

}
} // namespace vtkm::filter
////
//// END-EXAMPLE FilterCellImpl.cxx
////

#include <vtkm/cont/testing/MakeTestDataSet.h>
#include <vtkm/cont/testing/Testing.h>

namespace {

void CheckCellCenters(const vtkm::cont::DataSet &dataSet)
{
  std::cout << "Checking cell centers." << std::endl;
  vtkm::cont::CellSetStructured<3> cellSet;
  dataSet.GetCellSet().CopyTo(cellSet);

  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault,3> > cellCentersArray;
  dataSet.GetField("cell_center", vtkm::cont::Field::ASSOC_CELL_SET)
      .GetData().CopyTo(cellCentersArray);

  VTKM_TEST_ASSERT(
        cellSet.GetNumberOfCells() == cellCentersArray.GetNumberOfValues(),
        "Cell centers array has wrong number of values.");

  vtkm::Id3 cellDimensions = cellSet.GetCellDimensions() - vtkm::Id3(1);

  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault,3> >::PortalConstControl
      cellCentersPortal = cellCentersArray.GetPortalConstControl();

  vtkm::Id cellIndex = 0;
  for (vtkm::Id kIndex = 0; kIndex < cellDimensions[2]; kIndex++)
  {
    for (vtkm::Id jIndex = 0; jIndex < cellDimensions[1]; jIndex++)
    {
      for (vtkm::Id iIndex = 0; iIndex < cellDimensions[0]; iIndex++)
      {
        vtkm::Vec<vtkm::FloatDefault,3> center =
            cellCentersPortal.Get(cellIndex);
        VTKM_TEST_ASSERT(test_equal(center[0], iIndex+0.5), "Bad X coord.");
        VTKM_TEST_ASSERT(test_equal(center[1], jIndex+0.5), "Bad Y coord.");
        VTKM_TEST_ASSERT(test_equal(center[2], kIndex+0.5), "Bad Z coord.");
        cellIndex++;
      }
    }
  }
}

void Test()
{
  vtkm::cont::testing::MakeTestDataSet makeTestDataSet;

  std::cout << "Making test data set." << std::endl;
  vtkm::cont::DataSet dataSet = makeTestDataSet.Make3DUniformDataSet0();

  std::cout << "Finding cell centers directly." << std::endl;
  FindCellCenters(dataSet);

  CheckCellCenters(dataSet);

  std::cout << "Making fresh test data set." << std::endl;
  dataSet = makeTestDataSet.Make3DUniformDataSet0();

  std::cout << "Finding cell centers with filter." << std::endl;
  vtkm::filter::CellCenters cellCentersFilter;
  cellCentersFilter.SetActiveCellSet(0);
  cellCentersFilter.SetOutputFieldName("cell_center");
  vtkm::filter::ResultField results =
      cellCentersFilter.Execute(dataSet, dataSet.GetCoordinateSystem());

  CheckCellCenters(results.GetDataSet());
}

} // anonymous namespace

int UseWorkletMapPointToCell(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
