#include <vtkm/filter/PointElevation.h>
#include <vtkm/filter/VertexClustering.h>

#include <vtkm/cont/testing/MakeTestDataSet.h>
#include <vtkm/cont/testing/Testing.h>

namespace {

////
//// BEGIN-EXAMPLE PointElevation.cxx
////
VTKM_CONT_EXPORT
vtkm::cont::DataSet ComputeAirPressure(vtkm::cont::DataSet dataSet)
{
  vtkm::filter::PointElevation elevationFilter;

  // Use the elevation filter to estimate atmospheric pressure based on the
  // height of the point coordinates. Atmospheric pressure is 101325 Pa at
  // sea level and drops about 12 Pa per meter.
  elevationFilter.SetOutputFieldName("pressure");
  elevationFilter.SetLowPoint(0.0, 0.0, 0.0);
  elevationFilter.SetHighPoint(0.0, 0.0, 2000.0);
  elevationFilter.SetRange(101325.0, 77325.0);

  vtkm::filter::ResultField result =
      elevationFilter.Execute(dataSet, dataSet.GetCoordinateSystem());

  if (!result.IsValid())
  {
    throw vtkm::cont::ErrorControlBadValue("Failed to run elevation filter.");
  }

  return result.GetDataSet();
}
////
//// END-EXAMPLE PointElevation.cxx
////

void DoPointElevation()
{
  vtkm::cont::testing::MakeTestDataSet makeData;
  vtkm::cont::DataSet inData = makeData.Make3DRegularDataSet0();

  vtkm::cont::DataSet pressureData = ComputeAirPressure(inData);

  pressureData.GetField("pressure").PrintSummary(std::cout);
  std::cout << std::endl;
}

void DoVertexClustering()
{
  vtkm::cont::testing::MakeTestDataSet makeData;
  vtkm::cont::DataSet originalSurface = makeData.Make3DExplicitDataSetCowNose();

  ////
  //// BEGIN-EXAMPLE VertexClustering.cxx
  ////
  vtkm::filter::VertexClustering vertexClustering;

  vertexClustering.SetNumberOfDivisions(vtkm::Id3(128,128,128));

  vtkm::filter::ResultDataSet result =
      vertexClustering.Execute(originalSurface);

  if (!result.IsValid())
  {
    throw vtkm::cont::ErrorControlBadValue("Failed to run vertex clustering.");
  }

  vtkm::cont::DataSet simplifiedSurface = result.GetDataSet();
  ////
  //// END-EXAMPLE VertexClustering.cxx
  ////

  simplifiedSurface.PrintSummary(std::cout);
  std::cout << std::endl;
}

void Test()
{
  DoPointElevation();
  DoVertexClustering();
}

}  // anonymous namespace

int ProvidedFilters(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
