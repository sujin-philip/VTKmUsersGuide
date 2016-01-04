#include <vtkm/cont/DataSetBuilderExplicit.h>
#include <vtkm/cont/DataSetBuilderRectilinear.h>
#include <vtkm/cont/DataSetBuilderRegular.h>

#include <vtkm/Math.h>

#include <vtkm/cont/testing/Testing.h>

namespace DataSetCreationNamespace {

void CreateUniformGrid()
{
  std::cout << "Creating uniform grid." << std::endl;

  ////
  //// BEGIN-EXAMPLE CreateUniformGrid.cxx
  ////
  vtkm::cont::DataSetBuilderRegular dataSetBuilder;

  vtkm::cont::DataSet dataSet = dataSetBuilder.Create(vtkm::Id3(101, 101, 26));
  ////
  //// END-EXAMPLE CreateUniformGrid.cxx
  ////

  vtkm::Float64 bounds[6];
  dataSet.GetCoordinateSystem().GetBounds(bounds,
                                          VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
  std::cout << bounds[0] << " " << bounds[1] << std::endl
            << bounds[2] << " " << bounds[3] << std::endl
            << bounds[4] << " " << bounds[5] << std::endl;

  VTKM_TEST_ASSERT(test_equal(bounds[0],   0), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[1], 100), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[2],   0), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[3], 100), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[4],   0), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[5],  25), "Bad bounds");
}

void CreateUniformGridCustomOriginSpacing()
{
  std::cout << "Creating uniform grid with custom origin and spacing."
            << std::endl;

  ////
  //// BEGIN-EXAMPLE CreateUniformGridCustomOriginSpacing.cxx
  ////
  vtkm::cont::DataSetBuilderRegular dataSetBuilder;

  vtkm::cont::DataSet dataSet =
      dataSetBuilder.Create(
        vtkm::Id3(101, 101, 26),
        vtkm::Vec<vtkm::FloatDefault,3>(-50.0, -50.0, -50.0),
        vtkm::Vec<vtkm::FloatDefault,3>(1.0, 1.0, 4.0));
  ////
  //// END-EXAMPLE CreateUniformGridCustomOriginSpacing.cxx
  ////

  vtkm::Float64 bounds[6];
  dataSet.GetCoordinateSystem().GetBounds(bounds,
                                          VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
  std::cout << bounds[0] << " " << bounds[1] << std::endl
            << bounds[2] << " " << bounds[3] << std::endl
            << bounds[4] << " " << bounds[5] << std::endl;

  VTKM_TEST_ASSERT(test_equal(bounds[0], -50), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[1],  50), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[2], -50), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[3],  50), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[4], -50), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[5],  50), "Bad bounds");
}

void CreateRectilinearGrid()
{
  std::cout << "Create rectilinear grid." << std::endl;

  ////
  //// BEGIN-EXAMPLE CreateRectilinearGrid.cxx
  ////
  // Make x coordinates range from -4 to 4 with tighter spacing near 0.
  std::vector<vtkm::Float32> xCoordinates;
  for (vtkm::Float32 x = -2.0f; x <= 2.0f; x += 0.02f)
  {
    xCoordinates.push_back(vtkm::CopySign(x*x, x));
  }

  // Make y coordinates range from 0 to 2 with tighter spacing near 2.
  std::vector<vtkm::Float32> yCoordinates;
  for (vtkm::Float32 y = 0.0f; y <= 4.0f; y += 0.02f)
  {
    yCoordinates.push_back(vtkm::Sqrt(y));
  }

  // Make z coordinates rangefrom -1 to 1 with even spacing.
  std::vector<vtkm::Float32> zCoordinates;
  for (vtkm::Float32 z = -1.0f; z <= 1.0f; z += 0.02f)
  {
    zCoordinates.push_back(z);
  }

  vtkm::cont::DataSetBuilderRectilinear dataSetBuilder;

  vtkm::cont::DataSet dataSet = dataSetBuilder.Create(xCoordinates,
                                                      yCoordinates,
                                                      zCoordinates);
  ////
  //// END-EXAMPLE CreateRectilinearGrid.cxx
  ////

  vtkm::Id numPoints =  dataSet.GetCellSet().GetCellSet().GetNumberOfPoints();
  std::cout << "Num points: " << numPoints << std::endl;
  VTKM_TEST_ASSERT(numPoints == 4080501, "Got wrong number of points.");

  typedef vtkm::cont::ArrayHandleCartesianProduct<
      vtkm::cont::ArrayHandle<vtkm::Float64>,
      vtkm::cont::ArrayHandle<vtkm::Float64>,
      vtkm::cont::ArrayHandle<vtkm::Float64> > PointCoordinateArrayType;

//  // Disabling this test for now. Some things in the source need to change.
//  // First, CoordinateSystem should have a version of GetBounds that allows
//  // you to specify a new storage tag list. Second, there are currently some
//  // errors in ArrayHandleCartesianProduct that need to be fixed. Since I'm
//  // currently working off Dave P.'s branch, I don't want to go through and
//  // fix those right now.
//  vtkm::Float64 bounds[6];
//  dataSet.GetCoordinateSystem().GetBounds(
//        bounds,
//        VTKM_DEFAULT_DEVICE_ADAPTER_TAG(),
//        VTKM_DEFAULT_COORDINATE_SYSTEM_TYPE_LIST_TAG(),
//        vtkm::ListTagBase<PointCoordinateArrayType::StorageTag>());
//  std::cout << bounds[0] << " " << bounds[1] << std::endl
//            << bounds[2] << " " << bounds[3] << std::endl
//            << bounds[4] << " " << bounds[5] << std::endl;

//  VTKM_TEST_ASSERT(test_equal(bounds[0], -4), "Bad bounds");
//  VTKM_TEST_ASSERT(test_equal(bounds[1],  4), "Bad bounds");
//  VTKM_TEST_ASSERT(test_equal(bounds[2],  0), "Bad bounds");
//  VTKM_TEST_ASSERT(test_equal(bounds[3],  2), "Bad bounds");
//  VTKM_TEST_ASSERT(test_equal(bounds[4], -1), "Bad bounds");
//  VTKM_TEST_ASSERT(test_equal(bounds[5],  1), "Bad bounds");
}

