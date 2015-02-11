#include <vtkm/internal/FunctionInterface.h>

#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/DynamicArrayHandle.h>
#include <vtkm/cont/ErrorControlBadType.h>

#include <vtkm/cont/internal/DynamicTransform.h>

#include <vtkm/cont/testing/Testing.h>

#include <boost/static_assert.hpp>

#include <string.h>

namespace {

void BasicFunctionInterface()
{
  ////
  //// BEGIN-EXAMPLE DefineFunctionInterface.cxx
  ////
  // FunctionInterfaces matching some common POSIX functions.
  vtkm::internal::FunctionInterface<size_t(const char *)>
      strlenInterface;

  vtkm::internal::FunctionInterface<char *(char *, const char *s2, size_t)>
      strncpyInterface;
  ////
  //// END-EXAMPLE DefineFunctionInterface.cxx
  ////

  ////
  //// BEGIN-EXAMPLE UseMakeFunctionInterface.cxx
  ////
  const char *s = "Hello World";
  static const size_t BUFFER_SIZE = 100;
  char buffer[BUFFER_SIZE];

  strlenInterface =
      vtkm::internal::make_FunctionInterface<size_t>(s);

  strncpyInterface =
      vtkm::internal::make_FunctionInterface<char *>(buffer, s, BUFFER_SIZE);
  ////
  //// END-EXAMPLE UseMakeFunctionInterface.cxx
  ////

  std::cout << "Trying interfaces." << std::endl;

  strlenInterface.InvokeCont(strlen);
  VTKM_TEST_ASSERT(strlenInterface.GetReturnValue() == 11, "Bad length.");

  strncpyInterface.InvokeCont(strncpy);
  VTKM_TEST_ASSERT(strncpyInterface.GetReturnValue() == buffer,
                   "Bad return value.");
  VTKM_TEST_ASSERT(strncmp(s, buffer, BUFFER_SIZE) == 0, "Did not copy.");

  ////
  //// BEGIN-EXAMPLE FunctionInterfaceArity.cxx
  ////
  BOOST_STATIC_ASSERT(
        vtkm::internal::FunctionInterface<size_t(const char *)>::ARITY == 1);

  vtkm::IdComponent arity = strncpyInterface.GetArity();  // arity = 3
  ////
  //// END-EXAMPLE FunctionInterfaceArity.cxx
  ////

  VTKM_TEST_ASSERT(arity == 3, "Unexpected arity.");
}

////
//// BEGIN-EXAMPLE FunctionInterfaceGetParameter.cxx
////
void GetFirstParameterResolved(
    const vtkm::internal::FunctionInterface<void(const char *)> &interface)
{
  // The following two uses of GetParameter are equivalent
  std::cout << interface.GetParameter<1>() << std::endl;
  std::cout << interface.GetParameter(vtkm::internal::IndexTag<1>())
            << std::endl;
}

template<typename FunctionSignature>
void GetFirstParameterTemplated(
    const vtkm::internal::FunctionInterface<FunctionSignature> &interface)
{
  // The following two uses of GetParameter are equivalent
  std::cout << interface.template GetParameter<1>() << std::endl;
  std::cout << interface.GetParameter(vtkm::internal::IndexTag<1>())
            << std::endl;
}
////
//// END-EXAMPLE FunctionInterfaceGetParameter.cxx
////

void TryGetParameter()
{
  std::cout << "Getting parameters." << std::endl;

  GetFirstParameterResolved(vtkm::internal::make_FunctionInterface<void>("foo"));
  GetFirstParameterTemplated(vtkm::internal::make_FunctionInterface<void>("bar"));
}

////
//// BEGIN-EXAMPLE FunctionInterfaceSetParameter.cxx
////
void SetFirstParameterResolved(
    vtkm::internal::FunctionInterface<void(const char *)> &interface,
    const char *newFirstParameter)
{
  // The following two uses of SetParameter are equivalent
  interface.SetParameter<1>(newFirstParameter);
  interface.SetParameter(newFirstParameter, vtkm::internal::IndexTag<1>());
}

template<typename FunctionSignature, typename T>
void SetFirstParameterTemplated(
    vtkm::internal::FunctionInterface<FunctionSignature> &interface,
    T newFirstParameter)
{
  // The following two uses of SetParameter are equivalent
  interface.template SetParameter<1>(newFirstParameter);
  interface.SetParameter(newFirstParameter, vtkm::internal::IndexTag<1>());
}
////
//// END-EXAMPLE FunctionInterfaceSetParameter.cxx
////

void TrySetParameter()
{
  std::cout << "Setting parameters." << std::endl;

  vtkm::internal::FunctionInterface<void(const char *)> functionInterface;

  SetFirstParameterResolved(functionInterface, "foo");
  VTKM_TEST_ASSERT(functionInterface.GetParameter<1>() == std::string("foo"),
                   "Did not set string.");

  SetFirstParameterTemplated(functionInterface, "bar");
  VTKM_TEST_ASSERT(functionInterface.GetParameter<1>() == std::string("bar"),
                   "Did not set string.");
}

void BasicInvoke()
{
  ////
  //// BEGIN-EXAMPLE FunctionInterfaceBasicInvoke.cxx
  ////
  vtkm::internal::FunctionInterface<size_t(const char *)> strlenInterface =
      vtkm::internal::make_FunctionInterface<size_t>("Hello world");

  strlenInterface.InvokeCont(strlen);

  size_t length = strlenInterface.GetReturnValue();   // length = 11
  ////
  //// END-EXAMPLE FunctionInterfaceBasicInvoke.cxx
  ////

  VTKM_TEST_ASSERT(length == 11, "Bad length.");
}

namespace TransformedInvokeNamespace {

////
//// BEGIN-EXAMPLE FunctionInterfaceTransformInvoke.cxx
////
// Our transform converts C strings to integers, leaves everything else alone.
struct TransformFunctor
{
  template<typename T>
  VTKM_CONT_EXPORT
  const T &operator()(const T &x) const
  {
    return x;
  }

