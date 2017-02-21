#include <vtkm/exec/CellEdge.h>
#include <vtkm/exec/CellFace.h>

#include <vtkm/worklet/DispatcherMapTopology.h>
#include <vtkm/worklet/ScatterCounting.h>
#include <vtkm/worklet/WorkletMapTopology.h>

#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayHandleGroupVec.h>
#include <vtkm/cont/ArrayHandleGroupVecVariable.h>
#include <vtkm/cont/CellSetSingleType.h>

#include <vtkm/cont/testing/MakeTestDataSet.h>
#include <vtkm/cont/testing/Testing.h>

namespace {

struct ExtractEdges {
  ////
  //// BEGIN-EXAMPLE CellEdge.cxx
  ////
  struct EdgesCount : vtkm::worklet::WorkletMapPointToCell
  {
    typedef void ControlSignature(CellSetIn,
                                  FieldOutCell<> numEdgesInCell);
    typedef _2 ExecutionSignature(CellShape, PointCount);
    typedef _1 InputDomain;

    template<typename CellShapeTag>
    VTKM_EXEC
    vtkm::IdComponent
    operator()(CellShapeTag shape, vtkm::IdComponent numPointsInCell) const
    {
      return vtkm::exec::CellEdgeNumberOfEdges(numPointsInCell, shape, *this);
    }
  };

  struct EdgesExtract : vtkm::worklet::WorkletMapPointToCell
  {
    typedef void ControlSignature(CellSetIn,
                                  FieldOutCell<> edgeIndices);
    typedef void ExecutionSignature(CellShape, PointIndices, VisitIndex, _2);
    typedef _1 InputDomain;

    typedef vtkm::worklet::ScatterCounting ScatterType;
    VTKM_CONT ScatterType GetScatter() const { return this->Scatter; }

    VTKM_CONT
    EdgesExtract(const ScatterType &scatter)
      : Scatter(scatter) {  }

    template<typename CellShapeTag,
             typename PointIndexVecType,
             typename EdgeIndexVecType>
    VTKM_EXEC
    void operator()(CellShapeTag shape,
                    const PointIndexVecType &pointIndices,
                    vtkm::IdComponent visitIndex,
                    EdgeIndexVecType &edgeIndices) const
    {
      vtkm::Vec<vtkm::IdComponent,2> localEdgeIndices =
          vtkm::exec::CellEdgeLocalIndices(pointIndices.GetNumberOfComponents(),
                                           visitIndex,
                                           shape,
                                           *this);
      edgeIndices[0] = pointIndices[localEdgeIndices[0]];
      edgeIndices[1] = pointIndices[localEdgeIndices[1]];
    }

  private:
    ScatterType Scatter;
  };
  ////
  //// END-EXAMPLE CellEdge.cxx
  ////

  template<typename CellSetInType, typename Device>
  VTKM_CONT
  vtkm::cont::CellSetSingleType<>
  Run(const CellSetInType &cellSetIn, Device)
  {
    // Count how many edges each cell has
    vtkm::cont::ArrayHandle<vtkm::IdComponent> edgeCounts;
    vtkm::worklet::DispatcherMapTopology<EdgesCount,Device> countDispatcher;
    countDispatcher.Invoke(cellSetIn, edgeCounts);

    // Set up a "scatter" to create an output entry for each edge in the input
    vtkm::worklet::ScatterCounting scatter(edgeCounts, Device());

    // Get the cell index array for all the edges
    vtkm::cont::ArrayHandle<vtkm::Id> edgeIndices;
    vtkm::worklet::DispatcherMapTopology<EdgesExtract,Device>
        extractDispatcher(scatter);
    extractDispatcher.Invoke(
          cellSetIn,
          vtkm::cont::make_ArrayHandleGroupVec<2>(edgeIndices));

    // Construct the resulting cell set and return
    vtkm::cont::CellSetSingleType<> cellSetOut(cellSetIn.GetName());
    cellSetOut.Fill(cellSetIn.GetNumberOfPoints(),
                    vtkm::CELL_SHAPE_LINE,
                    2,
                    edgeIndices);
    return cellSetOut;
  }
};

void TryExtractEdges()
{
  std::cout << "Trying extract edges worklets." << std::endl;

  vtkm::cont::DataSet dataSet =
      vtkm::cont::testing::MakeTestDataSet().Make3DExplicitDataSet5();

  ExtractEdges extractEdges;
  vtkm::cont::CellSetSingleType<> edgeCells =
      extractEdges.Run(dataSet.GetCellSet(),
                       VTKM_DEFAULT_DEVICE_ADAPTER_TAG());

  VTKM_TEST_ASSERT(edgeCells.GetNumberOfPoints() == 11,
                   "Output has wrong number of points");
  VTKM_TEST_ASSERT(edgeCells.GetNumberOfCells() == 35,
                   "Output has wrong number of cells");
}

struct ExtractFaces {
  ////
  //// BEGIN-EXAMPLE CellFace.cxx
  ////
  struct FacesCount : vtkm::worklet::WorkletMapPointToCell
  {
    typedef void ControlSignature(CellSetIn,
                                  FieldOutCell<> numFacesInCell);
    typedef _2 ExecutionSignature(CellShape);
    typedef _1 InputDomain;

