# state file generated using paraview version 5.0.1

# ----------------------------------------------------------------
# setup views used in the visualization
# ----------------------------------------------------------------

#### import the simple module from the paraview
from paraview.simple import *
#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

# Create a new 'Render View'
renderView1 = CreateView('RenderView')
renderView1.ViewSize = [792, 551]
renderView1.AxesGrid = 'GridAxes3DActor'
renderView1.OrientationAxesLabelColor = [0.0, 0.0, 0.0]
renderView1.OrientationAxesOutlineColor = [0.0, 0.0, 0.0]
renderView1.CenterOfRotation = [0.5283281803131104, 0.1371404528617859, 0.48934412002563477]
renderView1.StereoType = 0
renderView1.CameraPosition = [1.2994298146982577, 0.5342821058510525, 10.024471534857792]
renderView1.CameraFocalPoint = [1.2994298146982577, 0.5342821058510525, -9.307525219633725]
renderView1.CameraParallelScale = 5.0034889399225255
renderView1.Background = [1.0, 1.0, 1.0]

# init the 'GridAxes3DActor' selected for 'AxesGrid'
renderView1.AxesGrid.XTitleColor = [0.0, 0.0, 0.0]
renderView1.AxesGrid.YTitleColor = [0.0, 0.0, 0.0]
renderView1.AxesGrid.ZTitleColor = [0.0, 0.0, 0.0]
renderView1.AxesGrid.GridColor = [0.0, 0.0, 0.0]
renderView1.AxesGrid.XLabelColor = [0.0, 0.0, 0.0]
renderView1.AxesGrid.YLabelColor = [0.0, 0.0, 0.0]
renderView1.AxesGrid.ZLabelColor = [0.0, 0.0, 0.0]

# ----------------------------------------------------------------
# setup the data processing pipelines
# ----------------------------------------------------------------

# create a new 'Point Source'
pointSource1 = PointSource()

# create a new 'Line'
frustum2 = Line()

# create a new 'Sphere'
sphere1 = Sphere()
sphere1.Radius = 4.0
sphere1.ThetaResolution = 40
sphere1.PhiResolution = 40

# create a new 'Line'
frustum1 = Line()

# create a new 'Line'
dolly = Line()

# create a new 'Slice'
elevationLine = Slice(Input=sphere1)
elevationLine.SliceType = 'Plane'
elevationLine.SliceOffsetValues = [0.0]

# init the 'Plane' selected for 'SliceType'
elevationLine.SliceType.Normal = [1.0, 0.0, -1.0]

# create a new 'Legacy VTK Reader'
teapotvtk = LegacyVTKReader(FileNames=['/Users/kmorel/papers/VTKmUsersGuide/data/teapot.vtk'])

# create a new 'Plane'
plane1 = Plane()
plane1.Origin = [-1.0, -1.0, 0.0]
plane1.Point1 = [1.0, -1.0, 0.0]
plane1.Point2 = [-1.0, 1.0, 0.0]

# create a new 'Transform'
farTranslate = Transform(Input=plane1)
farTranslate.Transform = 'Transform'

# init the 'Transform' selected for 'Transform'
farTranslate.Transform.Translate = [0.0, 0.0, -2.0]
farTranslate.Transform.Scale = [1.5, 1.5, 1.5]

# create a new 'Transform'
farElevation = Transform(Input=farTranslate)
farElevation.Transform = 'Transform'

# init the 'Transform' selected for 'Transform'
farElevation.Transform.Rotate = [-15.0, 0.0, 0.0]

# create a new 'Transform'
farAzimuth = Transform(Input=farElevation)
farAzimuth.Transform = 'Transform'

# init the 'Transform' selected for 'Transform'
farAzimuth.Transform.Rotate = [0.0, 45.0, 0.0]

# create a new 'Extract Edges'
extractEdges2 = ExtractEdges(Input=farAzimuth)

# create a new 'Slice'
azimuth = Slice(Input=sphere1)
azimuth.SliceType = 'Plane'
azimuth.SliceOffsetValues = [0.0]

# init the 'Plane' selected for 'SliceType'
azimuth.SliceType.Normal = [1.0, -5.25, 1.0]

# create a new 'Line'
frustum4 = Line()

# create a new 'Legacy VTK Reader'
cameravtk = LegacyVTKReader(FileNames=['/Users/kmorel/papers/VTKmUsersGuide/data/camera.vtk'])

