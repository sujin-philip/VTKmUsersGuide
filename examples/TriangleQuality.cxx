////
//// BEGIN-EXAMPLE TriangleQualityWholeArray.cxx
////
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/DataSet.h>

#include <vtkm/worklet/DispatcherMapTopology.h>
#include <vtkm/worklet/WorkletMapTopology.h>

#include <vtkm/CellShape.h>
#include <vtkm/Math.h>
#include <vtkm/VectorAnalysis.h>
//// PAUSE-EXAMPLE
namespace TriangleQualityNamespace {
//// RESUME-EXAMPLE

static const vtkm::Id TRIANGLE_QUALITY_TABLE_DIMENSION = 8;
static const vtkm::Id TRIANGLE_QUALITY_TABLE_SIZE =
    TRIANGLE_QUALITY_TABLE_DIMENSION*TRIANGLE_QUALITY_TABLE_DIMENSION;

VTKM_CONT
vtkm::cont::ArrayHandle<vtkm::Float32> GetTriangleQualityTable()
{
  // Use these precomputed values for the array. A real application would
  // probably use a larger array, but we are keeping it small for demonstration
  // purposes.
  static vtkm::Float32 triangleQualityBuffer[TRIANGLE_QUALITY_TABLE_SIZE] = {
    0, 0,        0,        0,        0,        0,        0,        0,
    0, 0,        0,        0,        0,        0,        0,        0.24431f,
    0, 0,        0,        0,        0,        0,        0.43298f, 0.47059f,
    0, 0,        0,        0,        0,        0.54217f, 0.65923f, 0.66408f,
    0, 0,        0,        0,        0.57972f, 0.75425f, 0.82154f, 0.81536f,
    0, 0,        0,        0.54217f, 0.75425f, 0.87460f, 0.92567f, 0.92071f,
    0, 0,        0.43298f, 0.65923f, 0.82154f, 0.92567f, 0.97664f, 0.98100f,
    0, 0.24431f, 0.47059f, 0.66408f, 0.81536f, 0.92071f, 0.98100f, 1
  };

  return vtkm::cont::make_ArrayHandle(triangleQualityBuffer,
                                      TRIANGLE_QUALITY_TABLE_SIZE);
}

template<typename T>
VTKM_EXEC_CONT
vtkm::Vec<T,3> TriangleEdgeLengths(const vtkm::Vec<T,3> &point1,
                                   const vtkm::Vec<T,3> &point2,
                                   const vtkm::Vec<T,3> &point3)
{
  return vtkm::make_Vec(vtkm::Magnitude(point1-point2),
                        vtkm::Magnitude(point2-point3),
                        vtkm::Magnitude(point3-point1));
}

VTKM_SUPPRESS_EXEC_WARNINGS
template<typename PortalType, typename T>
VTKM_EXEC_CONT
vtkm::Float32 LookupTriangleQuality(const PortalType &triangleQualityPortal,
                                    const vtkm::Vec<T,3> &point1,
                                    const vtkm::Vec<T,3> &point2,
                                    const vtkm::Vec<T,3> &point3)
{
  vtkm::Vec<T,3> edgeLengths = TriangleEdgeLengths(point1, point2, point3);

  // To reduce the size of the table, we just store the quality of triangles
  // with the longest edge of size 1. The table is 2D indexed by the length
  // of the other two edges. Thus, to use the table we have to identify the
  // longest edge and scale appropriately.
  T smallEdge1 = vtkm::Min(edgeLengths[0], edgeLengths[1]);
  T tmpEdge = vtkm::Max(edgeLengths[0], edgeLengths[1]);
  T smallEdge2 = vtkm::Min(edgeLengths[2], tmpEdge);
  T largeEdge = vtkm::Max(edgeLengths[2], tmpEdge);

  smallEdge1 /= largeEdge;
  smallEdge2 /= largeEdge;

  // Find index into array.
  vtkm::Id index1 = static_cast<vtkm::Id>(
        vtkm::Floor(smallEdge1*(TRIANGLE_QUALITY_TABLE_DIMENSION-1)+0.5));
  vtkm::Id index2 = static_cast<vtkm::Id>(
        vtkm::Floor(smallEdge2*(TRIANGLE_QUALITY_TABLE_DIMENSION-1)+0.5));
  vtkm::Id totalIndex = index1 + index2*TRIANGLE_QUALITY_TABLE_DIMENSION;

  return triangleQualityPortal.Get(totalIndex);
}

struct TriangleQualityWorklet : vtkm::worklet::WorkletMapPointToCell
{
  typedef void ControlSignature(CellSetIn cells,
                                FieldInPoint<Vec3> pointCoordinates,
                                WholeArrayIn<Scalar> triangleQualityTable,
                                FieldOutCell<Scalar> triangleQuality);
  typedef _4 ExecutionSignature(CellShape, _2, _3);
  typedef _1 InputDomain;