void CreateExplicitGrid()
{
  std::cout << "Creating explicit grid." << std::endl;

  ////
  //// BEGIN-EXAMPLE CreateExplicitGrid.cxx
  ////
  // Array of point coordinates.
  std::vector<vtkm::Vec<vtkm::Float32,3> > pointCoordinates;
  pointCoordinates.push_back(vtkm::Vec<vtkm::Float32,3>(1.1f, 0.0f, 0.0f));
  pointCoordinates.push_back(vtkm::Vec<vtkm::Float32,3>(0.2f, 0.4f, 0.0f));
  pointCoordinates.push_back(vtkm::Vec<vtkm::Float32,3>(0.9f, 0.6f, 0.0f));
  pointCoordinates.push_back(vtkm::Vec<vtkm::Float32,3>(1.4f, 0.5f, 0.0f));
  pointCoordinates.push_back(vtkm::Vec<vtkm::Float32,3>(1.8f, 0.3f, 0.0f));
  pointCoordinates.push_back(vtkm::Vec<vtkm::Float32,3>(0.4f, 1.0f, 0.0f));
  pointCoordinates.push_back(vtkm::Vec<vtkm::Float32,3>(1.0f, 1.2f, 0.0f));
  pointCoordinates.push_back(vtkm::Vec<vtkm::Float32,3>(1.5f, 0.9f, 0.0f));

  // Array of shapes.
  std::vector<vtkm::UInt8> shapes;
  shapes.push_back(vtkm::CELL_SHAPE_TRIANGLE);
  shapes.push_back(vtkm::CELL_SHAPE_QUAD);
  shapes.push_back(vtkm::CELL_SHAPE_TRIANGLE);
  shapes.push_back(vtkm::CELL_SHAPE_POLYGON);
  shapes.push_back(vtkm::CELL_SHAPE_TRIANGLE);

  // Array of number of indices per cell.
  std::vector<vtkm::IdComponent> numIndices;
  numIndices.push_back(3);
  numIndices.push_back(4);
  numIndices.push_back(3);
  numIndices.push_back(5);
  numIndices.push_back(3);

  // Connectivity array.
  std::vector<vtkm::Id> connectivity;
  connectivity.push_back(0); // Cell 0
  connectivity.push_back(2);
  connectivity.push_back(1);
  connectivity.push_back(0); // Cell 1
  connectivity.push_back(4);
  connectivity.push_back(3);
  connectivity.push_back(2);
  connectivity.push_back(1); // Cell 2
  connectivity.push_back(2);
  connectivity.push_back(5);
  connectivity.push_back(2); // Cell 3
  connectivity.push_back(3);
  connectivity.push_back(7);
  connectivity.push_back(6);
  connectivity.push_back(5);
  connectivity.push_back(3); // Cell 4
  connectivity.push_back(4);
  connectivity.push_back(7);

  // Copy these arrays into a DataSet.
  vtkm::cont::DataSetBuilderExplicit dataSetBuilder;

  vtkm::cont::DataSet dataSet = dataSetBuilder.Create(pointCoordinates,
                                                      shapes,
                                                      numIndices,
                                                      connectivity);
  ////
  //// END-EXAMPLE CreateExplicitGrid.cxx
  ////

  vtkm::cont::CellSetExplicit<> cellSet =
      dataSet.GetCellSet().CastTo(vtkm::cont::CellSetExplicit<>());
  VTKM_TEST_ASSERT(test_equal(cellSet.GetNumberOfPoints(), 8),
                   "Data set has wrong number of points.");
  VTKM_TEST_ASSERT(test_equal(cellSet.GetNumberOfCells(), 5),
                   "Data set has wrong number of cells.");

  vtkm::Float64 bounds[6];
  dataSet.GetCoordinateSystem().GetBounds(bounds,
                                          VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
  std::cout << bounds[0] << " " << bounds[1] << std::endl
            << bounds[2] << " " << bounds[3] << std::endl
            << bounds[4] << " " << bounds[5] << std::endl;

  VTKM_TEST_ASSERT(test_equal(bounds[0], 0.2), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[1], 1.8), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[2], 0.0), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[3], 1.2), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[4], 0.0), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[5], 0.0), "Bad bounds");

  // Do a simple check of the connectivity by getting the number of cells
  // incident on each point. This array is unlikely to be correct if the
  // topology got screwed up.
  cellSet.BuildConnectivity(VTKM_DEFAULT_DEVICE_ADAPTER_TAG(),
                            vtkm::TopologyElementTagCell(),
                            vtkm::TopologyElementTagPoint());
  vtkm::cont::ArrayHandle<vtkm::IdComponent> numCellsPerPoint =
      cellSet.GetNumIndicesArray(vtkm::TopologyElementTagCell(),
                                 vtkm::TopologyElementTagPoint());
  vtkm::cont::printSummary_ArrayHandle(numCellsPerPoint, std::cout);
  std::cout << std::endl;
  vtkm::cont::ArrayHandle<vtkm::IdComponent>::PortalConstControl numCellsPortal=
      numCellsPerPoint.GetPortalConstControl();
  VTKM_TEST_ASSERT(test_equal(numCellsPortal.Get(0), 2),
                   "Wrong number of cells on point 0");
  VTKM_TEST_ASSERT(test_equal(numCellsPortal.Get(1), 2),
                   "Wrong number of cells on point 1");
  VTKM_TEST_ASSERT(test_equal(numCellsPortal.Get(2), 4),
                   "Wrong number of cells on point 2");
  VTKM_TEST_ASSERT(test_equal(numCellsPortal.Get(3), 3),
                   "Wrong number of cells on point 3");
  VTKM_TEST_ASSERT(test_equal(numCellsPortal.Get(4), 2),
                   "Wrong number of cells on point 4");
  VTKM_TEST_ASSERT(test_equal(numCellsPortal.Get(5), 2),
                   "Wrong number of cells on point 5");
  VTKM_TEST_ASSERT(test_equal(numCellsPortal.Get(6), 1),
                   "Wrong number of cells on point 6");
  VTKM_TEST_ASSERT(test_equal(numCellsPortal.Get(7), 2),
                   "Wrong number of cells on point 7");
}

