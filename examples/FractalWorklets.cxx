#include <vtkm/TypeListTag.h>

#include <vtkm/cont/ArrayHandleGroupVec.h>
#include <vtkm/cont/ArrayRangeCompute.h>

#include <vtkm/cont/arg/Transport.h>
#include <vtkm/cont/arg/TypeCheck.h>
#include <vtkm/cont/arg/TypeCheckTagArray.h>

#include <vtkm/exec/arg/AspectTagDefault.h>
#include <vtkm/exec/arg/Fetch.h>
#include <vtkm/exec/arg/ThreadIndicesBasic.h>

#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/WorkletMapField.h>

#include <vtkm/cont/testing/Testing.h>

#include <fstream>
#include <type_traits>

template<typename T>
static vtkm::Vec<T,2> TransformSVGPoint(const vtkm::Vec<T,2> &point,
                                        const vtkm::Range xRange,
                                        const vtkm::Range yRange,
                                        float padding)
{
  return vtkm::Vec<T,2>(point[0] - xRange.Min + padding,
                        yRange.Max - point[1] + padding);
}

template<typename T>
static void WriteSVG(const std::string &filename,
                     const vtkm::cont::ArrayHandle<vtkm::Vec<T,2> > &data,
                     float width = 2.0,
                     const std::string &color = "black")
{
  static const float PADDING = 0.05;

  vtkm::cont::ArrayHandle<vtkm::Range> ranges =
      vtkm::cont::ArrayRangeCompute(data, VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
  vtkm::Range xRange = ranges.GetPortalConstControl().Get(0);
  vtkm::Range yRange = ranges.GetPortalConstControl().Get(1);

  std::ofstream file(filename.c_str());

  file << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
  file << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" "
       << "width=\"" << xRange.Length() + 2*PADDING << "in\" "
       << "height=\"" << yRange.Length() + 2*PADDING << "in\" "
       << ">\n";

  typename vtkm::cont::ArrayHandle<vtkm::Vec<T,2> >::PortalConstControl portal =
      data.GetPortalConstControl();
  for (vtkm::Id lineIndex = 0;
       lineIndex < portal.GetNumberOfValues()/2;
       ++lineIndex)
  {
    vtkm::Vec<T,2> p1 =
        TransformSVGPoint(portal.Get(lineIndex*2+0), xRange, yRange, PADDING);
    vtkm::Vec<T,2> p2 =
        TransformSVGPoint(portal.Get(lineIndex*2+1), xRange, yRange, PADDING);

    file << "  <line x1=\"" << p1[0]
         << "in\" y1=\"" << p1[1]
         << "in\" x2=\"" << p2[0]
         << "in\" y2=\"" << p2[1]
         << "in\" stroke=\"" << color
         << "\" stroke-width=\"" << width << "\" />\n";
  }

  file << "</svg>\n";
  file.close();
}

////
//// BEGIN-EXAMPLE TypeCheckImpl.h
////
namespace vtkm {
namespace cont {
namespace arg {

struct TypeCheckTag2DCoordinates {  };

template<typename ArrayType>
struct TypeCheck<TypeCheckTag2DCoordinates, ArrayType>
{
  static const bool value = vtkm::cont::arg::TypeCheck<
      vtkm::cont::arg::TypeCheckTagArray<vtkm::TypeListTagFieldVec2>,ArrayType
    >::value;
};

}
}
} // namespace vtkm::cont::arg
////
//// END-EXAMPLE TypeCheckImpl.h
////

////
//// BEGIN-EXAMPLE TransportImpl.h
////
namespace vtkm {
namespace cont {
namespace arg {

struct TransportTag2DLineSegmentsIn {  };

template<typename ContObjectType, typename Device>
struct Transport<
    vtkm::cont::arg::TransportTag2DLineSegmentsIn, ContObjectType, Device>
{
  VTKM_IS_ARRAY_HANDLE(ContObjectType);

  using GroupedArrayType = vtkm::cont::ArrayHandleGroupVec<ContObjectType,2>;

  using ExecObjectType =
      typename GroupedArrayType::template ExecutionTypes<Device>::PortalConst;

  template<typename InputDomainType>
  VTKM_CONT
  ExecObjectType operator()(const ContObjectType &object,
                            const InputDomainType &,
                            vtkm::Id size) const
  {
    VTKM_ASSERT(object.GetNumberOfValues() == size*2);
    (void)size;  // Shut up, compiler.

    GroupedArrayType groupedArray(object);
    return groupedArray.PrepareForInput(Device());
  }
};

}
}
} // namespace vtkm::cont::arg
////
//// END-EXAMPLE TransportImpl.h
////

////
//// BEGIN-EXAMPLE FetchImplBasic.h
////
namespace vtkm {
namespace exec {
namespace arg {

struct FetchTag2DLineSegmentsIn {  };

template<typename ThreadIndicesType, typename ExecObjectType>
struct Fetch<
    vtkm::exec::arg::FetchTag2DLineSegmentsIn,
    vtkm::exec::arg::AspectTagDefault,
    ThreadIndicesType,
    ExecObjectType>
{
  using ValueType = typename ExecObjectType::ValueType;

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC
  ValueType Load(const ThreadIndicesType &indices,
                 const ExecObjectType &arrayPortal) const
  {
    return arrayPortal.Get(indices.GetInputIndex());
  }

  VTKM_EXEC
  void Store(const ThreadIndicesType &,
             const ExecObjectType &,
             const ValueType &) const
  {
    // Store is a no-op for this fetch.
  }
};

}
}
} // namespace vtkm::exec::arg
////
//// END-EXAMPLE FetchImplBasic.h
////

////
//// BEGIN-EXAMPLE AspectImpl.h
////
namespace vtkm {
namespace exec {
namespace arg {

struct AspectTagFirstPoint {  };

template<typename ThreadIndicesType, typename ExecObjectType>
struct Fetch<
    vtkm::exec::arg::FetchTag2DLineSegmentsIn,
    vtkm::exec::arg::AspectTagFirstPoint,
    ThreadIndicesType,
    ExecObjectType>
{
  using ValueType = typename ExecObjectType::ValueType::ComponentType;

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC
  ValueType Load(const ThreadIndicesType &indices,
                 const ExecObjectType &arrayPortal) const
  {
    return arrayPortal.Get(indices.GetInputIndex())[0];
  }

  VTKM_EXEC
  void Store(const ThreadIndicesType &,
             const ExecObjectType &,
             const ValueType &) const
  {
    // Store is a no-op for this fetch.
  }
};

//// PAUSE-EXAMPLE
struct AspectTagSecondPoint {  };

template<typename ThreadIndicesType, typename ExecObjectType>
struct Fetch<
    vtkm::exec::arg::FetchTag2DLineSegmentsIn,
    vtkm::exec::arg::AspectTagSecondPoint,
    ThreadIndicesType,
    ExecObjectType>
{
  using ValueType = typename ExecObjectType::ValueType::ComponentType;

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC
  ValueType Load(const ThreadIndicesType &indices,
                 const ExecObjectType &arrayPortal) const
  {
    return arrayPortal.Get(indices.GetInputIndex())[1];
  }

  VTKM_EXEC
  void Store(const ThreadIndicesType &,
             const ExecObjectType &,
             const ValueType &) const
  {
    // Store is a no-op for this fetch.
  }
};

//// RESUME-EXAMPLE
}
}
} // namespace vtkm::exec::arg
////
//// END-EXAMPLE AspectImpl.h
////

struct VecLineSegments : vtkm::worklet::WorkletMapField
{
  ////
  //// BEGIN-EXAMPLE CustomControlSignatureTag.cxx
  ////
  struct LineSegment2DCoordinatesIn : vtkm::cont::arg::ControlSignatureTagBase
  {
    using TypeCheckTag = vtkm::cont::arg::TypeCheckTag2DCoordinates;
    using TransportTag = vtkm::cont::arg::TransportTag2DLineSegmentsIn;
    using FetchTag = vtkm::exec::arg::FetchTag2DLineSegmentsIn;
  };
  ////
  //// END-EXAMPLE CustomControlSignatureTag.cxx
  ////

  ////
  //// BEGIN-EXAMPLE CustomExecutionSignatureTag.cxx
  ////
  template<typename ArgTag>
  struct FirstPoint : vtkm::exec::arg::ExecutionSignatureTagBase
  {
    static const vtkm::IdComponent INDEX = ArgTag::INDEX;
    using AspectTag = vtkm::exec::arg::AspectTagFirstPoint;
  };
  ////
  //// END-EXAMPLE CustomExecutionSignatureTag.cxx
  ////

  template<typename ArgTag>
  struct SecondPoint : vtkm::exec::arg::ExecutionSignatureTagBase
  {
    static const vtkm::IdComponent INDEX = ArgTag::INDEX;
    using AspectTag = vtkm::exec::arg::AspectTagSecondPoint;
  };

  ////
  //// BEGIN-EXAMPLE UseCustomControlSignatureTag.cxx
  ////
  ////
  //// BEGIN-EXAMPLE UseCustomExecutionSignatureTag.cxx
  ////
  typedef void ControlSignature(LineSegment2DCoordinatesIn coordsIn,
                                FieldOut<Vec2> vecOut,
                                FieldIn<Index> index);
  ////
  //// END-EXAMPLE UseCustomControlSignatureTag.cxx
  ////
  typedef void ExecutionSignature(FirstPoint<_1>, SecondPoint<_1>, _2);
  ////
  //// END-EXAMPLE UseCustomExecutionSignatureTag.cxx
  ////
  using InputDomain = _3;

  template<typename T>
  VTKM_EXEC
  void operator()(const vtkm::Vec<T,2> &firstPoint,
                  const vtkm::Vec<T,2> &secondPoint,
                  vtkm::Vec<T,2> &vecOut) const
  {
    vecOut = secondPoint - firstPoint;
  }
};

void TryVecLineSegments()
{
  using VecType = vtkm::Vec<vtkm::FloatDefault,2>;
  static const vtkm::Id ARRAY_SIZE = 10;

  vtkm::cont::ArrayHandle<VecType> inputArray;
  inputArray.Allocate(ARRAY_SIZE*2);
  SetPortal(inputArray.GetPortalControl());

  vtkm::cont::ArrayHandle<VecType> outputArray;

  vtkm::worklet::DispatcherMapField<VecLineSegments> dispatcher;
  dispatcher.Invoke(
        inputArray, outputArray, vtkm::cont::ArrayHandleIndex(ARRAY_SIZE));

  VTKM_TEST_ASSERT(outputArray.GetNumberOfValues() == ARRAY_SIZE,
                   "Output wrong size.");

  for (vtkm::Id index = 0; index < ARRAY_SIZE; ++index)
  {
    VecType expectedVec =
        TestValue(index*2+1,VecType()) - TestValue(index*2,VecType());
    VecType computedVec = outputArray.GetPortalConstControl().Get(index);
    VTKM_TEST_ASSERT(test_equal(expectedVec, computedVec), "Bad value.");
  }
}

////
//// BEGIN-EXAMPLE TransportImpl2.h
////
namespace vtkm {
namespace cont {
namespace arg {

template<vtkm::IdComponent NumOutputPerInput>
struct TransportTag2DLineSegmentsOut {  };

template<vtkm::IdComponent NumOutputPerInput,
         typename ContObjectType,
         typename Device>
struct Transport<
    vtkm::cont::arg::TransportTag2DLineSegmentsOut<NumOutputPerInput>,
    ContObjectType,
    Device>
{
  VTKM_IS_ARRAY_HANDLE(ContObjectType);

  using GroupedArrayType =
      vtkm::cont::ArrayHandleGroupVec<
        vtkm::cont::ArrayHandleGroupVec<ContObjectType,2>, NumOutputPerInput>;

  using ExecObjectType =
      typename GroupedArrayType::template ExecutionTypes<Device>::Portal;

  template<typename InputDomainType>
  VTKM_CONT
  ExecObjectType operator()(const ContObjectType &object,
                            const InputDomainType &,
                            vtkm::Id size) const
  {
    GroupedArrayType groupedArray(
          vtkm::cont::make_ArrayHandleGroupVec<2>(object));
    return groupedArray.PrepareForOutput(size, Device());
  }
};

}
}
} // namespace vtkm::cont::arg
////
//// END-EXAMPLE TransportImpl2.h
////

////
//// BEGIN-EXAMPLE ThreadIndicesLineFractal.h
////
namespace vtkm {
namespace exec {
namespace arg {

class ThreadIndicesLineFractal : public vtkm::exec::arg::ThreadIndicesBasic
{
  using Superclass = vtkm::exec::arg::ThreadIndicesBasic;

public:
  using CoordinateType = vtkm::Vec<vtkm::FloatDefault,2>;

  VTKM_SUPPRESS_EXEC_WARNINGS
  template<typename OutToInPortalType,
           typename VisitPortalType,
           typename InputPointPortal>
  VTKM_EXEC
  ThreadIndicesLineFractal(vtkm::Id threadIndex,
                           const OutToInPortalType &outToIn,
                           const VisitPortalType &visit,
                           const InputPointPortal &inputPoints,
                           vtkm::Id globalThreadIndexOffset=0)
    : Superclass(threadIndex,
                 outToIn.Get(threadIndex),
                 visit.Get(threadIndex),
                 globalThreadIndexOffset)
  {
    this->Point0 = inputPoints.Get(this->GetInputIndex())[0];
    this->Point1 = inputPoints.Get(this->GetInputIndex())[1];
  }

  VTKM_EXEC
  const CoordinateType &GetPoint0() const
  {
    return this->Point0;
  }

  VTKM_EXEC
  const CoordinateType &GetPoint1() const
  {
    return this->Point1;
  }

private:
  CoordinateType Point0;
  CoordinateType Point1;
};

}
}
} // namespace vtkm::exec::arg
////
//// END-EXAMPLE ThreadIndicesLineFractal.h
////

////
//// BEGIN-EXAMPLE LineFractalTransform.h
////
namespace vtkm {
namespace exec {

class LineFractalTransform
{
  using VecType = vtkm::Vec<vtkm::FloatDefault,2>;

public:
  template<typename T>
  VTKM_EXEC
  LineFractalTransform(const vtkm::Vec<T,2> &point0,
                       const vtkm::Vec<T,2> &point1)
  {
    this->Offset = point0;
    this->UAxis = point1 - point0;
    this->VAxis = vtkm::make_Vec(-this->UAxis[1], this->UAxis[0]);
  }

  template<typename T>
  VTKM_EXEC
  vtkm::Vec<T,2> operator()(const vtkm::Vec<T,2> &ppoint) const
  {
    VecType ppointCast(ppoint);
    VecType transform =
        ppointCast[0]*this->UAxis + ppointCast[1]*this->VAxis + this->Offset;
    return vtkm::Vec<T,2>(transform);
  }

  template<typename T>
  VTKM_EXEC
  vtkm::Vec<T,2> operator()(T x, T y) const
  {
    return (*this)(vtkm::Vec<T,2>(x,y));
  }

private:
  VecType Offset;
  VecType UAxis;
  VecType VAxis;
};

}
} // namespace vtkm::exec
////
//// END-EXAMPLE LineFractalTransform.h
////

////
//// BEGIN-EXAMPLE InputDomainFetch.h
////
namespace vtkm {
namespace exec {
namespace arg {

struct AspectTagLineFractalTransform {  };

template<typename FetchTag, typename ExecObjectType>
struct Fetch<
    FetchTag,
    vtkm::exec::arg::AspectTagLineFractalTransform,
    vtkm::exec::arg::ThreadIndicesLineFractal,
    ExecObjectType>
{
  using ValueType = LineFractalTransform;

  VTKM_SUPPRESS_EXEC_WARNINGS
  VTKM_EXEC
  ValueType Load(const vtkm::exec::arg::ThreadIndicesLineFractal &indices,
                 const ExecObjectType &) const
  {
    return ValueType(indices.GetPoint0(), indices.GetPoint1());
  }

  VTKM_EXEC
  void Store(const vtkm::exec::arg::ThreadIndicesLineFractal &,
             const ExecObjectType &,
             const ValueType &) const
  {
    // Store is a no-op for this fetch.
  }
};

}
}
} // namespace vtkm::exec::arg
////
//// END-EXAMPLE InputDomainFetch.h
////

////
//// BEGIN-EXAMPLE WorkletLineFractal.h
////
namespace vtkm {
namespace worklet {

class WorkletLineFractal : public vtkm::worklet::internal::WorkletBase
{
public:
  /// Control signature tag for line segments in the plane. Used as the input
  /// domain.
  ///
  struct SegmentsIn : vtkm::cont::arg::ControlSignatureTagBase
  {
    using TypeCheckTag = vtkm::cont::arg::TypeCheckTag2DCoordinates;
    using TransportTag = vtkm::cont::arg::TransportTag2DLineSegmentsIn;
    using FetchTag = vtkm::exec::arg::FetchTag2DLineSegmentsIn;
  };

  /// Control signature tag for a group of output line segments. The template
  /// argument specifies how many line segments are outputted for each input.
  /// The type is a Vec-like (of size NumSegments) of Vec-2's.
  ///
  template<vtkm::IdComponent NumSegments>
  struct SegmentsOut : vtkm::cont::arg::ControlSignatureTagBase
  {
    using TypeCheckTag = vtkm::cont::arg::TypeCheckTag2DCoordinates;
    using TransportTag =
        vtkm::cont::arg::TransportTag2DLineSegmentsOut<NumSegments>;
    using FetchTag = vtkm::exec::arg::FetchTagArrayDirectOut;
  };

  /// Execution signature tag for a LineFractalTransform from the input.
  ///
  struct Transform : vtkm::exec::arg::ExecutionSignatureTagBase
  {
    static const vtkm::IdComponent INDEX = 1;
    using AspectTag = vtkm::exec::arg::AspectTagLineFractalTransform;
  };

  VTKM_SUPPRESS_EXEC_WARNINGS
  template<typename T,
           typename OutToInPortalType,
           typename VisitPortalType,
           typename InputDomainType>
  VTKM_EXEC
  vtkm::exec::arg::ThreadIndicesLineFractal
  GetThreadIndices(const T &threadIndex,
                   const OutToInPortalType &outToIn,
                   const VisitPortalType &visit,
                   const InputDomainType &inputPoints,
                   vtkm::Id globalThreadIndexOffset) const
  {
    return vtkm::exec::arg::ThreadIndicesLineFractal(
          threadIndex,
          outToIn,
          visit,
          inputPoints,
          globalThreadIndexOffset);
  }
};

}
} // namespace vtkm::worklet
////
//// END-EXAMPLE WorkletLineFractal.h
////

////
//// BEGIN-EXAMPLE DispatcherLineFractal.h
////
namespace vtkm {
namespace worklet {

template<typename WorkletType,
         typename Device = VTKM_DEFAULT_DEVICE_ADAPTER_TAG>
class DispatcherLineFractal :
    public vtkm::worklet::internal::DispatcherBase<
      DispatcherLineFractal<WorkletType, Device>,
      WorkletType,
      vtkm::worklet::WorkletLineFractal
      >
{
  using Superclass =
      vtkm::worklet::internal::DispatcherBase<
        DispatcherLineFractal<WorkletType, Device>,
        WorkletType,
        vtkm::worklet::WorkletLineFractal
        >;

public:
  VTKM_CONT
  DispatcherLineFractal(const WorkletType &worklet = WorkletType())
    : Superclass(worklet)
  {  }

  template<typename Invocation>
  VTKM_CONT
  void DoInvoke(const Invocation &invocation) const
  {
    // This is the type for the input domain
    using InputDomainType = typename Invocation::InputDomainType;

    // If you get a compile error on this line, then you have tried to use
    // something that is not a vtkm::cont::ArrayHandle as the input domain to a
    // topology operation (that operates on a cell set connection domain).
    VTKM_IS_ARRAY_HANDLE(InputDomainType);

    // Get the control signature tag for the input domain.
    using InputDomainTag = typename Invocation::InputDomainTag;

    // If you get a compile error on this line, then you have set the input
    // domain to something that is not a SegmentsIn parameter, which is not
    // valid.
    VTKM_STATIC_ASSERT((std::is_same<
                          InputDomainTag,
                          vtkm::worklet::WorkletLineFractal::SegmentsIn
                          >::value));

    // We can pull the input domain parameter (the data specifying the input
    // domain) from the invocation object.
    const InputDomainType &inputDomain = invocation.GetInputDomain();

    // Now that we have the input domain, we can extract the range of the
    // scheduling and call BadicInvoke.
    this->BasicInvoke(invocation,
                      inputDomain.GetNumberOfValues()/2,
                      Device());
  }
};

}
} // namespace vtkm::worklet
////
//// END-EXAMPLE DispatcherLineFractal.h
////

////
//// BEGIN-EXAMPLE KochSnowflake.cxx
////
struct KochSnowflake
{
  struct FractalWorklet : vtkm::worklet::WorkletLineFractal
  {
    typedef void ControlSignature(SegmentsIn, SegmentsOut<4>);
    typedef void ExecutionSignature(Transform, _2);
    using InputDomain = _1;

    template<typename SegmentsOutVecType>
    void operator()(const vtkm::exec::LineFractalTransform &transform,
                    SegmentsOutVecType &segmentsOutVec) const
    {
      segmentsOutVec[0][0] = transform(0.00f, 0.00f);
      segmentsOutVec[0][1] = transform(0.33f, 0.00f);

      segmentsOutVec[1][0] = transform(0.33f, 0.00f);
      segmentsOutVec[1][1] = transform(0.50f, 0.29f);

      segmentsOutVec[2][0] = transform(0.50f, 0.29f);
      segmentsOutVec[2][1] = transform(0.67f, 0.00f);

      segmentsOutVec[3][0] = transform(0.67f, 0.00f);
      segmentsOutVec[3][1] = transform(1.00f, 0.00f);
    }
  };

  VTKM_CONT
  static vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault,2> >
  Run(vtkm::IdComponent numIterations)
  {
    using VecType = vtkm::Vec<vtkm::Float32,2>;

    vtkm::cont::ArrayHandle<VecType> points;

    // Initialize points array with a single line
    points.Allocate(2);
    points.GetPortalControl().Set(0, VecType(0.0f, 0.0f));
    points.GetPortalControl().Set(1, VecType(1.0f, 0.0f));

    vtkm::worklet::DispatcherLineFractal<KochSnowflake::FractalWorklet>
        dispatcher;

    for (vtkm::IdComponent i = 0; i < numIterations; ++i)
    {
      vtkm::cont::ArrayHandle<VecType> outPoints;
      dispatcher.Invoke(points, outPoints);
      points = outPoints;
    }

    return points;
  }
};
////
//// END-EXAMPLE KochSnowflake.cxx
////

static void TryKoch()
{
  // Demonstrate a single line.
  using VecType = vtkm::Vec<vtkm::Float32,2>;
  vtkm::cont::ArrayHandle<VecType> points;

  points = KochSnowflake::Run(1);
  WriteSVG("Koch1.svg", points);

  for (vtkm::Id index = 0; index < points.GetNumberOfValues()/2; ++index)
  {
    std::cout << index << ": "
              << points.GetPortalConstControl().Get(index*2+0) << " "
              << points.GetPortalConstControl().Get(index*2+1) << std::endl;
  }

  points = KochSnowflake::Run(2);
  WriteSVG("Koch2.svg", points);

  for (vtkm::Id index = 0; index < points.GetNumberOfValues()/2; ++index)
  {
    std::cout << index << ": "
              << points.GetPortalConstControl().Get(index*2+0) << " "
              << points.GetPortalConstControl().Get(index*2+1) << std::endl;
  }

  points = KochSnowflake::Run(5);
  WriteSVG("Koch5.svg", points, 0.1);
}

////
//// BEGIN-EXAMPLE DragonFractal.cxx
////
struct DragonFractal
{
  struct FractalWorklet : vtkm::worklet::WorkletLineFractal
  {
    typedef void ControlSignature(SegmentsIn, SegmentsOut<2>);
    typedef void ExecutionSignature(Transform, _2);
    using InputDomain = _1;

    template<typename SegmentsOutVecType>
    void operator()(const vtkm::exec::LineFractalTransform &transform,
                    SegmentsOutVecType &segmentsOutVec) const
    {
      segmentsOutVec[0][0] = transform(0.5f, 0.5f);
      segmentsOutVec[0][1] = transform(0.0f, 0.0f);

      segmentsOutVec[1][0] = transform(0.5f, 0.5f);
      segmentsOutVec[1][1] = transform(1.0f, 0.0f);
    }
  };

  VTKM_CONT
  static vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault,2> >
  Run(vtkm::IdComponent numIterations)
  {
    using VecType = vtkm::Vec<vtkm::Float32,2>;

    vtkm::cont::ArrayHandle<VecType> points;

    // Initialize points array with a single line
    points.Allocate(2);
    points.GetPortalControl().Set(0, VecType(0.0f, 0.0f));
    points.GetPortalControl().Set(1, VecType(1.0f, 0.0f));

    vtkm::worklet::DispatcherLineFractal<DragonFractal::FractalWorklet>
        dispatcher;

    for (vtkm::IdComponent i = 0; i < numIterations; ++i)
    {
      vtkm::cont::ArrayHandle<VecType> outPoints;
      dispatcher.Invoke(points, outPoints);
      points = outPoints;
    }

    return points;
  }
};
////
//// END-EXAMPLE DragonFractal.cxx
////

static void TryDragon()
{
  // Demonstrate a single line.
  using VecType = vtkm::Vec<vtkm::Float32,2>;
  vtkm::cont::ArrayHandle<VecType> points;

  for (vtkm::IdComponent numIterations = 1;
       numIterations <= 13;
       ++numIterations)
  {
    points = DragonFractal::Run(numIterations);
    char filename[FILENAME_MAX];
    sprintf(filename, "Dragon%02d.svg", numIterations);
    WriteSVG(filename, points, 2.0/numIterations);
  }
}

////
//// BEGIN-EXAMPLE HilbertCurve.cxx
////
struct HilbertCurve
{
  struct FractalWorklet : vtkm::worklet::WorkletLineFractal
  {
    typedef void ControlSignature(SegmentsIn, SegmentsOut<4>);
    typedef void ExecutionSignature(Transform, _2);
    using InputDomain = _1;

    template<typename SegmentsOutVecType>
    void operator()(const vtkm::exec::LineFractalTransform &transform,
                    SegmentsOutVecType &segmentsOutVec) const
    {
      segmentsOutVec[0][0] = transform(0.0f, 0.5f);
      segmentsOutVec[0][1] = transform(0.0f, 0.0f);

      segmentsOutVec[1][0] = transform(0.0f, 0.5f);
      segmentsOutVec[1][1] = transform(0.5f, 0.5f);

      segmentsOutVec[2][0] = transform(0.5f, 0.5f);
      segmentsOutVec[2][1] = transform(1.0f, 0.5f);

      segmentsOutVec[3][0] = transform(1.0f, 0.0f);
      segmentsOutVec[3][1] = transform(1.0f, 0.5f);
    }
  };

  VTKM_CONT
  static vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault,2> >
  Run(vtkm::IdComponent numIterations)
  {
    using VecType = vtkm::Vec<vtkm::Float32,2>;

    vtkm::cont::ArrayHandle<VecType> points;

    // Initialize points array with a single line
    points.Allocate(2);
    points.GetPortalControl().Set(0, VecType(0.0f, 0.0f));
    points.GetPortalControl().Set(1, VecType(1.0f, 0.0f));

    vtkm::worklet::DispatcherLineFractal<HilbertCurve::FractalWorklet>
        dispatcher;

    for (vtkm::IdComponent i = 0; i < numIterations; ++i)
    {
      vtkm::cont::ArrayHandle<VecType> outPoints;
      dispatcher.Invoke(points, outPoints);
      points = outPoints;
    }

    return points;
  }
};
////
//// END-EXAMPLE HilbertCurve.cxx
////

static void TryHilbert()
{
  // Demonstrate a single line.
  using VecType = vtkm::Vec<vtkm::Float32,2>;
  vtkm::cont::ArrayHandle<VecType> points;

  for (vtkm::IdComponent numIterations = 1;
       numIterations <= 6;
       ++numIterations)
  {
    points = HilbertCurve::Run(numIterations);
    char filename[FILENAME_MAX];
    sprintf(filename, "Hilbert%02d.svg", numIterations);
    WriteSVG(filename, points, 2.0/numIterations);
  }
}

////
//// BEGIN-EXAMPLE TreeFractal.cxx
////
struct TreeFractal
{
  struct FractalWorklet : vtkm::worklet::WorkletLineFractal
  {
    typedef void ControlSignature(SegmentsIn, SegmentsOut<3>);
    typedef void ExecutionSignature(Transform, _2);
    using InputDomain = _1;

    template<typename SegmentsOutVecType>
    void operator()(const vtkm::exec::LineFractalTransform &transform,
                    SegmentsOutVecType &segmentsOutVec) const
    {
      segmentsOutVec[0][0] = transform(0.0f, 0.0f);
      segmentsOutVec[0][1] = transform(1.0f, 0.0f);

      segmentsOutVec[1][0] = transform(1.0f, 0.0f);
      segmentsOutVec[1][1] = transform(1.5f, -0.25f);

      segmentsOutVec[2][0] = transform(1.0f, 0.0f);
      segmentsOutVec[2][1] = transform(1.5f, 0.35f);
    }
  };

  VTKM_CONT
  static vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault,2> >
  Run(vtkm::IdComponent numIterations)
  {
    using VecType = vtkm::Vec<vtkm::Float32,2>;

    vtkm::cont::ArrayHandle<VecType> points;

    // Initialize points array with a single line
    points.Allocate(2);
    points.GetPortalControl().Set(0, VecType(0.0f, 0.0f));
    points.GetPortalControl().Set(1, VecType(0.0f, 1.0f));

    vtkm::worklet::DispatcherLineFractal<TreeFractal::FractalWorklet>
        dispatcher;

    for (vtkm::IdComponent i = 0; i < numIterations; ++i)
    {
      vtkm::cont::ArrayHandle<VecType> outPoints;
      dispatcher.Invoke(points, outPoints);
      points = outPoints;
    }

    return points;
  }
};
////
//// END-EXAMPLE TreeFractal.cxx
////

static void TryTree()
{
  // Demonstrate a single line.
  using VecType = vtkm::Vec<vtkm::Float32,2>;
  vtkm::cont::ArrayHandle<VecType> points;

  for (vtkm::IdComponent numIterations = 1;
       numIterations <= 8;
       ++numIterations)
  {
    points = TreeFractal::Run(numIterations);
    char filename[FILENAME_MAX];
    sprintf(filename, "Tree%02d.svg", numIterations);
    WriteSVG(filename, points, 2.0/numIterations);
  }
}

static void RunTests()
{
  TryVecLineSegments();
  TryKoch();
  TryDragon();
  TryHilbert();
  TryTree();
}

int FractalWorklets(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(RunTests);
}