  template<typename CellShape,
           typename PointCoordinatesType,
           typename TriangleQualityTablePortalType>
  VTKM_EXEC
  vtkm::Float32 operator()(
      CellShape shape,
      const PointCoordinatesType &pointCoordinates,
      const TriangleQualityTablePortalType &triangleQualityTable) const
  {
    if (shape.Id != vtkm::CELL_SHAPE_TRIANGLE)
    {
      this->RaiseError("Only triangles are supported for triangle quality.");
      return vtkm::Nan32();
    }

    return LookupTriangleQuality(triangleQualityTable,
                                 pointCoordinates[0],
                                 pointCoordinates[1],
                                 pointCoordinates[2]);
  }
};

// Normally we would encapsulate this call in a filter, but for demonstrative
// purposes we are just calling the worklet directly.
template<typename DeviceAdapterTag>
VTKM_CONT
vtkm::cont::ArrayHandle<vtkm::Float32>
RunTriangleQuality(vtkm::cont::DataSet dataSet,
                   DeviceAdapterTag)
{
  vtkm::cont::ArrayHandle<vtkm::Float32> triangleQualityTable =
      GetTriangleQualityTable();

  vtkm::cont::ArrayHandle<vtkm::Float32> triangleQualities;

  vtkm::worklet::DispatcherMapTopology<TriangleQualityWorklet,DeviceAdapterTag>
      dispatcher;
  dispatcher.Invoke(dataSet.GetCellSet(),
                    dataSet.GetCoordinateSystem().GetData(),
                    triangleQualityTable,
                    triangleQualities);

  return triangleQualities;
}
////
//// END-EXAMPLE TriangleQualityWholeArray.cxx
////

////
//// BEGIN-EXAMPLE TriangleQualityExecObject.cxx
////
template<typename DeviceAdapterTag>
class TriangleQualityTable : public vtkm::exec::ExecutionObjectBase
{
public:
  VTKM_CONT
  TriangleQualityTable()
  {
    this->TablePortal =
        GetTriangleQualityTable().PrepareForInput(DeviceAdapterTag());
  }

  template<typename T>
  VTKM_EXEC
  vtkm::Float32 GetQuality(const vtkm::Vec<T,3> &point1,
                           const vtkm::Vec<T,3> &point2,
                           const vtkm::Vec<T,3> &point3) const
  {
    return LookupTriangleQuality(this->TablePortal, point1, point2, point3);
  }

private:
  typedef vtkm::cont::ArrayHandle<vtkm::Float32> TableArrayType;
  typedef typename TableArrayType::ExecutionTypes<DeviceAdapterTag>::PortalConst
      TableArrayPortalType;
  TableArrayPortalType TablePortal;
};

struct TriangleQualityWorklet2 : vtkm::worklet::WorkletMapPointToCell
{
  typedef void ControlSignature(CellSetIn cells,
                                FieldInPoint<Vec3> pointCoordinates,
                                ExecObject triangleQualityTable,
                                FieldOutCell<Scalar> triangleQuality);
  typedef _4 ExecutionSignature(CellShape, _2, _3);
  typedef _1 InputDomain;

