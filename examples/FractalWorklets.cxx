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
#include <vtkm/worklet/ScatterCounting.h>
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
  return vtkm::Vec<T,2>(static_cast<T>(point[0] - xRange.Min + padding),
                        static_cast<T>(yRange.Max - point[1] + padding));
}

template<typename T>
static void WriteSVG(const std::string &filename,
                     const vtkm::cont::ArrayHandle<vtkm::Vec<T,2> > &data,
                     float width = 2.0,
                     const std::string &color = "black")
{
  static const float PADDING = 0.05f;

  vtkm::cont::ArrayHandle<vtkm::Range> ranges =
      vtkm::cont::ArrayRangeCompute(data);
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
         << "\" stroke-width=\"" << width
         << "\" stroke-linecap=\"round\" />\n";
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
                            vtkm::Id inputRange,
                            vtkm::Id) const
  {
    if (object.GetNumberOfValues() != inputRange*2)
    {
      throw vtkm::cont::ErrorBadValue(
            "2D line segment array size does not agree with input size.");
    }

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
                            vtkm::Id,
                            vtkm::Id outputRange) const
  {
    GroupedArrayType groupedArray(
          vtkm::cont::make_ArrayHandleGroupVec<2>(object));
    return groupedArray.PrepareForOutput(outputRange, Device());
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
  ////
  //// BEGIN-EXAMPLE WorkletLineFractalInputDomainTag.cxx
  ////
  struct SegmentsIn : vtkm::cont::arg::ControlSignatureTagBase
  {
    using TypeCheckTag = vtkm::cont::arg::TypeCheckTag2DCoordinates;
    using TransportTag = vtkm::cont::arg::TransportTag2DLineSegmentsIn;
    using FetchTag = vtkm::exec::arg::FetchTag2DLineSegmentsIn;
  };
  ////
  //// END-EXAMPLE WorkletLineFractalInputDomainTag.cxx
  ////

  /// Control signature tag for a group of output line segments. The template
  /// argument specifies how many line segments are outputted for each input.
  /// The type is a Vec-like (of size NumSegments) of Vec-2's.
  ///
  ////
  //// BEGIN-EXAMPLE WorkletLineFractalOutputTag.cxx
  ////
  template<vtkm::IdComponent NumSegments>
  struct SegmentsOut : vtkm::cont::arg::ControlSignatureTagBase
  {
    using TypeCheckTag = vtkm::cont::arg::TypeCheckTag2DCoordinates;
    using TransportTag =
        vtkm::cont::arg::TransportTag2DLineSegmentsOut<NumSegments>;
    using FetchTag = vtkm::exec::arg::FetchTagArrayDirectOut;
  };
  ////
  //// END-EXAMPLE WorkletLineFractalOutputTag.cxx
  ////

  /// Control signature tag for input fields. There is one entry per input line
  /// segment. This tag takes a template argument that is a type list tag that
  /// limits the possible value types in the array.
  ///
  ////
  //// BEGIN-EXAMPLE WorkletLineFractalFieldInTag.cxx
  ////
  template<typename TypeList = AllTypes>
  struct FieldIn : vtkm::cont::arg::ControlSignatureTagBase {
    using TypeCheckTag = vtkm::cont::arg::TypeCheckTagArray<TypeList>;
    using TransportTag = vtkm::cont::arg::TransportTagArrayIn;
    using FetchTag = vtkm::exec::arg::FetchTagArrayDirectIn;
  };
  ////
  //// END-EXAMPLE WorkletLineFractalFieldInTag.cxx
  ////

  /// Control signature tag for input fields. There is one entry per input line
  /// segment. This tag takes a template argument that is a type list tag that
  /// limits the possible value types in the array.
  ///
  template<typename TypeList = AllTypes>
  struct FieldOut : vtkm::cont::arg::ControlSignatureTagBase {
    using TypeCheckTag = vtkm::cont::arg::TypeCheckTagArray<TypeList>;
    using TransportTag = vtkm::cont::arg::TransportTagArrayOut;
    using FetchTag = vtkm::exec::arg::FetchTagArrayDirectOut;
  };

  /// Execution signature tag for a LineFractalTransform from the input.
  ///
  ////
  //// BEGIN-EXAMPLE WorkletLineFractalTransformTag.cxx
  ////
  struct Transform : vtkm::exec::arg::ExecutionSignatureTagBase
  {
    static const vtkm::IdComponent INDEX = 1;
    using AspectTag = vtkm::exec::arg::AspectTagLineFractalTransform;
  };
  ////
  //// END-EXAMPLE WorkletLineFractalTransformTag.cxx
  ////

  ////
  //// BEGIN-EXAMPLE GetThreadIndices.cxx
  ////
  VTKM_SUPPRESS_EXEC_WARNINGS
  template<typename OutToInPortalType,
           typename VisitPortalType,
           typename InputDomainType>
  VTKM_EXEC
  vtkm::exec::arg::ThreadIndicesLineFractal
  GetThreadIndices(vtkm::Id threadIndex,
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
  ////
  //// END-EXAMPLE GetThreadIndices.cxx
  ////
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

////
//// BEGIN-EXAMPLE DispatcherSuperclass.cxx
////
////
//// BEGIN-EXAMPLE DispatcherTemplate.cxx
////
template<typename WorkletType,
         typename Device = VTKM_DEFAULT_DEVICE_ADAPTER_TAG>
class DispatcherLineFractal
////
//// END-EXAMPLE DispatcherTemplate.cxx
////
    : public vtkm::worklet::internal::DispatcherBase<
        DispatcherLineFractal<WorkletType, Device>,
        WorkletType,
        vtkm::worklet::WorkletLineFractal
        >
////
//// END-EXAMPLE DispatcherSuperclass.cxx
////
{
  using Superclass =
      vtkm::worklet::internal::DispatcherBase<
        DispatcherLineFractal<WorkletType, Device>,
        WorkletType,
        vtkm::worklet::WorkletLineFractal
        >;

public:
  ////
  //// BEGIN-EXAMPLE DispatcherConstructor.cxx
  ////
  VTKM_CONT
  DispatcherLineFractal(const WorkletType &worklet = WorkletType())
    : Superclass(worklet)
  {  }
  ////
  //// END-EXAMPLE DispatcherConstructor.cxx
  ////

  ////
  //// BEGIN-EXAMPLE DispatcherDoInvokePrototype.cxx
  ////
  template<typename Invocation>
  VTKM_CONT
  void DoInvoke(const Invocation &invocation) const
  ////
  //// END-EXAMPLE DispatcherDoInvokePrototype.cxx
  ////
  {
    ////
    //// BEGIN-EXAMPLE CheckInputDomainType.cxx
    ////
    // Get the control signature tag for the input domain.
    using InputDomainTag = typename Invocation::InputDomainTag;

    // If you get a compile error on this line, then you have set the input
    // domain to something that is not a SegmentsIn parameter, which is not
    // valid.
    VTKM_STATIC_ASSERT((std::is_same<
                          InputDomainTag,
                          vtkm::worklet::WorkletLineFractal::SegmentsIn
                          >::value));

    // This is the type for the input domain
    using InputDomainType = typename Invocation::InputDomainType;

    // If you get a compile error on this line, then you have tried to use
    // something that is not a vtkm::cont::ArrayHandle as the input domain to a
    // topology operation (that operates on a cell set connection domain).
    VTKM_IS_ARRAY_HANDLE(InputDomainType);
    ////
    //// END-EXAMPLE CheckInputDomainType.cxx
    ////

    ////
    //// BEGIN-EXAMPLE CallBasicInvoke.cxx
    ////
    // We can pull the input domain parameter (the data specifying the input
    // domain) from the invocation object.
    const InputDomainType &inputDomain = invocation.GetInputDomain();

    // Now that we have the input domain, we can extract the range of the
    // scheduling and call BasicInvoke.
    this->BasicInvoke(invocation,
                      inputDomain.GetNumberOfValues()/2,
                      Device());
    ////
    //// END-EXAMPLE CallBasicInvoke.cxx
    ////
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

  template<typename Device>
  VTKM_CONT
  static vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault,2> >
  Run(vtkm::IdComponent numIterations, Device)
  {
    using VecType = vtkm::Vec<vtkm::Float32,2>;

    vtkm::cont::ArrayHandle<VecType> points;

    // Initialize points array with a single line
    points.Allocate(2);
    points.GetPortalControl().Set(0, VecType(0.0f, 0.0f));
    points.GetPortalControl().Set(1, VecType(1.0f, 0.0f));

    vtkm::worklet::DispatcherLineFractal<KochSnowflake::FractalWorklet, Device>
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

  points = KochSnowflake::Run(1, VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
  WriteSVG("Koch1.svg", points);

  for (vtkm::Id index = 0; index < points.GetNumberOfValues()/2; ++index)
  {
    std::cout << index << ": "
              << points.GetPortalConstControl().Get(index*2+0) << " "
              << points.GetPortalConstControl().Get(index*2+1) << std::endl;
  }

  points = KochSnowflake::Run(2, VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
  WriteSVG("Koch2.svg", points);

  for (vtkm::Id index = 0; index < points.GetNumberOfValues()/2; ++index)
  {
    std::cout << index << ": "
              << points.GetPortalConstControl().Get(index*2+0) << " "
              << points.GetPortalConstControl().Get(index*2+1) << std::endl;
  }

  points = KochSnowflake::Run(5, VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
  WriteSVG("Koch5.svg", points, 0.1f);
}

////
//// BEGIN-EXAMPLE QuadraticType2.cxx
////
struct QuadraticType2
{
  struct FractalWorklet : vtkm::worklet::WorkletLineFractal
  {
    typedef void ControlSignature(SegmentsIn, SegmentsOut<8>);
    typedef void ExecutionSignature(Transform, _2);
    using InputDomain = _1;

    template<typename SegmentsOutVecType>
    void operator()(const vtkm::exec::LineFractalTransform &transform,
                    SegmentsOutVecType &segmentsOutVec) const
    {
      segmentsOutVec[0][0] = transform(0.00f,  0.00f);
      segmentsOutVec[0][1] = transform(0.25f,  0.00f);

      segmentsOutVec[1][0] = transform(0.25f,  0.00f);
      segmentsOutVec[1][1] = transform(0.25f,  0.25f);

      segmentsOutVec[2][0] = transform(0.25f,  0.25f);
      segmentsOutVec[2][1] = transform(0.50f,  0.25f);

      segmentsOutVec[3][0] = transform(0.50f,  0.25f);
      segmentsOutVec[3][1] = transform(0.50f,  0.00f);

      segmentsOutVec[4][0] = transform(0.50f,  0.00f);
      segmentsOutVec[4][1] = transform(0.50f, -0.25f);

      segmentsOutVec[5][0] = transform(0.50f, -0.25f);
      segmentsOutVec[5][1] = transform(0.75f, -0.25f);

      segmentsOutVec[6][0] = transform(0.75f, -0.25f);
      segmentsOutVec[6][1] = transform(0.75f,  0.00f);

      segmentsOutVec[7][0] = transform(0.75f,  0.00f);
      segmentsOutVec[7][1] = transform(1.00f,  0.00f);
    }
  };

  template<typename Device>
  VTKM_CONT
  static vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault,2> >
  Run(vtkm::IdComponent numIterations, Device)
  {
    using VecType = vtkm::Vec<vtkm::Float32,2>;

    vtkm::cont::ArrayHandle<VecType> points;

    // Initialize points array with a single line
    points.Allocate(2);
    points.GetPortalControl().Set(0, VecType(0.0f, 0.0f));
    points.GetPortalControl().Set(1, VecType(1.0f, 0.0f));

    vtkm::worklet::DispatcherLineFractal<QuadraticType2::FractalWorklet, Device>
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
//// END-EXAMPLE QuadraticType2.cxx
////

static void TryQuadraticType2()
{
  // Demonstrate a single line.
  using VecType = vtkm::Vec<vtkm::Float32,2>;
  vtkm::cont::ArrayHandle<VecType> points;

  points = QuadraticType2::Run(1, VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
  WriteSVG("QuadraticType2_1.svg", points);

  for (vtkm::Id index = 0; index < points.GetNumberOfValues()/2; ++index)
  {
    std::cout << index << ": "
              << points.GetPortalConstControl().Get(index*2+0) << " "
              << points.GetPortalConstControl().Get(index*2+1) << std::endl;
  }

  points = QuadraticType2::Run(2, VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
  WriteSVG("QuadraticType2_2.svg", points);

  for (vtkm::Id index = 0; index < points.GetNumberOfValues()/2; ++index)
  {
    std::cout << index << ": "
              << points.GetPortalConstControl().Get(index*2+0) << " "
              << points.GetPortalConstControl().Get(index*2+1) << std::endl;
  }

  points = QuadraticType2::Run(4, VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
  WriteSVG("QuadraticType2_4.svg", points, 0.1f);
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

  template<typename Device>
  VTKM_CONT
  static vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault,2> >
  Run(vtkm::IdComponent numIterations, Device)
  {
    using VecType = vtkm::Vec<vtkm::Float32,2>;

    vtkm::cont::ArrayHandle<VecType> points;

    // Initialize points array with a single line
    points.Allocate(2);
    points.GetPortalControl().Set(0, VecType(0.0f, 0.0f));
    points.GetPortalControl().Set(1, VecType(1.0f, 0.0f));

    vtkm::worklet::DispatcherLineFractal<DragonFractal::FractalWorklet, Device>
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
    points = DragonFractal::Run(numIterations,
                                VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
    char filename[FILENAME_MAX];
    sprintf(filename, "Dragon%02d.svg", numIterations);
    WriteSVG(filename, points, 2.0f/numIterations);
  }
}

////
//// BEGIN-EXAMPLE HilbertCurve.cxx
////
struct HilbertCurve
{
  struct FractalWorklet : vtkm::worklet::WorkletLineFractal
  {
    typedef void ControlSignature(SegmentsIn,
                                  FieldIn<> directionIn,
                                  SegmentsOut<4>,
                                  FieldOut<> directionOut);
    typedef void ExecutionSignature(Transform, _2, _3, _4);
    using InputDomain = _1;

    template<typename SegmentsOutVecType>
    void operator()(const vtkm::exec::LineFractalTransform &transform,
                    vtkm::Int8 directionIn,
                    SegmentsOutVecType &segmentsOutVec,
                    vtkm::Vec<vtkm::Int8,4> &directionOut) const
    {
      segmentsOutVec[0][0] = transform(0.0f, directionIn*0.0f);
      segmentsOutVec[0][1] = transform(0.0f, directionIn*0.5f);
      directionOut[0] = -directionIn;

      segmentsOutVec[1][0] = transform(0.0f, directionIn*0.5f);
      segmentsOutVec[1][1] = transform(0.5f, directionIn*0.5f);
      directionOut[1] = directionIn;

      segmentsOutVec[2][0] = transform(0.5f, directionIn*0.5f);
      segmentsOutVec[2][1] = transform(1.0f, directionIn*0.5f);
      directionOut[2] = directionIn;

      segmentsOutVec[3][0] = transform(1.0f, directionIn*0.5f);
      segmentsOutVec[3][1] = transform(1.0f, directionIn*0.0f);
      directionOut[3] = -directionIn;
    }
  };

  template<typename Device>
  VTKM_CONT
  static vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault,2> >
  Run(vtkm::IdComponent numIterations, Device)
  {
    using VecType = vtkm::Vec<vtkm::Float32,2>;

    vtkm::cont::ArrayHandle<VecType> points;

    // Initialize points array with a single line
    points.Allocate(2);
    points.GetPortalControl().Set(0, VecType(0.0f, 0.0f));
    points.GetPortalControl().Set(1, VecType(1.0f, 0.0f));

    vtkm::cont::ArrayHandle<vtkm::Int8> directions;

    // Initialize direction with positive.
    directions.Allocate(1);
    directions.GetPortalControl().Set(0, 1);

    vtkm::worklet::DispatcherLineFractal<HilbertCurve::FractalWorklet, Device>
        dispatcher;

    for (vtkm::IdComponent i = 0; i < numIterations; ++i)
    {
      vtkm::cont::ArrayHandle<VecType> outPoints;
      vtkm::cont::ArrayHandle<vtkm::Int8> outDirections;
      dispatcher.Invoke(points,
                        directions,
                        outPoints,
                        vtkm::cont::make_ArrayHandleGroupVec<4>(outDirections));
      points = outPoints;
      directions = outDirections;
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
    points = HilbertCurve::Run(numIterations,
                               VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
    char filename[FILENAME_MAX];
    sprintf(filename, "Hilbert%02d.svg", numIterations);
    WriteSVG(filename, points, 2.0f/numIterations);
  }
}

////
//// BEGIN-EXAMPLE TreeFractal.cxx
////
struct TreeFractal
{
  struct FractalWorklet : vtkm::worklet::WorkletLineFractal
  {
    typedef void ControlSignature(SegmentsIn,
                                  SegmentsOut<1>,
                                  FieldOut<> countNextIteration);
    typedef void ExecutionSignature(Transform, VisitIndex, _2, _3);
    using InputDomain = _1;

    using ScatterType = vtkm::worklet::ScatterCounting;
    VTKM_CONT
    ScatterType GetScatter() const { return this->Scatter; }

    template<typename Storage, typename Device>
    VTKM_CONT
    FractalWorklet(
        const vtkm::cont::ArrayHandle<vtkm::IdComponent,Storage> &count,
        Device)
      : Scatter(count, Device())
    {  }

    template<typename SegmentsOutVecType>
    void operator()(const vtkm::exec::LineFractalTransform &transform,
                    vtkm::IdComponent visitIndex,
                    SegmentsOutVecType &segmentsOutVec,
                    vtkm::IdComponent &countNextIteration) const
    {
      switch (visitIndex)
      {
        case 0:
          segmentsOutVec[0][0] = transform(0.0f, 0.0f);
          segmentsOutVec[0][1] = transform(1.0f, 0.0f);
          countNextIteration = 1;
          break;
        case 1:
          segmentsOutVec[0][0] = transform(1.0f, 0.0f);
          segmentsOutVec[0][1] = transform(1.5f, -0.25f);
          countNextIteration = 3;
          break;
        case 2:
          segmentsOutVec[0][0] = transform(1.0f, 0.0f);
          segmentsOutVec[0][1] = transform(1.5f, 0.35f);
          countNextIteration = 3;
          break;
        default:
          this->RaiseError("Unexpected visit index.");
      }
    }

  private:
    ScatterType Scatter;
  };

  template<typename Device>
  VTKM_CONT
  static vtkm::cont::ArrayHandle<vtkm::Vec<vtkm::FloatDefault,2> >
  Run(vtkm::IdComponent numIterations, Device)
  {
    using VecType = vtkm::Vec<vtkm::Float32,2>;

    vtkm::cont::ArrayHandle<VecType> points;

    // Initialize points array with a single line
    points.Allocate(2);
    points.GetPortalControl().Set(0, VecType(0.0f, 0.0f));
    points.GetPortalControl().Set(1, VecType(0.0f, 1.0f));

    vtkm::cont::ArrayHandle<vtkm::IdComponent> count;

    // Initialize count array with 3 (meaning iterate)
    count.Allocate(1);
    count.GetPortalControl().Set(0, 3);

    for (vtkm::IdComponent i = 0; i < numIterations; ++i)
    {
      vtkm::worklet::DispatcherLineFractal<TreeFractal::FractalWorklet, Device>
          dispatcher(FractalWorklet(count, Device()));

      vtkm::cont::ArrayHandle<VecType> outPoints;
      dispatcher.Invoke(points, outPoints, count);
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
    points = TreeFractal::Run(numIterations, VTKM_DEFAULT_DEVICE_ADAPTER_TAG());
    char filename[FILENAME_MAX];
    sprintf(filename, "Tree%02d.svg", numIterations);
    WriteSVG(filename, points, 2.0f/numIterations);
  }
}

static void RunTests()
{
  TryVecLineSegments();
  TryKoch();
  TryQuadraticType2();
  TryDragon();
  TryHilbert();
  TryTree();
}

int FractalWorklets(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(RunTests);
}
