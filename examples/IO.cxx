////
//// BEGIN-EXAMPLE VTKDataSetWriter.cxx
////
#include <vtkm/io/writer/VTKDataSetWriter.h>

void SaveDataAsVTKFile(vtkm::cont::DataSet data)
{
  vtkm::io::writer::VTKDataSetWriter writer("data.vtk");

  writer.WriteDataSet(data);
}
////
//// END-EXAMPLE VTKDataSetWriter.cxx
////

////
//// BEGIN-EXAMPLE VTKDataSetReader.cxx
////
#include <vtkm/io/reader/VTKDataSetReader.h>

vtkm::cont::DataSet OpenDataFromVTKFile()
{
  vtkm::io::reader::VTKDataSetReader reader("data.vtk");

  return reader.ReadDataSet();
}
////
//// END-EXAMPLE VTKDataSetReader.cxx
////

#include <vtkm/cont/testing/MakeTestDataSet.h>
#include <vtkm/cont/testing/Testing.h>

namespace {

void TestIO()
{
  std::cout << "Writing data" << std::endl;
  vtkm::cont::testing::MakeTestDataSet makeDataSet;
  vtkm::cont::DataSet createdData = makeDataSet.Make3DExplicitDataSetCowNose();
  SaveDataAsVTKFile(createdData);

  std::cout << "Reading data" << std::endl;
  vtkm::cont::DataSet readData = OpenDataFromVTKFile();

  const vtkm::cont::CellSet &createdCellSet = createdData.GetCellSet().CastToBase();
  const vtkm::cont::CellSet &readCellSet = readData.GetCellSet().CastToBase();
  VTKM_TEST_ASSERT(
        createdCellSet.GetNumberOfCells() == readCellSet.GetNumberOfCells(),
        "Createded and read data do not match.");
  VTKM_TEST_ASSERT(
        createdCellSet.GetNumberOfPoints() == readCellSet.GetNumberOfPoints(),
        "Createded and read data do not match.");
}

}

int IO(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(TestIO);
}