    template<typename CellShapeTag>
    VTKM_EXEC
    vtkm::IdComponent
    operator()(CellShapeTag shape) const
    {
      return vtkm::exec::CellFaceNumberOfFaces(shape, *this);
    }
  };

  struct FacesCountPoints : vtkm::worklet::WorkletMapPointToCell
  {
    typedef void ControlSignature(CellSetIn,
                                  FieldOutCell<> numPointsInFace,
                                  FieldOutCell<> faceShape);
    typedef void ExecutionSignature(CellShape, VisitIndex, _2, _3);
    typedef _1 InputDomain;

    typedef vtkm::worklet::ScatterCounting ScatterType;
    VTKM_CONT ScatterType GetScatter() const { return this->Scatter; }

    VTKM_CONT
    FacesCountPoints(const ScatterType &scatter)
      : Scatter(scatter) {  }

    template<typename CellShapeTag>
    VTKM_EXEC
    void operator()(CellShapeTag shape,
                    vtkm::IdComponent visitIndex,
                    vtkm::IdComponent &numPointsInFace,
                    vtkm::UInt8 &faceShape) const
    {
      numPointsInFace =
          vtkm::exec::CellFaceNumberOfPoints(visitIndex, shape, *this);
      switch (numPointsInFace)
      {
        case 3: faceShape = vtkm::CELL_SHAPE_TRIANGLE; break;
        case 4: faceShape = vtkm::CELL_SHAPE_QUAD; break;
        default: faceShape = vtkm::CELL_SHAPE_POLYGON; break;
      }
    }

  private:
    ScatterType Scatter;
  };

  struct FacesExtract : vtkm::worklet::WorkletMapPointToCell
  {
    typedef void ControlSignature(CellSetIn,
                                  FieldOutCell<> faceIndices);
    typedef void ExecutionSignature(CellShape, PointIndices, VisitIndex, _2);
    typedef _1 InputDomain;

    typedef vtkm::worklet::ScatterCounting ScatterType;
    VTKM_CONT ScatterType GetScatter() const { return this->Scatter; }

    VTKM_CONT
    FacesExtract(const ScatterType &scatter)
      : Scatter(scatter) {  }

    template<typename CellShapeTag,
             typename PointIndexVecType,
             typename FaceIndexVecType>
    VTKM_EXEC
    void operator()(CellShapeTag shape,
                    const PointIndexVecType &pointIndices,
                    vtkm::IdComponent visitIndex,
                    FaceIndexVecType &faceIndices) const
    {
      vtkm::VecCConst<vtkm::IdComponent> localFaceIndices =
          vtkm::exec::CellFaceLocalIndices(visitIndex,
                                           shape,
                                           *this);

      vtkm::IdComponent numPoints = faceIndices.GetNumberOfComponents();
      VTKM_ASSERT(numPoints == localFaceIndices.GetNumberOfComponents());
      for (vtkm::IdComponent localPointIndex = 0;
           localPointIndex < numPoints;
           localPointIndex++)
      {
        faceIndices[localPointIndex] =
            pointIndices[localFaceIndices[localPointIndex]];
      }
    }

  private:
    ScatterType Scatter;
  };
  ////
  //// END-EXAMPLE CellFace.cxx
  ////

