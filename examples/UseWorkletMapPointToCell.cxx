////
//// BEGIN-EXAMPLE UseWorkletMapPointToCell.cxx
////
#include <vtkm/worklet/DispatcherMapTopology.h>
#include <vtkm/worklet/WorkletMapTopology.h>

#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/DataSetFieldAdd.h>

#include <vtkm/exec/CellInterpolate.h>
#include <vtkm/exec/ParametricCoordinates.h>

//// PAUSE-EXAMPLE
namespace {
//// RESUME-EXAMPLE

class CellCenter : public vtkm::worklet::WorkletMapPointToCell
{
public:
  typedef void ControlSignature(TopologyIn cellSet,
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

VTKM_CONT_EXPORT
void FindCellCenters(vtkm::cont::DataSet &dataSet)
{
  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault,3> > cellCentersArray;

  vtkm::worklet::DispatcherMapTopology<CellCenter> dispatcher;
  dispatcher.Invoke(dataSet.GetCellSet(),
                    dataSet.GetCoordinateSystem().GetData(),
                    cellCentersArray);

  vtkm::cont::DataSetFieldAdd dataSetFieldAdd;
  dataSetFieldAdd.AddCellField(dataSet, "cell_center", cellCentersArray);
}
////
//// END-EXAMPLE UseWorkletMapPointToCell.cxx
////

} // anonymous namespace

#include <vtkm/cont/testing/MakeTestDataSet.h>
#include <vtkm/cont/testing/Testing.h>

namespace {

void Test()
{
  vtkm::cont::testing::MakeTestDataSet makeTestDataSet;

  std::cout << "Making test data set." << std::endl;
  vtkm::cont::DataSet dataSet = makeTestDataSet.Make3DUniformDataSet0();

  std::cout << "Finding cell centers." << std::endl;
  FindCellCenters(dataSet);

  std::cout << "Checking cell centers." << std::endl;
  vtkm::cont::CellSetStructured<3> cellSet;
  dataSet.GetCellSet().CopyTo(cellSet);

  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault,3> > cellCentersArray;
  dataSet.GetField("cell_center", vtkm::cont::Field::ASSOC_CELL_SET)
      .GetData().CopyTo(cellCentersArray);

  VTKM_TEST_ASSERT(
        cellSet.GetNumberOfCells() == cellCentersArray.GetNumberOfValues(),
        "Cell centers array has wrong number of values.");

  // TODO: Add a GetCellDimensions method to CellSetStructured.
  vtkm::Id3 cellDimensions = cellSet.GetPointDimensions() - vtkm::Id3(1);

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

} // anonymous namespace

int UseWorkletMapPointToCell(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
