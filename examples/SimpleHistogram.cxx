#include <vtkm/Math.h>
#include <vtkm/Range.h>

#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayHandleConstant.h>
#include <vtkm/cont/ArrayRangeCompute.h>
#include <vtkm/cont/DeviceAdapter.h>

#include <vtkm/worklet/DispatcherMapField.h>
#include <vtkm/worklet/WorkletMapField.h>

#include <vtkm/cont/testing/Testing.h>

struct SimpleHistogram
{
  ////
  //// BEGIN-EXAMPLE SimpleHistogram.cxx
  ////
  struct CountBins : vtkm::worklet::WorkletMapField
  {
    typedef void ControlSignature(FieldIn<Scalar> data,
                                  AtomicArrayInOut<> histogramBins);
    typedef void ExecutionSignature(_1, _2);
    using InputDomain = _1;

    vtkm::Range HistogramRange;
    vtkm::Id NumberOfBins;

    VTKM_CONT
    CountBins(const vtkm::Range &histogramRange, vtkm::Id &numBins)
      : HistogramRange(histogramRange), NumberOfBins(numBins)
    {  }

    template<typename T, typename AtomicArrayType>
    void operator()(T value, const AtomicArrayType &histogramBins) const
    {
      vtkm::Float64 interp =
          (value - this->HistogramRange.Min)/this->HistogramRange.Length();
      vtkm::Id bin = static_cast<vtkm::Id>(interp*this->NumberOfBins);
      if (bin < 0) { bin = 0; }
      if (bin >= this->NumberOfBins) { bin = this->NumberOfBins - 1; }

      histogramBins.Add(bin, 1);
    }
  };
  ////
  //// END-EXAMPLE SimpleHistogram.cxx
  ////

  template<typename InputArray, typename Device>
  VTKM_CONT
  static vtkm::cont::ArrayHandle<vtkm::Int32>
  Run(const InputArray &input, vtkm::Id numberOfBins, Device)
  {
    VTKM_IS_ARRAY_HANDLE(InputArray);

    // TODO: Should check that input type has only one component.

    vtkm::Range range =
        vtkm::cont::ArrayRangeCompute(input).GetPortalConstControl().Get(0);

    // Initialize histogram to 0
    vtkm::cont::ArrayHandle<vtkm::Int32> histogram;
    vtkm::cont::DeviceAdapterAlgorithm<Device>::Copy(
          vtkm::cont::ArrayHandleConstant<vtkm::Int32>(0, numberOfBins),
          histogram);

    CountBins histogramWorklet(range, numberOfBins);

    vtkm::worklet::DispatcherMapField<CountBins, Device>
        dispatcher(histogramWorklet);
    dispatcher.Invoke(input, histogram);

    return histogram;
  }
};

VTKM_CONT
static inline void TrySimpleHistogram()
{
  std::cout << "Try Simple Histogram" << std::endl;

  static const vtkm::Id ARRAY_SIZE = 100;
  vtkm::cont::ArrayHandle<vtkm::Float32> inputArray;
  inputArray.Allocate(ARRAY_SIZE);
  SetPortal(inputArray.GetPortalControl());

  vtkm::cont::ArrayHandle<vtkm::Int32> histogram =
      SimpleHistogram::Run(inputArray,
                           ARRAY_SIZE/2,
                           VTKM_DEFAULT_DEVICE_ADAPTER_TAG());

  VTKM_TEST_ASSERT(histogram.GetNumberOfValues() == ARRAY_SIZE/2,
                   "Bad array size");
  for (vtkm::Id index = 0; index < histogram.GetNumberOfValues(); ++index)
  {
    vtkm::Int32 binSize = histogram.GetPortalConstControl().Get(index);
    VTKM_TEST_ASSERT(binSize == 2, "Bad bin size.");
  }
}

int SimpleHistogram(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(TrySimpleHistogram);
}
