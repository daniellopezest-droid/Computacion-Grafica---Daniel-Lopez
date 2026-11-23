# Paint Grilla 2D con Bresenham y Imgui

Daniel Lopez - 6014394  
Proyecto: Computación Gráfica (`Main.cpp` + clases OpenGL + Shaders GLSL + Dear ImGui)

## Arquitectura del proyecto

```text
Computacion Grafica
├── Main.cpp                  → Bucle principal, grilla 50x35, Bresenham, transformaciones 2D e ImGui
├── Shaders/
│   ├── default.vert          → Vertex shader (transformación de vértices con uTransformacion)
│   └── default.frag          → Fragment shader (color uniforme con uColor)
├── Clases OpenGL (C++ / GLAD)
│   ├── shaderClass.h / .cpp  → Carga de archivos GLSL, compilación y enlace del programa de shaders
│   ├── VAO.h / .cpp          → Gestión de Vertex Array Objects y enlace de atributos de vértice
│   ├── VBO.h / .cpp          → Gestión de Vertex Buffer Objects para envío de geometría
│   └── EBO.h / .cpp          → Gestión de Element Buffer Objects
└── Dear ImGui                → Interfaz de usuario para herramientas, color y transformaciones
Todo empieza en el  Main.cpp, donde se arma la ventana con GLFW, se cargan las funciones de OpenGL con GLAD y se cuadra el panel de Dear ImGui para poder manejar la grilla, ponerle los botones de las acciones, etc.
Ahí mismo está toda la lógica para pintar con pincel o borrador, tirar líneas y círculos de Bresenham, y mover, escalar o rotar las figuras usando seno y coseno con respecto a su centro.
Tambien en vez de tener los shaders metidos como texto dentro del Main.cpp, ahora estan afuera en la carpeta Shaders, y quedaron guardados como default.vert y default.frag.
Así todo el manejo de shaders y buffers (Shader, VAO, VBO y EBO) quedó separado en sus propias clases .h y .cpp para que el código no sea un solo bloque gigante y se pueda limpiar bien al cerrar.