# create a new 'Transform'
translate = Transform(Input=cameravtk)
translate.Transform = 'Transform'

# init the 'Transform' selected for 'Transform'
translate.Transform.Translate = [0.0, 0.0, 4.0]

# create a new 'Transform'
elevation = Transform(Input=translate)
elevation.Transform = 'Transform'

# init the 'Transform' selected for 'Transform'
elevation.Transform.Rotate = [-15.0, 0.0, 0.0]

# create a new 'Transform'
azimuth_1 = Transform(Input=elevation)
azimuth_1.Transform = 'Transform'

# init the 'Transform' selected for 'Transform'
azimuth_1.Transform.Rotate = [0.0, 45.0, 0.0]

# create a new 'Generate Surface Normals'
generateSurfaceNormals1 = GenerateSurfaceNormals(Input=azimuth_1)

# create a new 'Transform'
transform1 = Transform(Input=teapotvtk)
transform1.Transform = 'Transform'

# init the 'Transform' selected for 'Transform'
transform1.Transform.Translate = [0.0, -0.45, 0.0]

# create a new 'Line'
frustum3 = Line()

# create a new 'Transform'
nearTranslate = Transform(Input=plane1)
nearTranslate.Transform = 'Transform'

# init the 'Transform' selected for 'Transform'
nearTranslate.Transform.Translate = [0.0, 0.0, 2.0]
nearTranslate.Transform.Scale = [0.5, 0.5, 0.5]

# create a new 'Transform'
nearElevation = Transform(Input=nearTranslate)
nearElevation.Transform = 'Transform'

# init the 'Transform' selected for 'Transform'
nearElevation.Transform.Rotate = [-15.0, 0.0, 0.0]

# create a new 'Transform'
nearAzimuth = Transform(Input=nearElevation)
nearAzimuth.Transform = 'Transform'

# init the 'Transform' selected for 'Transform'
nearAzimuth.Transform.Rotate = [0.0, 45.0, 0.0]

# create a new 'Extract Edges'
extractEdges1 = ExtractEdges(Input=nearAzimuth)

# create a new 'Transform'
transform2 = Transform(Input=pointSource1)
transform2.Transform = 'Transform'

# init the 'Transform' selected for 'Transform'
transform2.Transform.Translate = [0.0, 0.0, 4.0]

# create a new 'Transform'
transform3 = Transform(Input=transform2)
transform3.Transform = 'Transform'

# init the 'Transform' selected for 'Transform'
transform3.Transform.Rotate = [-15.0, 0.0, 0.0]

# create a new 'Transform'
transform4 = Transform(Input=transform3)
transform4.Transform = 'Transform'

# init the 'Transform' selected for 'Transform'
transform4.Transform.Rotate = [0.0, 45.0, 0.0]

# ----------------------------------------------------------------
# setup the visualization in view 'renderView1'
# ----------------------------------------------------------------

# show data from transform1
transform1Display = Show(transform1, renderView1)
# trace defaults for the display properties.
transform1Display.AmbientColor = [0.0, 0.0, 0.0]
transform1Display.ColorArrayName = [None, '']
transform1Display.GlyphType = 'Arrow'
transform1Display.CubeAxesColor = [0.0, 0.0, 0.0]
transform1Display.SetScaleArray = [None, '']
transform1Display.ScaleTransferFunction = 'PiecewiseFunction'
transform1Display.OpacityArray = [None, '']
transform1Display.OpacityTransferFunction = 'PiecewiseFunction'

# show data from generateSurfaceNormals1
generateSurfaceNormals1Display = Show(generateSurfaceNormals1, renderView1)
# trace defaults for the display properties.
generateSurfaceNormals1Display.AmbientColor = [0.0, 0.0, 0.0]
generateSurfaceNormals1Display.ColorArrayName = [None, '']
generateSurfaceNormals1Display.DiffuseColor = [0.4980392156862745, 0.4980392156862745, 0.4980392156862745]
generateSurfaceNormals1Display.Specular = 1.0
generateSurfaceNormals1Display.GlyphType = 'Arrow'
generateSurfaceNormals1Display.CubeAxesColor = [0.0, 0.0, 0.0]
generateSurfaceNormals1Display.SetScaleArray = [None, '']
generateSurfaceNormals1Display.ScaleTransferFunction = 'PiecewiseFunction'
generateSurfaceNormals1Display.OpacityArray = [None, '']
generateSurfaceNormals1Display.OpacityTransferFunction = 'PiecewiseFunction'

