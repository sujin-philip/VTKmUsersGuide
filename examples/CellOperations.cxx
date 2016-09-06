#include <vtkm/exec/CellDerivative.h>
#include <vtkm/exec/CellInterpolate.h>
#include <vtkm/exec/ParametricCoordinates.h>

#include <vtkm/worklet/DispatcherMapTopology.h>
#include <vtkm/worklet/DispatcherMapTopology.h>

#include <vtkm/cont/testing/MakeTestDataSet.h>
#include <vtkm/cont/testing/Testing.h>

namespace {

////
//// BEGIN-EXAMPLE CellCenters.cxx
////
struct CellCenters : vtkm::worklet::WorkletMapPointToCell
{
  typedef void ControlSignature(CellSetIn,
                                FieldInPoint<> inputField,
                                FieldOutCell<> outputField);
  typedef void ExecutionSignature(CellShape, PointCount, _2, _3);
  typedef _1 InputDomain;

  template<typename CellShapeTag,typename FieldInVecType,typename FieldOutType>
  VTKM_EXEC_EXPORT
  void operator()(CellShapeTag shape,
                  vtkm::IdComponent pointCount,
                  const FieldInVecType &inputField,
                  FieldOutType &outputField) const
  {
    vtkm::Vec<vtkm::FloatDefault,3> center =
        vtkm::exec::ParametricCoordinatesCenter(pointCount, shape, *this);
    outputField = vtkm::exec::CellInterpolate(inputField, center, shape, *this);
  }
};
////
//// END-EXAMPLE CellCenters.cxx
////

void TryCellCenters()
{
  std::cout << "Trying CellCenters worklet." << std::endl;

  vtkm::cont::DataSet dataSet =
      vtkm::cont::testing::MakeTestDataSet().Make3DUniformDataSet0();

  typedef vtkm::cont::ArrayHandle<vtkm::Float32> ArrayType;
  ArrayType centers;

  vtkm::worklet::DispatcherMapTopology<CellCenters> dispatcher;
  dispatcher.Invoke(dataSet.GetCellSet(),
                    dataSet.GetField("pointvar").GetData().Cast<ArrayType>(),
                    centers);

  vtkm::cont::printSummary_ArrayHandle(centers, std::cout);
  std::cout << std::endl;

  VTKM_TEST_ASSERT(centers.GetNumberOfValues() ==
                   dataSet.GetCellSet().GetNumberOfCells(),
                   "Bad number of cells.");
  VTKM_TEST_ASSERT(test_equal(60.1875, centers.GetPortalConstControl().Get(0)),
                   "Bad first value.");
}
////
//// BEGIN-EXAMPLE CellDerivatives.cxx
////
struct CellDerivatives : vtkm::worklet::WorkletMapPointToCell
{
  typedef void ControlSignature(CellSetIn,
                                FieldInPoint<> inputField,
                                FieldInPoint<Vec3> pointCoordinates,
                                FieldOutCell<> outputField);
  typedef void ExecutionSignature(CellShape, PointCount, _2, _3, _4);
  typedef _1 InputDomain;

  template<typename CellShapeTag,
           typename FieldInVecType,
           typename PointCoordVecType,
           typename FieldOutType>
  VTKM_EXEC_EXPORT
  void operator()(CellShapeTag shape,
                  vtkm::IdComponent pointCount,
                  const FieldInVecType &inputField,
                  const PointCoordVecType &pointCoordinates,
                  FieldOutType &outputField) const
  {
    vtkm::Vec<vtkm::FloatDefault,3> center =
        vtkm::exec::ParametricCoordinatesCenter(pointCount, shape, *this);
    outputField = vtkm::exec::CellDerivative(inputField,
                                             pointCoordinates,
                                             center,
                                             shape,
                                             *this);
  }
};
////
//// END-EXAMPLE CellDerivatives.cxx
////

void TryCellDerivatives()
{
  std::cout << "Trying CellDerivatives worklet." << std::endl;

  vtkm::cont::DataSet dataSet =
      vtkm::cont::testing::MakeTestDataSet().Make3DUniformDataSet0();

  typedef vtkm::cont::ArrayHandle<vtkm::Float32> ArrayType;
  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32,3> > derivatives;

  vtkm::worklet::DispatcherMapTopology<CellDerivatives> dispatcher;
  dispatcher.Invoke(dataSet.GetCellSet(),
                    dataSet.GetField("pointvar").GetData().Cast<ArrayType>(),
                    dataSet.GetCoordinateSystem().GetData(),
                    derivatives);

  vtkm::cont::printSummary_ArrayHandle(derivatives, std::cout);
  std::cout << std::endl;

  VTKM_TEST_ASSERT(derivatives.GetNumberOfValues() ==
                   dataSet.GetCellSet().GetNumberOfCells(),
                   "Bad number of cells.");
  VTKM_TEST_ASSERT(test_equal(vtkm::make_Vec(10.025,30.075,60.125),
                              derivatives.GetPortalConstControl().Get(0)),
                   "Bad first value.");
}

void Run()
{
  TryCellCenters();
  TryCellDerivatives();
}

} // anonymous namespace

int CellOperations(int, char*[])
{
  return vtkm::cont::testing::Testing::Run(Run);
}
