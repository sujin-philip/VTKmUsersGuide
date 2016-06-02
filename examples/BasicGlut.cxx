#ifdef __APPLE__
// Glut is depricated on apple, but is sticking around for now. Hopefully
// someone will step up and make FreeGlut or OpenGlut compatible. Or perhaps
// we should move to GLFW. For now, just disable the warnings.
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <vtkm/io/reader/VTKDataSetReader.h>

#include <vtkm/rendering/Actor.h>
#include <vtkm/rendering/Camera.h>
#include <vtkm/rendering/CanvasGL.h>
#include <vtkm/rendering/MapperGL.h>
#include <vtkm/rendering/View.h>
#include <vtkm/rendering/WorldAnnotatorGL.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

namespace BasicGlutExample {

typedef vtkm::rendering::MapperGL<> MapperType;
typedef vtkm::rendering::CanvasGL CanvasType;
typedef vtkm::rendering::WorldAnnotatorGL WorldAnnotatorType;
typedef vtkm::rendering::View3D<
    MapperType, CanvasType, WorldAnnotatorType> ViewType;

ViewType *gViewPointer = NULL;

int gButtonState[3] = { GLUT_UP, GLUT_UP, GLUT_UP };
int gMousePositionX;
int gMousePositionY;
//// PAUSE-EXAMPLE
bool gNoInteraction;
//// RESUME-EXAMPLE

void DisplayCallback()
{
  gViewPointer->Paint();
  glutSwapBuffers();
  //// PAUSE-EXAMPLE
  if (gNoInteraction)
  {
    delete gViewPointer;
    gViewPointer = NULL;
    exit(0);
  }
  //// RESUME-EXAMPLE
}

void WindowReshapeCallback(int width, int height)
{
  gViewPointer->Camera.Width = width;
  gViewPointer->Camera.Height = height;
}

void MouseButtonCallback(int buttonIndex, int state, int x, int y)
{
  gButtonState[buttonIndex] = state;
  gMousePositionX = x;
  gMousePositionY = y;
}

void MouseMoveCallback(int x, int y)
{
  vtkm::Int32 width = gViewPointer->Camera.Width;
  vtkm::Int32 height = gViewPointer->Camera.Height;

  vtkm::Float32 lastX = (2.0f*gMousePositionX)/width - 1.0f;
  vtkm::Float32 lastY = 1.0f - (2.0f*gMousePositionY)/height;
  vtkm::Float32 nextX = (2.0f*x)/width - 1.0f;
  vtkm::Float32 nextY = 1.0f - (2.0f*y)/height;

  if (gButtonState[0] == GLUT_DOWN)
  {
    gViewPointer->Camera.TrackballRotate(lastX, lastY, nextX, nextY);
  }
  else if (gButtonState[1] == GLUT_DOWN)
  {
    gViewPointer->Camera.Pan3D(nextX-lastX, nextY-lastY);
  }
  else if (gButtonState[2] == GLUT_DOWN)
  {
    gViewPointer->Camera.Zoom3D(nextY-lastY);
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
      delete gViewPointer;
      gViewPointer = NULL;
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

  MapperType mapper;
  CanvasType canvas;

  vtkm::rendering::Actor actor(surfaceData.GetCellSet(),
                               surfaceData.GetCoordinateSystem(),
                               surfaceData.GetField("RandomPointScalars"),
                               // Get rid of this after edits.
                               vtkm::rendering::ColorTable(""));

  vtkm::rendering::Scene scene;
  scene.Actors.push_back(actor);

  vtkm::rendering::Camera camera(vtkm::rendering::Camera::VIEW_3D);
  camera.Camera3d.LookAt = vtkm::Vec<vtkm::Float32,3>(0.775f, -0.44f, 0.0f);
  camera.Camera3d.Position = vtkm::Vec<vtkm::Float32,3>(0.775f, -0.44f, -25.0f);
  camera.Camera3d.Up = vtkm::Vec<vtkm::Float32,3>(0.0f, 1.0f, 0.0f);
  camera.Camera3d.FieldOfView = 30.0f;
  camera.NearPlane = 1.0f;
  camera.FarPlane = 10000.0f;

  gViewPointer = new ViewType(scene, mapper, canvas, camera);

  //// PAUSE-EXAMPLE
  if ((argc > 1) && (strcmp(argv[1], "--no-interaction") == 0))
  {
    gNoInteraction = true;
  }
  else
  {
    gNoInteraction = false;
  }
  //// RESUME-EXAMPLE
  // Start the GLUT rendering system. This function typically does not return.
  glutMainLoop();

  return 0;
}

} // namespace BasicGlutExample

int BasicGlut(int argc, char *argv[])
{
  return BasicGlutExample::main(argc, argv);
}
