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

////
//// BEGIN-EXAMPLE MouseRotate.cxx
////
void DoMouseRotate(vtkm::rendering::View &view,
                   vtkm::Id mouseStartX,
                   vtkm::Id mouseStartY,
                   vtkm::Id mouseEndX,
                   vtkm::Id mouseEndY)
{
  vtkm::Id screenWidth = view.GetCanvas().GetWidth();
  vtkm::Id screenHeight = view.GetCanvas().GetHeight();

  // Convert the mouse position coordinates, given in pixels from 0 to
  // width/height, to normalized screen coordinates from -1 to 1. Note that y
  // screen coordinates are usually given from the top down whereas our
  // geometry transforms are given from bottom up, so you have to reverse the y
  // coordiantes.
  vtkm::Float32 startX = (2.0f*mouseStartX)/screenWidth - 1.0f;
  vtkm::Float32 startY = -((2.0f*mouseStartY)/screenHeight - 1.0f);
  vtkm::Float32 endX = (2.0f*mouseEndX)/screenWidth - 1.0f;
  vtkm::Float32 endY = -((2.0f*mouseEndY)/screenHeight - 1.0f);

  view.GetCamera().TrackballRotate(startX, startY, endX, endY);
}
////
//// END-EXAMPLE MouseRotate.cxx
////

////
//// BEGIN-EXAMPLE MousePan.cxx
////
void DoMousePan(vtkm::rendering::View &view,
                vtkm::Id mouseStartX,
                vtkm::Id mouseStartY,
                vtkm::Id mouseEndX,
                vtkm::Id mouseEndY)
{
  vtkm::Id screenWidth = view.GetCanvas().GetWidth();
  vtkm::Id screenHeight = view.GetCanvas().GetHeight();

  // Convert the mouse position coordinates, given in pixels from 0 to
  // width/height, to normalized screen coordinates from -1 to 1. Note that y
  // screen coordinates are usually given from the top down whereas our
  // geometry transforms are given from bottom up, so you have to reverse the y
  // coordiantes.
  vtkm::Float32 startX = (2.0f*mouseStartX)/screenWidth - 1.0f;
  vtkm::Float32 startY = -((2.0f*mouseStartY)/screenHeight - 1.0f);
  vtkm::Float32 endX = (2.0f*mouseEndX)/screenWidth - 1.0f;
  vtkm::Float32 endY = -((2.0f*mouseEndY)/screenHeight - 1.0f);

  view.GetCamera().Pan(endX-startX, endY-startY);
}
////
//// END-EXAMPLE MousePan.cxx
////

////
//// BEGIN-EXAMPLE MouseZoom.cxx
////
void DoMouseZoom(vtkm::rendering::View &view,
                 vtkm::Id mouseStartY,
                 vtkm::Id mouseEndY)
{
  vtkm::Id screenHeight = view.GetCanvas().GetHeight();

  // Convert the mouse position coordinates, given in pixels from 0 to height,
  // to normalized screen coordinates from -1 to 1. Note that y screen
  // coordinates are usually given from the top down whereas our geometry
  // transforms are given from bottom up, so you have to reverse the y
  // coordiantes.
  vtkm::Float32 startY = -((2.0f*mouseStartY)/screenHeight - 1.0f);
  vtkm::Float32 endY = -((2.0f*mouseEndY)/screenHeight - 1.0f);

  view.GetCamera().Zoom(endY-startY);
}
////
//// END-EXAMPLE MouseZoom.cxx
////

void MouseMoveCallback(int x, int y)
{
  if (gButtonState[0] == GLUT_DOWN)
  {
    DoMouseRotate(*gViewPointer, gMousePositionX, gMousePositionY, x, y);
  }
  else if (gButtonState[1] == GLUT_DOWN)
  {
    DoMousePan(*gViewPointer, gMousePositionX, gMousePositionY, x, y);
  }
  else if (gButtonState[2] == GLUT_DOWN)
  {
    DoMouseZoom(*gViewPointer, gMousePositionY, y);
  }

  gMousePositionX = x;
  gMousePositionY = y;

  glutPostRedisplay();
}

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

void ResetCamera(vtkm::rendering::View &view)
{
  vtkm::Bounds bounds = view.GetScene().GetSpatialBounds();
  view.GetCamera().ResetToBounds(bounds);
  //// PAUSE-EXAMPLE
  std::cout << "Position:  " << view.GetCamera().GetPosition() << std::endl;
  std::cout << "LookAt:    " << view.GetCamera().GetLookAt() << std::endl;
  std::cout << "ViewUp:    " << view.GetCamera().GetViewUp() << std::endl;
  std::cout << "FOV:       " << view.GetCamera().GetFieldOfView() << std::endl;
  std::cout << "ClipRange: " << view.GetCamera().GetClippingRange() << std::endl;
  //// RESUME-EXAMPLE
}

void ChangeCamera(vtkm::rendering::Camera &camera)
{
  // Just set some camera parameters for demonstration purposes.
  ////
  //// BEGIN-EXAMPLE CameraPositionOrientation.cxx
  ////
  camera.SetPosition(vtkm::make_Vec(10.0, 6.0, 6.0));
  camera.SetLookAt(vtkm::make_Vec(0.0, 0.0, 0.0));
  camera.SetViewUp(vtkm::make_Vec(0.0, 1.0, 0.0));
  camera.SetFieldOfView(60.0);
  camera.SetClippingRange(0.1, 100.0);
  ////
  //// END-EXAMPLE CameraPositionOrientation.cxx
  ////
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
    case 'r':
    case 'R':
      ResetCamera(*gViewPointer);
      break;
    case 'c':
    case 'C':
      ChangeCamera(gViewPointer->GetCamera());
      break;
  }
  glutPostRedisplay();
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
  try
  {
    vtkm::io::reader::VTKDataSetReader reader("data/cow.vtk");
    surfaceData = reader.ReadDataSet();
  }
  catch (vtkm::io::ErrorIO &error)
  {
    std::cout << "Could not read file:" << std::endl
              << error.GetMessage() << std::endl;
  }
  catch (...)
  {
    throw;
  }

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