  template<typename CellShape,
           typename PointCoordinatesType,
           typename TriangleQualityTableType>
  VTKM_EXEC
  vtkm::Float32 operator()(
      CellShape shape,
      const PointCoordinatesType &pointCoordinates,
      const TriangleQualityTableType &triangleQualityTable) const
  {
    if (shape.Id != vtkm::CELL_SHAPE_TRIANGLE)
    {
      this->RaiseError("Only triangles are supported for triangle quality.");
      return vtkm::Nan32();
    }

    return triangleQualityTable.GetQuality(pointCoordinates[0],
                                           pointCoordinates[1],
                                           pointCoordinates[2]);
  }
};

// Normally we would encapsulate this call in a filter, but for demonstrative
// purposes we are just calling the worklet directly.
template<typename DeviceAdapterTag>
VTKM_CONT
vtkm::cont::ArrayHandle<vtkm::Float32>
RunTriangleQuality2(vtkm::cont::DataSet dataSet,
                    DeviceAdapterTag)
{
  TriangleQualityTable<DeviceAdapterTag> triangleQualityTable;

  vtkm::cont::ArrayHandle<vtkm::Float32> triangleQualities;

  vtkm::worklet::DispatcherMapTopology<TriangleQualityWorklet2,DeviceAdapterTag>
      dispatcher;
  dispatcher.Invoke(dataSet.GetCellSet(),
                    dataSet.GetCoordinateSystem().GetData(),
                    triangleQualityTable,
                    triangleQualities);

  return triangleQualities;
}
////
//// END-EXAMPLE TriangleQualityExecObject.cxx
////

} // namespace TriangleQualityNamespace

#include <vtkm/cont/ArrayHandleUniformPointCoordinates.h>
#include <vtkm/cont/DataSetBuilderExplicit.h>

#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/WorkletMapField.h>

#include <vtkm/cont/testing/Testing.h>

namespace TriangleQualityNamespace {

template<typename T>
VTKM_EXEC
T TriangleQuality(const vtkm::Vec<T,3> &edgeLengths)
{
  // Heron's formula for triangle area.
  T semiperimeter = (edgeLengths[0]+edgeLengths[1]+edgeLengths[2])/2;
  T area = vtkm::Sqrt(semiperimeter*
                      (semiperimeter - edgeLengths[0])*
                      (semiperimeter - edgeLengths[1])*
                      (semiperimeter - edgeLengths[2]));

  if (!vtkm::IsFinite(area))
  {
    // If the edge lengths do not make a valid triangle (i.e. the sum of the
    // two smaller lengths is smaller than the larger length), then Heron's
    // formula gives an imaginary number, which we expect to result in a NaN.
    // If that happens, just return a quality of 0 for the degenerate triangle.
    return 0;
  }

  // Formula for triangle quality.
  return 4*area*vtkm::Sqrt(T(3))/vtkm::MagnitudeSquared(edgeLengths);
}

struct ComputeTriangleQualityValues : vtkm::worklet::WorkletMapField
{
  typedef void ControlSignature(FieldIn<Vec3>, FieldOut<Scalar>);
  typedef _2 ExecutionSignature(_1);

