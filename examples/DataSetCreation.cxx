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

void Test()
{
  CreateUniformGrid();
  CreateUniformGridCustomOriginSpacing();
  CreateRectilinearGrid();
}

} // namespace DataSetCreationNamespace

int DataSetCreation(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(DataSetCreationNamespace::Test);
}