  VTKM_CONT_EXPORT
  const vtkm::Int32 operator()(const char *x) const
  {
    return atoi(x);
  }
};

// The function we are invoking simply compares two numbers.
struct IsSameFunctor
{
  template<typename T1, typename T2>
  VTKM_CONT_EXPORT
  bool operator()(const T1 &x, const T2 &y) const
  {
    return x == y;
  }
};

void TryTransformedInvoke()
{
  vtkm::internal::FunctionInterface<bool(const char *, vtkm::Int32)>
      functionInterface = vtkm::internal::make_FunctionInterface<bool>("42",42);

  functionInterface.InvokeCont(IsSameFunctor(), TransformFunctor());

  bool isSame = functionInterface.GetReturnValue();     // isSame = true
  ////
  //// PAUSE-EXAMPLE
  ////
  VTKM_TEST_ASSERT(isSame, "Did not get right return value.");
  ////
  //// RESUME-EXAMPLE
  ////
}
////
//// END-EXAMPLE FunctionInterfaceTransformInvoke.cxx
////

} // namespace TransformedInvokeNamespace

using namespace TransformedInvokeNamespace;

namespace ReturnContainerNamespace {

////
//// BEGIN-EXAMPLE FunctionInterfaceReturnContainer.cxx
////
template<typename ResultType, bool Valid> struct PrintReturnFunctor;

template<typename ResultType>
struct PrintReturnFunctor<ResultType, true>
{
  VTKM_CONT_EXPORT
  void operator()(
      const vtkm::internal::FunctionInterfaceReturnContainer<ResultType> &x)
  const
  {
    std::cout << x.Value << std::endl;
  }
};

template<typename ResultType>
struct PrintReturnFunctor<ResultType, false>
{
  VTKM_CONT_EXPORT
  void operator()(
      const vtkm::internal::FunctionInterfaceReturnContainer<ResultType> &)
  const
  {
    std::cout << "No return type." << std::endl;
  }
};

template<typename FunctionInterfaceType>
void PrintReturn(const FunctionInterfaceType &functionInterface)
{
  typedef typename FunctionInterfaceType::ResultType ResultType;
  typedef vtkm::internal::FunctionInterfaceReturnContainer<ResultType>
      ReturnContainerType;

  PrintReturnFunctor<ResultType, ReturnContainerType::VALID> printReturn;
  printReturn(functionInterface.GetReturnValueSafe());
}
////
//// END-EXAMPLE FunctionInterfaceReturnContainer.cxx
////

} // namespace ReturnContainerNamespace

void TryPrintReturn()
{
  vtkm::internal::FunctionInterface<size_t(const char *)> strlenInterface =
      vtkm::internal::make_FunctionInterface<size_t>("Hello world");
  strlenInterface.InvokeCont(strlen);
  ReturnContainerNamespace::PrintReturn(strlenInterface);

  ReturnContainerNamespace::PrintReturn(
        vtkm::internal::make_FunctionInterface<void>("Hello world"));
}

void Append()
{
  ////
  //// BEGIN-EXAMPLE FunctionInterfaceAppend.cxx
  ////
  using vtkm::internal::FunctionInterface;
  using vtkm::internal::make_FunctionInterface;

  typedef FunctionInterface<void(const char *, vtkm::Id)>
      InitialFunctionInterfaceType;
  InitialFunctionInterfaceType initialFunctionInterface =
      make_FunctionInterface<void>("Hello World", vtkm::Id(42));

  typedef FunctionInterface<void(const char *, vtkm::Id, const char *)>
      AppendedFunctionInterfaceType1;
  AppendedFunctionInterfaceType1 appendedFunctionInterface1 =
      initialFunctionInterface.Append("foobar");
  // appendedFunctionInterface1 has parameters ("Hello World", 42, "foobar")

  typedef InitialFunctionInterfaceType::AppendType<vtkm::Float32>::type
      AppendedFunctionInterfaceType2;
  AppendedFunctionInterfaceType2 appendedFunctionInterface2 =
      initialFunctionInterface.Append(vtkm::Float32(3.141));
  // appendedFunctionInterface2 has parameters ("Hello World", 42, 3.141)
  ////
  //// END-EXAMPLE FunctionInterfaceAppend.cxx
  ////

  std::cout << "Checking appended interface 1." << std::endl;
  VTKM_TEST_ASSERT(appendedFunctionInterface1.GetParameter<1>() == std::string("Hello World"),
                   "Bad value in interface.");
  VTKM_TEST_ASSERT(appendedFunctionInterface1.GetParameter<2>() == 42,
                   "Bad value in interface.");
  VTKM_TEST_ASSERT(appendedFunctionInterface1.GetParameter<3>() == std::string("foobar"),
                   "Bad value in interface.");

  std::cout << "Checking appended interface 2." << std::endl;
  VTKM_TEST_ASSERT(appendedFunctionInterface2.GetParameter<1>() == std::string("Hello World"),
                   "Bad value in interface.");
  VTKM_TEST_ASSERT(appendedFunctionInterface2.GetParameter<2>() == 42,
                   "Bad value in interface.");
  VTKM_TEST_ASSERT(appendedFunctionInterface2.GetParameter<3>() == vtkm::Float32(3.141),
                   "Bad value in interface.");
}

void Replace()
{
  ////
  //// BEGIN-EXAMPLE FunctionInterfaceReplace.cxx
  ////
  using vtkm::internal::FunctionInterface;
  using vtkm::internal::make_FunctionInterface;

  typedef FunctionInterface<void(const char *, vtkm::Id)>
      InitialFunctionInterfaceType;
  InitialFunctionInterfaceType initialFunctionInterface =
      make_FunctionInterface<void>("Hello World", vtkm::Id(42));

  typedef FunctionInterface<void(vtkm::Float32, vtkm::Id)>
      ReplacedFunctionInterfaceType1;
  ReplacedFunctionInterfaceType1 replacedFunctionInterface1 =
      initialFunctionInterface.Replace<1>(vtkm::Float32(3.141));
  // replacedFunctionInterface1 has parameters (3.141, 42)

  typedef InitialFunctionInterfaceType::ReplaceType<2, const char *>::type
      ReplacedFunctionInterfaceType2;
  ReplacedFunctionInterfaceType2 replacedFunctionInterface2 =
      initialFunctionInterface.Replace<2>("foobar");
  // replacedFunctionInterface2 has parameters ("Hello World", "foobar")
  ////
  //// END-EXAMPLE FunctionInterfaceReplace.cxx
  ////

  std::cout << "Checking replaced interface 1." << std::endl;
  VTKM_TEST_ASSERT(replacedFunctionInterface1.GetParameter<1>() == vtkm::Float32(3.141),
                   "Bad value in interface.");
  VTKM_TEST_ASSERT(replacedFunctionInterface1.GetParameter<2>() == 42,
                   "Bad value in interface.");

  std::cout << "Checking replaced interface 2." << std::endl;
  VTKM_TEST_ASSERT(replacedFunctionInterface2.GetParameter<1>() == std::string("Hello World"),
                   "Bad value in interface.");
  VTKM_TEST_ASSERT(replacedFunctionInterface2.GetParameter<2>() == std::string("foobar"),
                   "Bad value in interface.");
}

void NextFunctionChainCall(
    const vtkm::internal::FunctionInterface<void(vtkm::Id *, vtkm::Id)> &parameters)
{
  vtkm::Id expectedValue = TestValue(0, vtkm::Id());

  vtkm::Id *array = parameters.GetParameter<1>();
  vtkm::Id numValues = parameters.GetParameter<2>();

  std::cout << "Checking values." << std::endl;
  for (vtkm::Id index = 0; index < numValues; index++)
  {
    VTKM_TEST_ASSERT(array[index] == expectedValue, "Bad value.");
  }
}

////
//// BEGIN-EXAMPLE FunctionInterfaceAppendAndReplace.cxx
////
template<typename FunctionInterfaceType>
void FunctionCallChain(const FunctionInterfaceType &parameters,
                       vtkm::Id arraySize)
{
  // In this hypothetical function call chain, this function replaces the
  // first parameter with an array of that type and appends the array size
  // to the end of the parameters.

  typedef typename FunctionInterfaceType::template ParameterType<1>::type
      ArrayValueType;

  // Allocate and initialize array.
  ArrayValueType value = parameters.template GetParameter<1>();
  ArrayValueType *array = new ArrayValueType[arraySize];
  for (vtkm::Id index = 0; index < arraySize; index++)
  {
    array[index] = value;
  }

  // Call next function with modified parameters.
  NextFunctionChainCall(
        parameters.template Replace<1>(array).Append(arraySize));

  // Clean up.
  delete[] array;
}
////
//// END-EXAMPLE FunctionInterfaceAppendAndReplace.cxx
////

void TryAppendReplace()
{
  std::cout << "Using replace and append in function call chain." << std::endl;
  FunctionCallChain(
        vtkm::internal::make_FunctionInterface<void>(TestValue(0, vtkm::Id())),
        10);
}

namespace StaticTransformNamespace {

////
//// BEGIN-EXAMPLE FunctionInterfaceStaticTransform.cxx
////
struct ParametersToPointersFunctor {
  template<typename T, vtkm::IdComponent Index>
  struct ReturnType {
    typedef const T *type;
  };

