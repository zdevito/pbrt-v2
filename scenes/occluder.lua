
Film("image", { filename = "occluder.exr", xresolution = Integer { 300 }, yresolution = Integer { 300 } })

Sampler("lowdiscrepancy", { pixelsamples = Integer { 4 } })
--LookAt {20, 20, 20,  0, 0, 0,   0, 1, 0 }
LookAt {4, 4, 10,   0, 2, 0,  0, 1, 0 }
Camera("perspective", { fov = Float {30} })


WorldBegin()



AttributeBegin()
   AreaLightSource("area", { L = Color { {10,10,10} }, nsamples = Integer { 4 } })
   Translate { 0, 4.5, 0 }
   Rotate { 45, 0, 1, 0 }
   Shape("trianglemesh", {indices = Integer { 0,1,2,2,3,0 }, P = Point { {-.6,0,-.6},   {.6,0,-.6},   {.6,0,.6}, {-.6,0,.6} } })
AttributeEnd()




AttributeBegin()
Material("matte", { Kd = Color { {.5, .5, .5} } })
Translate {0, 2, 0}
Shape("trianglemesh", {P =  Point { {-1, 0, -1},   {1, 0, -1},   {1, 0, 1},   {-1, 0, 1}, }, 
                       uv = Float { 0, 0, 1, 0, 1, 1, 0, 1 },
                       indices = Integer { 0, 1, 2, 2, 3, 0 } })
AttributeEnd()

AttributeBegin()
Material("matte", { Kd = Color { {.5, .5, .5} } })

Shape("trianglemesh", { P = Point { {-100, 0, -100},   {100, 0, -100},   {100, 0,100},   {-100, 0, 100} },
      uv = Float { 0, 0, 1, 0, 1, 1, 0, 1 },
	  indices = Integer {0, 1, 2, 2, 3, 0} })
AttributeEnd()


WorldEnd()