  template<typename CellSetInType, typename Device>
  VTKM_CONT
  vtkm::cont::CellSetExplicit<>
  Run(const CellSetInType &cellSetIn, Device)
  {
    // Count how many faces each cell has
    vtkm::cont::ArrayHandle<vtkm::IdComponent> faceCounts;
    vtkm::worklet::DispatcherMapTopology<FacesCount,Device> countDispatcher;
    countDispatcher.Invoke(cellSetIn, faceCounts);

    // Set up a "scatter" to create an output entry for each face in the input
    vtkm::worklet::ScatterCounting scatter(faceCounts, Device());

    // Count how many points each face has. Also get the shape of each face.
    vtkm::cont::ArrayHandle<vtkm::IdComponent> pointsPerFace;
    vtkm::cont::ArrayHandle<vtkm::UInt8> faceShapes;
    vtkm::worklet::DispatcherMapTopology<FacesCountPoints,Device>
        countPointsDispatcher(scatter);
    countPointsDispatcher.Invoke(cellSetIn, pointsPerFace, faceShapes);

    // To construct an ArrayHandleGroupVecVariable, we need to convert
    // pointsPerFace to an array of offsets
    vtkm::Id faceIndicesSize;
    vtkm::cont::ArrayHandle<vtkm::Id> faceIndexOffsets =
        vtkm::cont::ConvertNumComponentsToOffsets(pointsPerFace,
                                                  faceIndicesSize);

    // We need to preallocate the array for faceIndices (because that is the
    // way ArrayHandleGroupVecVariable works). We use the value previously
    // returned from ConvertNumComponentsToOffsets.
    vtkm::cont::ArrayHandle<vtkm::Id> faceIndices;
    faceIndices.Allocate(faceIndicesSize);

    // Get the cell index array for all the faces
    vtkm::worklet::DispatcherMapTopology<FacesExtract,Device>
        extractDispatcher(scatter);
    extractDispatcher.Invoke(
          cellSetIn,
          vtkm::cont::make_ArrayHandleGroupVecVariable(faceIndices,
                                                       faceIndexOffsets));

    // Construct the resulting cell set and return
    vtkm::cont::CellSetExplicit<> cellSetOut(cellSetIn.GetName());
    cellSetOut.Fill(cellSetIn.GetNumberOfPoints(),
                    faceShapes,
                    pointsPerFace,
                    faceIndices,
                    faceIndexOffsets);
    return cellSetOut;
  }
};

void TryExtractFaces()
{
  std::cout << "Trying extract faces worklets." << std::endl;

  vtkm::cont::DataSet dataSet =
      vtkm::cont::testing::MakeTestDataSet().Make3DExplicitDataSet5();

  ExtractFaces extractFaces;
  vtkm::cont::CellSetExplicit<> faceCells =
      extractFaces.Run(dataSet.GetCellSet(),
                       VTKM_DEFAULT_DEVICE_ADAPTER_TAG());

  VTKM_TEST_ASSERT(faceCells.GetNumberOfPoints() == 11,
                   "Output has wrong number of points");
  VTKM_TEST_ASSERT(faceCells.GetNumberOfCells() == 20,
                   "Output has wrong number of cells");

  VTKM_TEST_ASSERT(faceCells.GetCellShape(0) == vtkm::CELL_SHAPE_QUAD,
                   "Face wrong");
  vtkm::Vec<vtkm::Id,4> quadIndices;
  faceCells.GetIndices(0, quadIndices);
  VTKM_TEST_ASSERT(test_equal(quadIndices, vtkm::Vec<vtkm::Id,4>(0,3,7,4)),
                   "Face wrong");

  VTKM_TEST_ASSERT(faceCells.GetCellShape(12) == vtkm::CELL_SHAPE_TRIANGLE,
                   "Face wrong");
  vtkm::Vec<vtkm::Id,3> triIndices;
  faceCells.GetIndices(12, triIndices);
  VTKM_TEST_ASSERT(test_equal(triIndices, vtkm::Id3(8,10,6)),
                   "Face wrong");
}

void Run()
{
  TryExtractEdges();
  TryExtractFaces();
}

} // anonymous namespace

int CellEdgesFaces(int, char*[])
{
  return vtkm::cont::testing::Testing::Run(Run);
}