  template<typename T, vtkm::IdComponent Index>
  VTKM_CONT_EXPORT
  const T *operator()(const T &x, vtkm::internal::IndexTag<Index>) const {
    return &x;
  }
};

template<typename FunctionInterfaceType>
VTKM_CONT_EXPORT
typename FunctionInterfaceType::
    template StaticTransformType<ParametersToPointersFunctor>::type
ParametersToPointers(const FunctionInterfaceType &functionInterface)
{
  return functionInterface.StaticTransformCont(ParametersToPointersFunctor());
}
////
//// END-EXAMPLE FunctionInterfaceStaticTransform.cxx
////

} // namespace StaticTransformNamespace

using namespace StaticTransformNamespace;

void TryStaticTransform()
{
  vtkm::internal::FunctionInterface<void(vtkm::Float32, vtkm::Int32)>
      originalFunctionInterface =
      vtkm::internal::make_FunctionInterface<void>(TestValue(1,vtkm::Float32()),
                                                   TestValue(2,vtkm::Int32()));

  vtkm::internal::FunctionInterface<void(const vtkm::Float32 *, const vtkm::Int32 *)>
      transformedFunctionInterface =
      ParametersToPointers(originalFunctionInterface);

  VTKM_TEST_ASSERT(
        *transformedFunctionInterface.GetParameter<1>() == TestValue(1,vtkm::Float32()),
        "Bad value in pointer.");
  VTKM_TEST_ASSERT(
        *transformedFunctionInterface.GetParameter<2>() == TestValue(2,vtkm::Int32()),
        "Bad value in pointer.");
}

namespace DynamicTransformNamespace {

////
//// BEGIN-EXAMPLE FunctionInterfaceDynamicTransform.cxx
////
struct UnpackNumbersTransformFunctor {
  template<typename InputType,
           typename ContinueFunctor,
           vtkm::IdComponent Index>
  VTKM_CONT_EXPORT
  void operator()(const InputType &input,
                  const ContinueFunctor &continueFunction,
                  vtkm::internal::IndexTag<Index>) const
  {
    continueFunction(input);
  }

