#ifdef __APPLE__
// Glut is depricated on apple, but is sticking around for now. Hopefully
// someone will step up and make FreeGlut or OpenGlut compatible. Or perhaps
// we should move to GLFW. For now, just disable the warnings.
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <vtkm/io/reader/VTKDataSetReader.h>

#include <vtkm/rendering/RenderSurfaceGL.h>
#include <vtkm/rendering/SceneRendererGL.h>
#include <vtkm/rendering/View.h>
#include <vtkm/rendering/Window.h>
#include <vtkm/rendering/WorldAnnotatorGL.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

namespace BasicGlutExample {

typedef vtkm::rendering::SceneRendererGL<> SceneRendererType;
typedef vtkm::rendering::RenderSurfaceGL RenderSurfaceType;
typedef vtkm::rendering::WorldAnnotatorGL WorldAnnotatorType;
typedef vtkm::rendering::Window3D<
    SceneRendererType, RenderSurfaceType, WorldAnnotatorType> WindowType;

WindowType *gWindowPointer = NULL;

int gButtonState[3] = { GLUT_UP, GLUT_UP, GLUT_UP };
int gMousePositionX;
int gMousePositionY;

void DisplayCallback()
{
  gWindowPointer->Paint();
  glutSwapBuffers();
}

void WindowReshapeCallback(int width, int height)
{
  gWindowPointer->View.Width = width;
  gWindowPointer->View.Height = height;
}

void MouseButtonCallback(int buttonIndex, int state, int x, int y)
{
  gButtonState[buttonIndex] = state;
  gMousePositionX = x;
  gMousePositionY = y;
}

void MouseMoveCallback(int x, int y)
{
  vtkm::Int32 width = gWindowPointer->View.Width;
  vtkm::Int32 height = gWindowPointer->View.Height;

  vtkm::Float32 lastX = (2.0f*gMousePositionX)/width - 1.0f;
  vtkm::Float32 lastY = 1.0f - (2.0f*gMousePositionY)/height;
  vtkm::Float32 nextX = (2.0f*x)/width - 1.0f;
  vtkm::Float32 nextY = 1.0f - (2.0f*y)/height;

  if (gButtonState[0] == GLUT_DOWN)
  {
    gWindowPointer->View.TrackballRotate(lastX, lastY, nextX, nextY);
  }
  else if (gButtonState[1] == GLUT_DOWN)
  {
    gWindowPointer->View.Pan3D(nextX-lastX, nextY-lastY);
  }
  else if (gButtonState[2] == GLUT_DOWN)
  {
    gWindowPointer->View.Zoom3D(nextY-lastY);
  }

  gMousePositionX = x;
  gMousePositionY = y;

  glutPostRedisplay();
}

void KeyPressCallback(unsigned char key, int x, int y)
{
  switch (key)
  {
    case 'q':
    case 'Q':
      delete gWindowPointer;
      gWindowPointer = NULL;
      exit(0);
      break;
  }
  //// PAUSE-EXAMPLE
  (void)x;  (void)y;
  //// RESUME-EXAMPLE
}

int main(int argc, char *argv[])
{
  // Initialize GLUT window and callbacks
  glutInit(&argc, argv);
  glutInitWindowSize(960, 600);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH);
  glutCreateWindow("VTK-m Example");

  glutDisplayFunc(DisplayCallback);
  glutReshapeFunc(WindowReshapeCallback);
  glutMouseFunc(MouseButtonCallback);
  glutMotionFunc(MouseMoveCallback);
  glutKeyboardFunc(KeyPressCallback);

  // Initialize VTK-m rendering classes
  vtkm::cont::DataSet surfaceData;
  vtkm::io::reader::VTKDataSetReader reader("data/cow.vtk");
  surfaceData = reader.ReadDataSet();
  std::cout << surfaceData.GetCoordinateSystem().GetBounds(VTKM_DEFAULT_DEVICE_ADAPTER_TAG()) << std::endl;

  SceneRendererType sceneRenderer;
  RenderSurfaceType renderSurface;

  vtkm::rendering::Plot plot(surfaceData.GetCellSet(),
                             surfaceData.GetCoordinateSystem(),
                             surfaceData.GetField("RandomPointScalars"),
                             // Get rid of this after edits.
                             vtkm::rendering::ColorTable(""));

  vtkm::rendering::Scene scene;
  scene.Plots.push_back(plot);

  vtkm::rendering::View view(vtkm::rendering::View::VIEW_3D);
  view.View3d.LookAt = vtkm::Vec<vtkm::Float32,3>(0.775f, -0.44f, 0.0f);
  view.View3d.Position = vtkm::Vec<vtkm::Float32,3>(0.775f, -0.44f, -25.0f);
  view.View3d.Up = vtkm::Vec<vtkm::Float32,3>(0.0f, 1.0f, 0.0f);
  view.View3d.FieldOfView = 30.0f;
  view.NearPlane = 1.0f;
  view.FarPlane = 10000.0f;

  gWindowPointer = new WindowType(scene, sceneRenderer, renderSurface, view);

  // Start the GLUT rendering system. This function typically does not return.
  glutMainLoop();

  return 0;
}

} // namespace BasicGlutExample

int BasicGlut(int argc, char *argv[])
{
  return BasicGlutExample::main(argc, argv);
}