  template<typename T>
  VTKM_EXEC
  T operator()(const vtkm::Vec<T,3> &edgeLengths) const
  {
    return TriangleQuality(edgeLengths);
  }
};

VTKM_CONT
vtkm::cont::ArrayHandle<vtkm::Float32>
BuildTriangleQualityTable()
{
  // Repurpose uniform point coordinates to compute triange edge lengths.
  vtkm::cont::ArrayHandleUniformPointCoordinates edgeLengths(
        vtkm::Id3(TRIANGLE_QUALITY_TABLE_DIMENSION,
                  TRIANGLE_QUALITY_TABLE_DIMENSION,
                  1),
        vtkm::Vec<vtkm::FloatDefault,3>(0, 0, 1),
        vtkm::Vec<vtkm::FloatDefault,3>(1.0f/(TRIANGLE_QUALITY_TABLE_DIMENSION-1),
                                        1.0f/(TRIANGLE_QUALITY_TABLE_DIMENSION-1),
                                        1.0f));

  vtkm::cont::ArrayHandle<vtkm::Float32> triQualityArray;

  vtkm::worklet::DispatcherMapField<ComputeTriangleQualityValues> dispatcher;
  dispatcher.Invoke(edgeLengths, triQualityArray);

  return triQualityArray;
}

template<typename PortalType>
VTKM_CONT
void PrintTriangleQualityTable(const PortalType &portal)
{
  for (vtkm::Id index = 0; index < portal.GetNumberOfValues(); index++)
  {
    if (index%TRIANGLE_QUALITY_TABLE_DIMENSION == 0)
    {
      std::cout << std::endl;
    }
    std::cout << portal.Get(index) << ", ";
  }
  std::cout << std::endl << std::endl;
}

VTKM_CONT
vtkm::cont::DataSet BuildDataSet()
{
  static const vtkm::Id NUM_ROWS = 5;

  vtkm::cont::DataSetBuilderExplicitIterative dataSetBuilder;
  dataSetBuilder.Begin();

  for (vtkm::Id row = 0; row < NUM_ROWS; row++)
  {
    dataSetBuilder.AddPoint(0, static_cast<vtkm::Float32>(row*row), 0);
    dataSetBuilder.AddPoint(1, static_cast<vtkm::Float32>(row*row), 0);
  }

  for (vtkm::Id row = 0; row < NUM_ROWS-1; row++)
  {
    vtkm::Id firstPoint = 2*row;

    dataSetBuilder.AddCell(vtkm::CELL_SHAPE_TRIANGLE);
    dataSetBuilder.AddCellPoint(firstPoint+0);
    dataSetBuilder.AddCellPoint(firstPoint+1);
    dataSetBuilder.AddCellPoint(firstPoint+2);

    dataSetBuilder.AddCell(vtkm::CELL_SHAPE_TRIANGLE);
    dataSetBuilder.AddCellPoint(firstPoint+1);
    dataSetBuilder.AddCellPoint(firstPoint+3);
    dataSetBuilder.AddCellPoint(firstPoint+2);
  }

  return dataSetBuilder.Create();
}

VTKM_CONT
void CheckQualityArray(vtkm::cont::ArrayHandle<vtkm::Float32> qualities)
{
  vtkm::cont::printSummary_ArrayHandle(qualities, std::cout);
  std::cout << std::endl;

  vtkm::cont::ArrayHandle<vtkm::Float32>::PortalConstControl qualityPortal =
      qualities.GetPortalConstControl();

  // Pairwise triangles should have the same quality.
  for (vtkm::Id pairIndex = 0;
       pairIndex < qualities.GetNumberOfValues()/2;
       pairIndex++)
  {
    vtkm::Float32 q1 = qualityPortal.Get(2*pairIndex);
    vtkm::Float32 q2 = qualityPortal.Get(2*pairIndex+1);
    VTKM_TEST_ASSERT(test_equal(q1,q2),
                     "Isometric triangles have different quality.");
  }

  // Triangle qualities should be monotonically decreasing.
  vtkm::Float32 lastQuality = 1;
  for (vtkm::Id triIndex = 0;
       triIndex < qualities.GetNumberOfValues();
       triIndex++)
  {
    vtkm::Float32 quality = qualityPortal.Get(triIndex);
    VTKM_TEST_ASSERT(test_equal(quality,lastQuality)
                     || (quality <= lastQuality),
                     "Triangle quality not monotonically decreasing.");
    lastQuality = quality;
  }

  // The first quality should definitely be better than the last.
  vtkm::Float32 firstQuality = qualityPortal.Get(0);
  VTKM_TEST_ASSERT(firstQuality > lastQuality,
                   "First quality not better than last.");
}

VTKM_CONT
void TestTriangleQuality()
{
  std::cout << "Building triangle quality array." << std::endl;
  vtkm::cont::ArrayHandle<vtkm::Float32> triQualityTable =
      BuildTriangleQualityTable();
  VTKM_TEST_ASSERT(
        triQualityTable.GetNumberOfValues() ==
        TRIANGLE_QUALITY_TABLE_DIMENSION*TRIANGLE_QUALITY_TABLE_DIMENSION,
        "Bad size for triangle quality array.");
  PrintTriangleQualityTable(triQualityTable.GetPortalConstControl());

  std::cout << "Creating a data set." << std::endl;
  vtkm::cont::DataSet dataSet = BuildDataSet();

  std::cout << "Getting triangle quality using whole array argument."
            << std::endl;
  vtkm::cont::ArrayHandle<vtkm::Float32> qualities =
      RunTriangleQuality(dataSet, VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
  CheckQualityArray(qualities);

  std::cout << "Getting triangle quality using execution object argument."
            << std::endl;
  qualities = RunTriangleQuality2(dataSet, VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
  CheckQualityArray(qualities);
}

} // namespace TriangleQualityNamespace

int TriangleQuality(int, char*[])
{
  return vtkm::cont::testing::Testing::Run(
        TriangleQualityNamespace::TestTriangleQuality);
}