  template<typename ContinueFunctor, vtkm::IdComponent Index>
  VTKM_CONT_EXPORT
  void operator()(const std::string &input,
                  const ContinueFunctor &continueFunction,
                  vtkm::internal::IndexTag<Index>) const
  {
    if ((input[0] >= '0') && (input[0] <= '9'))
    {
      std::stringstream stream(input);
      vtkm::FloatDefault value;
      stream >> value;
      continueFunction(value);
    }
    else
    {
      continueFunction(input);
    }
  }
};

////
//// PAUSE-EXAMPLE
////
struct CheckFunctor
{
  VTKM_CONT_EXPORT
  void operator()(vtkm::Float32 value1, std::string value2) const
  {
    VTKM_TEST_ASSERT(test_equal(value1, 42), "Wrong converted value.");
    VTKM_TEST_ASSERT(value2 == "Hello World", "Wrong passed value");
  }

  template<typename T1, typename T2>
  VTKM_CONT_EXPORT
  void operator()(T1, T2) const
  {
    VTKM_TEST_FAIL("Called wrong form of CheckFunctor");
  }
};
////
//// RESUME-EXAMPLE
////
struct UnpackNumbersFinishFunctor {
  template<typename FunctionInterfaceType>
  VTKM_CONT_EXPORT
  void operator()(FunctionInterfaceType &functionInterface) const
  {
    // Do something
    ////
    //// PAUSE-EXAMPLE
    ////
    functionInterface.InvokeCont(CheckFunctor());
    ////
    //// RESUME-EXAMPLE
    ////
  }
};

template<typename FunctionInterfaceType>
void DoUnpackNumbers(const FunctionInterfaceType &functionInterface)
{
  functionInterface.DynamicTransformCont(UnpackNumbersTransformFunctor(),
                                         UnpackNumbersFinishFunctor());
}
////
//// END-EXAMPLE FunctionInterfaceDynamicTransform.cxx
////

} // namespace DynamicTransformNamespace

using namespace DynamicTransformNamespace;

void TryDynamicTransform()
{
  vtkm::internal::FunctionInterface<void(std::string, std::string)>
      functionInterface = vtkm::internal::make_FunctionInterface<void>(
        std::string("42"), std::string("Hello World"));
  DoUnpackNumbers(functionInterface);
}

namespace DynamicTransformFunctorNamespace {

////
//// BEGIN-EXAMPLE DynamicTransform.cxx
////
template<typename Device>
struct ArrayCopyFunctor {
  template<typename Signature>
  VTKM_CONT_EXPORT
  void operator()(
      vtkm::internal::FunctionInterface<Signature> functionInterface) const
  {
    functionInterface.InvokeCont(*this);
  }