# show data from nearAzimuth
nearAzimuthDisplay = Show(nearAzimuth, renderView1)
# trace defaults for the display properties.
nearAzimuthDisplay.AmbientColor = [0.0, 0.0, 0.0]
nearAzimuthDisplay.ColorArrayName = [None, '']
nearAzimuthDisplay.DiffuseColor = [1.0, 0.0, 0.0]
nearAzimuthDisplay.Opacity = 0.1
nearAzimuthDisplay.GlyphType = 'Arrow'
nearAzimuthDisplay.CubeAxesColor = [0.0, 0.0, 0.0]
nearAzimuthDisplay.SetScaleArray = [None, '']
nearAzimuthDisplay.ScaleTransferFunction = 'PiecewiseFunction'
nearAzimuthDisplay.OpacityArray = [None, '']
nearAzimuthDisplay.OpacityTransferFunction = 'PiecewiseFunction'

# show data from farAzimuth
farAzimuthDisplay = Show(farAzimuth, renderView1)
# trace defaults for the display properties.
farAzimuthDisplay.AmbientColor = [0.0, 0.0, 0.0]
farAzimuthDisplay.ColorArrayName = [None, '']
farAzimuthDisplay.DiffuseColor = [1.0, 0.0, 0.0]
farAzimuthDisplay.Opacity = 0.1
farAzimuthDisplay.GlyphType = 'Arrow'
farAzimuthDisplay.CubeAxesColor = [0.0, 0.0, 0.0]
farAzimuthDisplay.SetScaleArray = [None, '']
farAzimuthDisplay.ScaleTransferFunction = 'PiecewiseFunction'
farAzimuthDisplay.OpacityArray = [None, '']
farAzimuthDisplay.OpacityTransferFunction = 'PiecewiseFunction'

# show data from frustum1
frustum1Display = Show(frustum1, renderView1)
# trace defaults for the display properties.
frustum1Display.AmbientColor = [0.0, 0.0, 0.0]
frustum1Display.ColorArrayName = [None, '']
frustum1Display.DiffuseColor = [1.0, 0.0, 0.0]
frustum1Display.LineWidth = 2.0
frustum1Display.GlyphType = 'Arrow'
frustum1Display.CubeAxesColor = [0.0, 0.0, 0.0]
frustum1Display.SetScaleArray = [None, '']
frustum1Display.ScaleTransferFunction = 'PiecewiseFunction'
frustum1Display.OpacityArray = [None, '']
frustum1Display.OpacityTransferFunction = 'PiecewiseFunction'

# show data from frustum2
frustum2Display = Show(frustum2, renderView1)
# trace defaults for the display properties.
frustum2Display.AmbientColor = [0.0, 0.0, 0.0]
frustum2Display.ColorArrayName = [None, '']
frustum2Display.DiffuseColor = [1.0, 0.0, 0.0]
frustum2Display.LineWidth = 2.0
frustum2Display.GlyphType = 'Arrow'
frustum2Display.CubeAxesColor = [0.0, 0.0, 0.0]
frustum2Display.SetScaleArray = [None, '']
frustum2Display.ScaleTransferFunction = 'PiecewiseFunction'
frustum2Display.OpacityArray = [None, '']
frustum2Display.OpacityTransferFunction = 'PiecewiseFunction'

# show data from frustum3
frustum3Display = Show(frustum3, renderView1)
# trace defaults for the display properties.
frustum3Display.AmbientColor = [0.0, 0.0, 0.0]
frustum3Display.ColorArrayName = [None, '']
frustum3Display.DiffuseColor = [1.0, 0.0, 0.0]
frustum3Display.LineWidth = 2.0
frustum3Display.GlyphType = 'Arrow'
frustum3Display.CubeAxesColor = [0.0, 0.0, 0.0]
frustum3Display.SetScaleArray = [None, '']
frustum3Display.ScaleTransferFunction = 'PiecewiseFunction'
frustum3Display.OpacityArray = [None, '']
frustum3Display.OpacityTransferFunction = 'PiecewiseFunction'

