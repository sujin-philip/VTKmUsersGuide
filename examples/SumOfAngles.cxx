#include <vtkm/worklet/DispatcherMapTopology.h>
#include <vtkm/worklet/WorkletMapTopology.h>

#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/CellSetExplicit.h>
#include <vtkm/cont/DataSet.h>
#include <vtkm/cont/DataSetFieldAdd.h>
#include <vtkm/cont/DeviceAdapter.h>

#include <vtkm/exec/CellEdge.h>

#include <vtkm/Math.h>
#include <vtkm/VectorAnalysis.h>

#include <vtkm/io/reader/VTKDataSetReader.h>
#include <vtkm/io/writer/VTKDataSetWriter.h>

#include <vtkm/cont/testing/Testing.h>

struct GaussianCurvature
{
  // This worklet computes the sum of the angles of all polygons connected
  // to each point. This sum is related (but not equal to) the Gaussian
  // curvature of the surface. A flat mesh will have a sum equal to 2 pi.
  // A concave or convex surface will have a sum less than 2 pi. A saddle
  // will have a sum greater than 2 pi. The actual Gaussian curvature is
  // equal to (2 pi - angle sum)/A where A is an area of influence (which
  // we are not calculating here). See
  // http://computergraphics.stackexchange.com/questions/1718/what-is-the-simplest-way-to-compute-principal-curvature-for-a-mesh-triangle#1721
  // or the publication "Discrete Differential-Geometry Operators for
  // Triangulated 2-Manifolds" by Mayer et al. (Equation 9).
  ////
  //// BEGIN-EXAMPLE SumOfAngles.cxx
  ////
  struct SumOfAngles : vtkm::worklet::WorkletMapCellToPoint
  {
    typedef void ControlSignature(CellSetIn inputCells,
                                  WholeCellSetIn<>, // Same as inputCells
                                  WholeArrayIn<> pointCoords,
                                  FieldOutPoint<Scalar> angleSum);
    typedef void ExecutionSignature(CellIndices incidentCells,
                                    InputIndex pointIndex,
                                    _2 cellSet,
                                    _3 pointCoordsPortal,
                                    _4 outSum);
    using InputDomain = _1;

    template<typename IncidentCellVecType,
             typename CellSetType,
             typename PointCoordsPortalType,
             typename SumType>
    VTKM_EXEC
    void operator()(const IncidentCellVecType &incidentCells,
                    vtkm::Id pointIndex,
                    const CellSetType &cellSet,
                    const PointCoordsPortalType &pointCoordsPortal,
                    SumType &outSum) const
    {
      using CoordType = PointCoordsPortalType::ValueType;

      CoordType thisPoint = pointCoordsPortal.Get(pointIndex);

      outSum = 0;
      for (vtkm::IdComponent incidentCellIndex = 0;
           incidentCellIndex < incidentCells.GetNumberOfComponents();
           ++incidentCellIndex)
      {
        // Get information about incident cell.
        vtkm::Id cellIndex = incidentCells[incidentCellIndex];
        typename CellSetType::CellShapeTag cellTag =
            cellSet.GetCellShape(cellIndex);
        typename CellSetType::IndicesType cellConnections =
            cellSet.GetIndices(cellIndex);
        vtkm::IdComponent numConnections =
            cellSet.GetNumberOfIndices(cellIndex);
        vtkm::IdComponent numEdges =
            vtkm::exec::CellEdgeNumberOfEdges(numConnections, cellTag, *this);

        // Iterate over all edges and find the first one with pointIndex.
        // Use that to find the first vector.
        vtkm::IdComponent edgeIndex = -1;
        CoordType vec1;
        while (true)
        {
          ++edgeIndex;
          if (edgeIndex >= numEdges)
          {
            this->RaiseError("Bad cell. Could not find two incident edges.");
            return;
          }
          vtkm::Vec<vtkm::IdComponent,2> edge =
              vtkm::exec::CellEdgeLocalIndices(
                numConnections, edgeIndex, cellTag, *this);
          if (cellConnections[edge[0]] == pointIndex)
          {
            vec1 = pointCoordsPortal.Get(cellConnections[edge[1]]) - thisPoint;
            break;
          }
          else if (cellConnections[edge[1]] == pointIndex)
          {
            vec1 = pointCoordsPortal.Get(cellConnections[edge[0]]) - thisPoint;
            break;
          }
          else
          {
            // Continue to next iteration of loop.
          }
        }

        // Continue iteration over remaining edges and find the second one with
        // pointIndex. Use that to find the second vector.
        CoordType vec2;
        while (true)
        {
          ++edgeIndex;
          if (edgeIndex >= numEdges)
          {
            this->RaiseError("Bad cell. Could not find two incident edges.");
            return;
          }
          vtkm::Vec<vtkm::IdComponent,2> edge =
              vtkm::exec::CellEdgeLocalIndices(
                numConnections, edgeIndex, cellTag, *this);
          if (cellConnections[edge[0]] == pointIndex)
          {
            vec2 = pointCoordsPortal.Get(cellConnections[edge[1]]) - thisPoint;
            break;
          }
          else if (cellConnections[edge[1]] == pointIndex)
          {
            vec2 = pointCoordsPortal.Get(cellConnections[edge[0]]) - thisPoint;
            break;
          }
          else
          {
            // Continue to next iteration of loop.
          }
        }

        // The dot product of two unit vectors is equal to the cosine of the
        // angle between them.
        vtkm::Normalize(vec1);
        vtkm::Normalize(vec2);
        SumType cosine = static_cast<SumType>(vtkm::dot(vec1, vec2));

        outSum += vtkm::ACos(cosine);
      }
    }
  };
  ////
  //// END-EXAMPLE SumOfAngles.cxx
  ////

  template<typename CellSetType,
           typename T,
           typename CoordStorage,
           typename Device>
  static vtkm::cont::ArrayHandle<T>
  Run(const CellSetType &cellSet,
      const vtkm::cont::ArrayHandle<vtkm::Vec<T,3>, CoordStorage> &pointCoords,
      Device)
  {
    VTKM_IS_CELL_SET(CellSetType);

    vtkm::cont::ArrayHandle<T> angleSums;

    vtkm::worklet::DispatcherMapTopology<SumOfAngles, Device> dispatcher;
    dispatcher.Invoke(cellSet, cellSet, pointCoords, angleSums);

    return angleSums;
  }
};

VTKM_CONT
static void TrySumOfAngles()
{
  std::cout << "Read input data" << std::endl;
  vtkm::io::reader::VTKDataSetReader reader("data/cow.vtk");
  vtkm::cont::DataSet dataSet = reader.ReadDataSet();

  std::cout << "Get information out of data" << std::endl;
  vtkm::cont::CellSetExplicit<> cellSet;
  dataSet.GetCellSet().CopyTo(cellSet);

  vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::Float32,3> > pointCoordinates;
  dataSet.GetCoordinateSystem().GetData().CopyTo(pointCoordinates);

  std::cout << "Run algorithm" << std::endl;
  vtkm::cont::ArrayHandle<vtkm::Float32> angleSums =
      GaussianCurvature::Run(
        cellSet, pointCoordinates, VTKM_DEFAULT_DEVICE_ADAPTER_TAG());

  std::cout << "Add field to data set" << std::endl;
  vtkm::cont::DataSetFieldAdd::AddPointField(dataSet, "angle-sum", angleSums);

  std::cout << "Write result" << std::endl;
  vtkm::io::writer::VTKDataSetWriter writer("cow-curvature.vtk");
  writer.WriteDataSet(dataSet);
}

int SumOfAngles(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(TrySumOfAngles);
}
