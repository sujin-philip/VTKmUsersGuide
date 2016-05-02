#include <vtkm/filter/CellAverage.h>
#include <vtkm/filter/PointElevation.h>

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

  vtkm::filter::FieldResult fieldResult =
      elevationFilter.Execute(dataSet, dataSet.GetCoordinateSystem());

  if (!fieldResult.IsValid())
  {
    throw vtkm::cont::ErrorControlBadValue("Failed to run elevation filter.");
  }

  // In the near future, FieldResult should contain a data set that is a copy
  // of the input with the field attached with the appropriate association.
  // When that exists, we should just return that.
  vtkm::cont::DataSet result = dataSet;
  result.AddField(fieldResult.GetField());
  return result;
}
////
//// END-EXAMPLE PointElevation.cxx
////

void Test()
{
  vtkm::cont::testing::MakeTestDataSet makeData;
  vtkm::cont::DataSet inData = makeData.Make3DRegularDataSet0();

  vtkm::cont::DataSet pressureData = ComputeAirPressure(inData);
  pressureData.GetField("pressure");
}

}  // anonymous namespace

int ProvidedFilters(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