  template<typename T, class CIn, class COut>
  VTKM_CONT_EXPORT
  void operator()(const vtkm::cont::ArrayHandle<T, CIn> &input,
                  vtkm::cont::ArrayHandle<T, COut> &output) const
  {
    vtkm::cont::DeviceAdapterAlgorithm<Device>::Copy(input, output);
  }

  template<typename TIn, typename TOut, class CIn, class COut>
  VTKM_CONT_EXPORT
  void operator()(const vtkm::cont::ArrayHandle<TIn, CIn> &,
                  vtkm::cont::ArrayHandle<TOut, COut> &) const
  {
    throw vtkm::cont::ErrorControlBadType(
          "Arrays to copy must be the same type.");
  }
};

template<typename Device>
void CopyDynamicArrays(vtkm::cont::DynamicArrayHandle input,
                       vtkm::cont::DynamicArrayHandle output,
                       Device)
{
  vtkm::internal::FunctionInterface<void(vtkm::cont::DynamicArrayHandle,
                                         vtkm::cont::DynamicArrayHandle)>
      functionInterface =
        vtkm::internal::make_FunctionInterface<void>(input, output);

  functionInterface.DynamicTransformCont(
        vtkm::cont::internal::DynamicTransform(), ArrayCopyFunctor<Device>());
}
////
//// END-EXAMPLE DynamicTransform.cxx
////

} // namespace DynamicTransformFunctorNamespace

using namespace DynamicTransformFunctorNamespace;

void TryDynamicTransformFunctor()
{
  static const vtkm::Id ARRAY_SIZE = 10;
  vtkm::Float32 buffer[ARRAY_SIZE];
  for (vtkm::Id index = 0; index < ARRAY_SIZE; index++)
  {
    buffer[index] = TestValue(index, vtkm::Float32());
  }

  vtkm::cont::ArrayHandle<vtkm::Float32> inputArray =
      vtkm::cont::make_ArrayHandle(buffer, ARRAY_SIZE);
  vtkm::cont::ArrayHandle<vtkm::Float32> outputArray;

  CopyDynamicArrays(inputArray, outputArray, VTKM_DEFAULT_DEVICE_ADAPTER_TAG());

  CheckPortal(outputArray.GetPortalConstControl());
}

namespace ForEachNamespace {

////
//// BEGIN-EXAMPLE FunctionInterfaceForEach.cxx
////
struct PrintArgumentFunctor{
  template<typename T, vtkm::IdComponent Index>
  VTKM_CONT_EXPORT
  void operator()(const T &argument, vtkm::internal::IndexTag<Index>) const
  {
    std::cout << Index << ":" << argument << " ";
  }
};

template<typename FunctionInterfaceType>
VTKM_CONT_EXPORT
void PrintArguments(const FunctionInterfaceType &functionInterface)
{
  std::cout << "( ";
  functionInterface.ForEachCont(PrintArgumentFunctor());
  std::cout << ")" << std::endl;
}
////
//// END-EXAMPLE FunctionInterfaceForEach.cxx
////

} // namespace ForEachNamespace

using namespace ForEachNamespace;

void TryPrintArguments()
{
  PrintArguments(
        vtkm::internal::make_FunctionInterface<void>("Hello", 42, "World", 3.14));
}

void Test()
{
  BasicFunctionInterface();
  TryGetParameter();
  TrySetParameter();
  BasicInvoke();
  TryTransformedInvoke();
  TryPrintReturn();
  Append();
  Replace();
  TryAppendReplace();
  TryStaticTransform();
  TryDynamicTransform();
  TryDynamicTransformFunctor();
  TryPrintArguments();
}

} // anonymous namespace

int FunctionInterface(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(Test);
}