# show data from frustum4
frustum4Display = Show(frustum4, renderView1)
# trace defaults for the display properties.
frustum4Display.AmbientColor = [0.0, 0.0, 0.0]
frustum4Display.ColorArrayName = [None, '']
frustum4Display.DiffuseColor = [1.0, 0.0, 0.0]
frustum4Display.LineWidth = 2.0
frustum4Display.GlyphType = 'Arrow'
frustum4Display.CubeAxesColor = [0.0, 0.0, 0.0]
frustum4Display.SetScaleArray = [None, '']
frustum4Display.ScaleTransferFunction = 'PiecewiseFunction'
frustum4Display.OpacityArray = [None, '']
frustum4Display.OpacityTransferFunction = 'PiecewiseFunction'

# show data from elevationLine
elevationLineDisplay = Show(elevationLine, renderView1)
# trace defaults for the display properties.
elevationLineDisplay.Representation = 'Wireframe'
elevationLineDisplay.AmbientColor = [0.0, 0.0, 0.0]
elevationLineDisplay.ColorArrayName = [None, '']
elevationLineDisplay.GlyphType = 'Arrow'
elevationLineDisplay.CubeAxesColor = [0.0, 0.0, 0.0]
elevationLineDisplay.SetScaleArray = [None, '']
elevationLineDisplay.ScaleTransferFunction = 'PiecewiseFunction'
elevationLineDisplay.OpacityArray = [None, '']
elevationLineDisplay.OpacityTransferFunction = 'PiecewiseFunction'

# show data from azimuth
azimuthDisplay = Show(azimuth, renderView1)
# trace defaults for the display properties.
azimuthDisplay.Representation = 'Wireframe'
azimuthDisplay.AmbientColor = [0.0, 0.0, 0.0]
azimuthDisplay.ColorArrayName = [None, '']
azimuthDisplay.GlyphType = 'Arrow'
azimuthDisplay.CubeAxesColor = [0.0, 0.0, 0.0]
azimuthDisplay.SetScaleArray = [None, '']
azimuthDisplay.ScaleTransferFunction = 'PiecewiseFunction'
azimuthDisplay.OpacityArray = [None, '']
azimuthDisplay.OpacityTransferFunction = 'PiecewiseFunction'

# show data from dolly
dollyDisplay = Show(dolly, renderView1)
# trace defaults for the display properties.
dollyDisplay.Representation = 'Wireframe'
dollyDisplay.AmbientColor = [0.0, 0.0, 0.0]
dollyDisplay.ColorArrayName = [None, '']
dollyDisplay.GlyphType = 'Arrow'
dollyDisplay.CubeAxesColor = [0.0, 0.0, 0.0]
dollyDisplay.SetScaleArray = [None, '']
dollyDisplay.ScaleTransferFunction = 'PiecewiseFunction'
dollyDisplay.OpacityArray = [None, '']
dollyDisplay.OpacityTransferFunction = 'PiecewiseFunction'

# show data from extractEdges1
extractEdges1Display = Show(extractEdges1, renderView1)
# trace defaults for the display properties.
extractEdges1Display.Representation = 'Wireframe'
extractEdges1Display.AmbientColor = [1.0, 0.0, 0.0]
extractEdges1Display.ColorArrayName = [None, '']
extractEdges1Display.LineWidth = 2.0
extractEdges1Display.GlyphType = 'Arrow'
extractEdges1Display.CubeAxesColor = [0.0, 0.0, 0.0]
extractEdges1Display.SetScaleArray = [None, '']
extractEdges1Display.ScaleTransferFunction = 'PiecewiseFunction'
extractEdges1Display.OpacityArray = [None, '']
extractEdges1Display.OpacityTransferFunction = 'PiecewiseFunction'

# show data from extractEdges2
extractEdges2Display = Show(extractEdges2, renderView1)
# trace defaults for the display properties.
extractEdges2Display.Representation = 'Wireframe'
extractEdges2Display.AmbientColor = [1.0, 0.0, 0.0]
extractEdges2Display.ColorArrayName = [None, '']
extractEdges2Display.LineWidth = 2.0
extractEdges2Display.GlyphType = 'Arrow'
extractEdges2Display.CubeAxesColor = [0.0, 0.0, 0.0]
extractEdges2Display.SetScaleArray = [None, '']
extractEdges2Display.ScaleTransferFunction = 'PiecewiseFunction'
extractEdges2Display.OpacityArray = [None, '']
extractEdges2Display.OpacityTransferFunction = 'PiecewiseFunction'
