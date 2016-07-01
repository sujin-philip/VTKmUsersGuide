
#include <vtkm/rendering/CanvasRayTracer.h>
#include <vtkm/rendering/ColorTable.h>

#include <vtkm/cont/testing/Testing.h>

namespace {

static const vtkm::Id TABLE_IMAGE_WIDTH = 300;

static const char *tableNames[] = {
  "blue",
  "orange",
  "cool2warm",
  "temperature",
  "rainbow",
  "levels",
  "dense",
  "sharp",
  "thermal",
  "IsoL",
  "CubicL",
  "CubicYF",
  "LinearL",
  "LinLhot",
  "PuRd",
  "Accent",
  "Blues",
  "BrBG",
  "BuGn",
  "BuPu",
  "Dark2",
  "GnBu",
  "Greens",
  "Greys",
  "Oranges",
  "OrRd",
  "Paired",
  "Pastel1",
  "Pastel2",
  "PiYG",
  "PRGn",
  "PuBu",
  "PuBuGn",
  "PuOr",
  "PuRd",
  "Purples",
  "RdBu",
  "RdGy",
  "RdPu",
  "RdYlBu",
  "RdYlGn",
  "Reds",
  "Set1",
  "Set2",
  "Set3",
  "Spectral",
  "YlGnBu",
  "YlGn",
  "YlOrBr",
  "YlOrRd"
};

static const vtkm::Id NUM_TABLES = sizeof(tableNames)/sizeof(char *);

void CreateColorTableImage(const std::string &name)
{
  std::cout << "Creating color table " << name << std::endl;

  vtkm::rendering::ColorTable colorTable(name);

  // Create a CanvasRayTracer simply for the color buffer and the ability to
  // write out images.
  vtkm::rendering::CanvasRayTracer canvas(TABLE_IMAGE_WIDTH, 1);
  typedef vtkm::rendering::CanvasRayTracer::ColorBufferType ColorBufferType;
  ColorBufferType colorBuffer = canvas.GetColorBuffer();
  ColorBufferType::PortalControl colorPortal = colorBuffer.GetPortalControl();
  VTKM_TEST_ASSERT(colorPortal.GetNumberOfValues() == TABLE_IMAGE_WIDTH,
                   "Wrong size of color buffer.");

  const vtkm::Float32 indexScale =
      1.0f/static_cast<vtkm::Float32>(TABLE_IMAGE_WIDTH-1);
  for (vtkm::Id index = 0; index < TABLE_IMAGE_WIDTH; index++)
  {
    vtkm::Float32 scalar = static_cast<vtkm::Float32>(index)*indexScale;
    vtkm::rendering::Color color = colorTable.MapRGB(scalar);
    colorPortal.Set(index, color.Components);
  }

  canvas.SaveAs(name + ".ppm");
}

void DoColorTables()
{
  for (vtkm::Id tableNameIndex = 0;
       tableNameIndex < NUM_TABLES;
       tableNameIndex++)
  {
    CreateColorTableImage(tableNames[tableNameIndex]);
  }
}

} // anonymous namespace

int ColorTables(int, char*[])
{
  return vtkm::cont::testing::Testing::Run(DoColorTables);
}
