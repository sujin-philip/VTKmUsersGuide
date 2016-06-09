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

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

namespace OtherGlutExample {

vtkm::rendering::View3D *gViewPointer = NULL;

int gButtonState[3] = { GLUT_UP, GLUT_UP, GLUT_UP };
int gMousePositionX;
int gMousePositionY;
bool gNoInteraction;

void SaveImage()
{
  std::cout << "Saving image." << std:: endl;

  vtkm::rendering::Canvas &canvas = gViewPointer->GetCanvas();

  ////
  //// BEGIN-EXAMPLE SaveCanvasImage.cxx
  ////
  canvas.SaveAs("MyVis.ppm");
  ////
  //// END-EXAMPLE SaveCanvasImage.cxx
  ////
}

void DisplayCallback()
{
  gViewPointer->Paint();
  glutSwapBuffers();
  if (gNoInteraction)
  {
    delete gViewPointer;
    gViewPointer = NULL;
    exit(0);
  }
}

void WindowReshapeCallback(int width, int height)
{
  gViewPointer->GetCanvas().ResizeBuffers(width, height);
}

void MouseButtonCallback(int buttonIndex, int state, int x, int y)
{
  gButtonState[buttonIndex] = state;
  gMousePositionX = x;
  gMousePositionY = y;
}

void MouseMoveCallback(int x, int y)
{
  vtkm::Int32 width = gViewPointer->GetCanvas().GetWidth();
  vtkm::Int32 height = gViewPointer->GetCanvas().GetHeight();

  vtkm::Float32 lastX = (2.0f*gMousePositionX)/width - 1.0f;
  vtkm::Float32 lastY = 1.0f - (2.0f*gMousePositionY)/height;
  vtkm::Float32 nextX = (2.0f*x)/width - 1.0f;
  vtkm::Float32 nextY = 1.0f - (2.0f*y)/height;

  if (gButtonState[0] == GLUT_DOWN)
  {
    gViewPointer->GetCamera().TrackballRotate(lastX, lastY, nextX, nextY);
  }
  else if (gButtonState[1] == GLUT_DOWN)
  {
    gViewPointer->GetCamera().Pan(nextX-lastX, nextY-lastY);
  }
  else if (gButtonState[2] == GLUT_DOWN)
  {
    gViewPointer->GetCamera().Zoom(nextY-lastY);
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
    case 's':
    case 'S':
      SaveImage();
      break;
  }
  (void)x;  (void)y;
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

  vtkm::rendering::Actor actor(surfaceData.GetCellSet(),
                               surfaceData.GetCoordinateSystem(),
                               surfaceData.GetField("RandomPointScalars"));

  vtkm::rendering::Scene scene;
  scene.AddActor(actor);

  vtkm::rendering::MapperGL<> mapper;
  vtkm::rendering::CanvasGL canvas;

  gViewPointer =
  ////
  //// BEGIN-EXAMPLE ViewBackgroundColor.cxx
  ////
      new vtkm::rendering::View3D(
        scene, mapper, canvas, vtkm::rendering::Color(1.0f, 1.0f, 1.0f));
  ////
  //// END-EXAMPLE ViewBackgroundColor.cxx
  ////
  gViewPointer->Initialize();

  if ((argc > 1) && (strcmp(argv[1], "--no-interaction") == 0))
  {
    gNoInteraction = true;
  }
  else
  {
    gNoInteraction = false;
  }

  // Start the GLUT rendering system. This function typically does not return.
  glutMainLoop();

  return 0;
}

} // namespace OtherGlutExample

int OtherGlut(int argc, char *argv[])
{
  return OtherGlutExample::main(argc, argv);
}