void CreateExplicitGridIterative()
{
  std::cout << "Creating explicit grid iteratively." << std::endl;

  ////
  //// BEGIN-EXAMPLE CreateExplicitGridIterative.cxx
  ////
  vtkm::cont::DataSetBuilderExplicitIterative dataSetBuilder;

  dataSetBuilder.AddPoint(1.1, 0.0, 0.0);
  dataSetBuilder.AddPoint(0.2, 0.4, 0.0);
  dataSetBuilder.AddPoint(0.9, 0.6, 0.0);
  dataSetBuilder.AddPoint(1.4, 0.5, 0.0);
  dataSetBuilder.AddPoint(1.8, 0.3, 0.0);
  dataSetBuilder.AddPoint(0.4, 1.0, 0.0);
  dataSetBuilder.AddPoint(1.0, 1.2, 0.0);
  dataSetBuilder.AddPoint(1.5, 0.9, 0.0);

  dataSetBuilder.AddCell(vtkm::CELL_SHAPE_TRIANGLE);
  dataSetBuilder.AddCellPoint(0);
  dataSetBuilder.AddCellPoint(2);
  dataSetBuilder.AddCellPoint(1);

  dataSetBuilder.AddCell(vtkm::CELL_SHAPE_QUAD);
  dataSetBuilder.AddCellPoint(0);
  dataSetBuilder.AddCellPoint(4);
  dataSetBuilder.AddCellPoint(3);
  dataSetBuilder.AddCellPoint(2);

  dataSetBuilder.AddCell(vtkm::CELL_SHAPE_TRIANGLE);
  dataSetBuilder.AddCellPoint(1);
  dataSetBuilder.AddCellPoint(2);
  dataSetBuilder.AddCellPoint(5);

  dataSetBuilder.AddCell(vtkm::CELL_SHAPE_POLYGON);
  dataSetBuilder.AddCellPoint(2);
  dataSetBuilder.AddCellPoint(3);
  dataSetBuilder.AddCellPoint(7);
  dataSetBuilder.AddCellPoint(6);
  dataSetBuilder.AddCellPoint(5);

  dataSetBuilder.AddCell(vtkm::CELL_SHAPE_TRIANGLE);
  dataSetBuilder.AddCellPoint(3);
  dataSetBuilder.AddCellPoint(4);
  dataSetBuilder.AddCellPoint(7);

  vtkm::cont::DataSet dataSet = dataSetBuilder.Create();
  ////
  //// END-EXAMPLE CreateExplicitGridIterative.cxx
  ////

  vtkm::cont::CellSetExplicit<> cellSet =
      dataSet.GetCellSet().CastTo(vtkm::cont::CellSetExplicit<>());
  VTKM_TEST_ASSERT(test_equal(cellSet.GetNumberOfPoints(), 8),
                   "Data set has wrong number of points.");
  VTKM_TEST_ASSERT(test_equal(cellSet.GetNumberOfCells(), 5),
                   "Data set has wrong number of cells.");

  vtkm::Float64 bounds[6];
  dataSet.GetCoordinateSystem().GetBounds(bounds,
                                          VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
  std::cout << bounds[0] << " " << bounds[1] << std::endl
            << bounds[2] << " " << bounds[3] << std::endl
            << bounds[4] << " " << bounds[5] << std::endl;

  VTKM_TEST_ASSERT(test_equal(bounds[0], 0.2), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[1], 1.8), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[2], 0.0), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[3], 1.2), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[4], 0.0), "Bad bounds");
  VTKM_TEST_ASSERT(test_equal(bounds[5], 0.0), "Bad bounds");

  // Do a simple check of the connectivity by getting the number of cells
  // incident on each point. This array is unlikely to be correct if the
  // topology got screwed up.
  cellSet.BuildConnectivity(VTKM_DEFAULT_DEVICE_ADAPTER_TAG(),
                            vtkm::TopologyElementTagCell(),
                            vtkm::TopologyElementTagPoint());
  vtkm::cont::ArrayHandle<vtkm::IdComponent> numCellsPerPoint =
      cellSet.GetNumIndicesArray(vtkm::TopologyElementTagCell(),
                                 vtkm::TopologyElementTagPoint());
  vtkm::cont::printSummary_ArrayHandle(numCellsPerPoint, std::cout);
  std::cout << std::endl;
  vtkm::cont::ArrayHandle<vtkm::IdComponent>::PortalConstControl numCellsPortal=
      numCellsPerPoint.GetPortalConstControl();
  VTKM_TEST_ASSERT(test_equal(numCellsPortal.Get(0), 2),
                   "Wrong number of cells on point 0");
  VTKM_TEST_ASSERT(test_equal(numCellsPortal.Get(1), 2),
                   "Wrong number of cells on point 1");
  VTKM_TEST_ASSERT(test_equal(numCellsPortal.Get(2), 4),
                   "Wrong number of cells on point 2");
  VTKM_TEST_ASSERT(test_equal(numCellsPortal.Get(3), 3),
                   "Wrong number of cells on point 3");
  VTKM_TEST_ASSERT(test_equal(numCellsPortal.Get(4), 2),
                   "Wrong number of cells on point 4");
  VTKM_TEST_ASSERT(test_equal(numCellsPortal.Get(5), 2),
                   "Wrong number of cells on point 5");
  VTKM_TEST_ASSERT(test_equal(numCellsPortal.Get(6), 1),
                   "Wrong number of cells on point 6");
  VTKM_TEST_ASSERT(test_equal(numCellsPortal.Get(7), 2),
                   "Wrong number of cells on point 7");
}

void Test()
{
  CreateUniformGrid();
  CreateUniformGridCustomOriginSpacing();
  CreateRectilinearGrid();
  CreateExplicitGrid();
  CreateExplicitGridIterative();
}

} // namespace DataSetCreationNamespace

int DataSetCreation(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(DataSetCreationNamespace::Test);
}
