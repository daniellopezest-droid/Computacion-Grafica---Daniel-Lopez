# Paint Grilla 2D con Bresenham y Imgui

Daniel Lopez - 6014394  
Proyecto: Computación Gráfica (`Main.cpp` + clases OpenGL + Shaders GLSL + Dear ImGui)

# Arquitectura del proyecto – Computación Gráfica

## 1. Estructura general

```text
Computacion Grafica/
│
├── Main.cpp
│
├── Shaders/
│   ├── default.vert
│   └── default.frag
│
├── Clases/
│   ├── shaderClass.h
│   ├── shaderClass.cpp
│   ├── VAO.h
│   ├── VAO.cpp
│   ├── VBO.h
│   ├── VBO.cpp
│   ├── EBO.h
│   └── EBO.cpp
│
└── Dear ImGui/
    └── Archivos de la biblioteca
```

## 2. Descripción de los módulos

| Módulo | Responsabilidad |
|---|---|
| `Main.cpp` | Es donde inicia el programa. Inicializa GLFW y GLAD y configura la ventana y administra el ciclo principal de ejecución. |
| `Shaders/` | Contiene los programas GLSL de OpenGL para procesar los vértices y establecer el color de los fragmentos. |
| `shaderClass` | Se encarga de leer los archivos GLSL, compilar los shaders y enlazarlos en el programa. |
| `VAO` | Administra los Vertex Array Objects y la configuración de los atributos de vértices. |
| `VBO` | Administra los Vertex Buffer Objects, utilizados para almacenar y enviar información de geometría a la GPU. |
| `EBO` | Administra los Element Buffer Objects, utilizados para definir índices de dibujo. |
| `Dear ImGui` | Proporciona la interfaz gráfica para seleccionar herramientas, modificar colores y controlar las transformaciones. |

## 3. Funcionamiento general

Todo empieza en `Main.cpp`, donde se arma la ventana por medio de GLFW y se cargan las funciones de OpenGL utilizando GLAD.

Despues se configura la interfaz de Dear ImGui y se prepara la grilla de 50 × 35 celdas.

El codigo tiene las siguientes herramientas:

- **Pincel:** permite colorear las celdas de la grilla.
- **Borrador:** permite eliminar el contenido dibujado.
- **Línea:** genera líneas mediante el algoritmo de Bresenham.
- **Círculo:** genera circunferencias mediante el algoritmo de Bresenham.

También se pueden aplicar transformaciones geométricas individuales a las líneas y los círculos:

- Traslación para cambiar su posición.
- Escalamiento para modificar su tamaño.
- Rotación mediante matrices de transformación, utilizando seno y coseno.

## 4. Manejo de shaders y buffers

Los shaders se encuentran separados de `Main.cpp` y almacenados en la carpeta `Shaders`, con los archivos `default.vert` y `default.frag`.

Esta organización permite modificar el código GLSL sin tener que editar directamente el archivo principal del programa.

Ademas las clases encargadas de administrar los shaders y los buffers de OpenGL permiten distribuir las responsabilidades del código y que sea mas facil su mantenimiento.

## 5. Tecnologías utilizadas

- **C++:** lenguaje principal del proyecto.
- **OpenGL:** biblioteca gráfica para el procesamiento y dibujo de elementos.
- **GLFW:** creación y administración de ventanas y contextos de OpenGL.
- **GLAD:** carga de las funciones de OpenGL.
- **Dear ImGui:** construcción de la interfaz gráfica.
- **GLSL:** lenguaje utilizado para escribir los shaders de vértices y fragmentos.

## 6. Objetivo de la arquitectura

La organización del proyecto busca separar la lógica principal de dibujo, los recursos gráficos y la administración de OpenGL. De esta manera, el código resulta más organizado, facilita futuras modificaciones y permite identificar con mayor claridad la función de cada componente.
