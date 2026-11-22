#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"

// Inclusión de Dear ImGui
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// ============================================================================
// CONFIGURACIÓN DE LA GRILLA
// ============================================================================
const int COLUMNAS = 50;
const int FILAS = 35;
const int TAMANO_CELDA = 15;
const int ANCHO_VENTANA = COLUMNAS * TAMANO_CELDA;
const int ALTO_VENTANA = FILAS * TAMANO_CELDA;

// ============================================================================
// ESTADOS Y VARIABLES DE DIBUJO
// ============================================================================
float colorPincel[3] = { 0.9f, 0.2f, 0.2f };
bool modoBorrar = false;

enum Herramienta
{
    PINCEL,
    LINEA,
    CIRCULO
};

Herramienta herramientaActual = PINCEL;
bool esperandoSegundoPunto = false;
int primerX = 0;
int primerY = 0;

// ============================================================================
// MATRICES QUE REPRESENTAN LA GRILLA
// ============================================================================
std::vector<std::vector<bool>> grilla(
    FILAS, std::vector<bool>(COLUMNAS, false)
);

std::vector<std::vector<bool>> grillaFiguras(
    FILAS, std::vector<bool>(COLUMNAS, false)
);

// ============================================================================
// ESTRUCTURA PARA GUARDAR CADA FIGURA Y SU TRANSFORMACIÓN
// ============================================================================
struct Figura
{
    int tipo; // 0 = línea, 1 = círculo
    int x1, y1, x2, y2;
    float moverX;
    float moverY;
    float escala;
    float angulo;
};

std::vector<Figura> figuras;
int figuraSeleccionada = -1;

// ============================================================================
// FUNCIÓN AUXILIAR PARA ACTIVAR O DESACTIVAR UNA CELDA
// ============================================================================
void marcarCelda(int x, int y, bool activar)
{
    if (x >= 0 && x < COLUMNAS && y >= 0 && y < FILAS)
    {
        grilla[y][x] = activar;
    }
}

void marcarCeldaFigura(int x, int y)
{
    if (x >= 0 && x < COLUMNAS && y >= 0 && y < FILAS)
    {
        grillaFiguras[y][x] = true;
    }
}

// ============================================================================
// ALGORITMO DE BRESENHAM PARA LÍNEAS
// ============================================================================
void calcularLineaBresenham(int x0, int y0, int x1, int y1)
{
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx - dy;

    while (true)
    {
        marcarCelda(x0, y0, true);

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}

// ============================================================================
// DIBUJA LOS 8 PUNTOS SIMÉTRICOS DEL CÍRCULO
// ============================================================================
void dibujarPuntosCirculo(int xc, int yc, int x, int y)
{
    marcarCelda(xc + x, yc + y, true);
    marcarCelda(xc - x, yc + y, true);
    marcarCelda(xc + x, yc - y, true);
    marcarCelda(xc - x, yc - y, true);
    marcarCelda(xc + y, yc + x, true);
    marcarCelda(xc - y, yc + x, true);
    marcarCelda(xc + y, yc - x, true);
    marcarCelda(xc - y, yc - x, true);
}

// ============================================================================
// ALGORITMO DE BRESENHAM PARA CÍRCULOS
// ============================================================================
void calcularCirculoBresenham(int xc, int yc, int radio)
{
    int x = 0;
    int y = radio;
    int d = 3 - (2 * radio);

    while (x <= y)
    {
        dibujarPuntosCirculo(xc, yc, x, y);

        if (d < 0)
        {
            d = d + (4 * x) + 6;
        }
        else
        {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

// ============================================================================
// CONVERSIÓN A COORDENADAS NORMALIZADAS (NDC)
// ============================================================================
void transformarACoordenadasNDC(float x, float y, float& ndcX, float& ndcY)
{
    ndcX = (x / ANCHO_VENTANA) * 2.0f - 1.0f;
    ndcY = (y / ALTO_VENTANA) * 2.0f - 1.0f;
}

// ============================================================================
// APLICA LA MATRIZ DE TRANSFORMACIÓN 2D A UN PUNTO
// Incluye traslación, escalamiento y rotación usando seno y coseno.
// La transformación se realiza respecto al centro original de la figura.
// ============================================================================
void transformarPunto(float x, float y, float centroX, float centroY,
    const Figura& figura, float& nuevoX, float& nuevoY)
{
    float radianes = figura.angulo * 3.14159265f / 180.0f;

    float coseno = std::cos(radianes);
    float seno = std::sin(radianes);

    float dx = (x - centroX) * figura.escala;
    float dy = (y - centroY) * figura.escala;

    // Matriz de rotación y escalamiento
    float rotadoX = dx * coseno - dy * seno;
    float rotadoY = dx * seno + dy * coseno;

    nuevoX = centroX + rotadoX + figura.moverX;
    nuevoY = centroY + rotadoY + figura.moverY;
}

// ============================================================================
// GENERA NUEVAMENTE LA GRILLA DE FIGURAS APLICANDO SUS TRANSFORMACIONES
// ============================================================================
void actualizarFiguras()
{
    for (int y = 0; y < FILAS; y++)
        for (int x = 0; x < COLUMNAS; x++)
            grillaFiguras[y][x] = false;

    for (int i = 0; i < (int)figuras.size(); i++)
    {
        Figura f = figuras[i];

        float centroX;
        float centroY;

        if (f.tipo == 0)
        {
            centroX = (f.x1 + f.x2) / 2.0f;
            centroY = (f.y1 + f.y2) / 2.0f;
        }
        else
        {
            centroX = (float)f.x1;
            centroY = (float)f.y1;
        }

        // Se dibuja primero la figura original en una grilla temporal.
        std::vector<std::vector<bool>> temporal(
            FILAS, std::vector<bool>(COLUMNAS, false)
        );

        if (f.tipo == 0)
        {
            int x0 = f.x1;
            int y0 = f.y1;
            int x1 = f.x2;
            int y1 = f.y2;

            int dx = std::abs(x1 - x0);
            int dy = std::abs(y1 - y0);
            int sx = (x0 < x1) ? 1 : -1;
            int sy = (y0 < y1) ? 1 : -1;
            int err = dx - dy;

            while (true)
            {
                if (x0 >= 0 && x0 < COLUMNAS && y0 >= 0 && y0 < FILAS)
                    temporal[y0][x0] = true;

                if (x0 == x1 && y0 == y1) break;

                int e2 = 2 * err;
                if (e2 > -dy)
                {
                    err -= dy;
                    x0 += sx;
                }
                if (e2 < dx)
                {
                    err += dx;
                    y0 += sy;
                }
            }
        }
        else
        {
            int radio = (int)std::round(std::sqrt(
                (f.x2 - f.x1) * (f.x2 - f.x1) +
                (f.y2 - f.y1) * (f.y2 - f.y1)
            ));

            int x = 0;
            int y = radio;
            int d = 3 - (2 * radio);

            while (x <= y)
            {
                int puntosX[8] = {
                    f.x1 + x, f.x1 - x, f.x1 + x, f.x1 - x,
                    f.x1 + y, f.x1 - y, f.x1 + y, f.x1 - y
                };
                int puntosY[8] = {
                    f.y1 + y, f.y1 + y, f.y1 - y, f.y1 - y,
                    f.y1 + x, f.y1 + x, f.y1 - x, f.y1 - x
                };

                for (int p = 0; p < 8; p++)
                {
                    if (puntosX[p] >= 0 && puntosX[p] < COLUMNAS &&
                        puntosY[p] >= 0 && puntosY[p] < FILAS)
                        temporal[puntosY[p]][puntosX[p]] = true;
                }

                if (d < 0)
                    d = d + (4 * x) + 6;
                else
                {
                    d = d + 4 * (x - y) + 10;
                    y--;
                }
                x++;
            }
        }

        // Cada píxel original se transforma con la matriz 2D.
        for (int y = 0; y < FILAS; y++)
        {
            for (int x = 0; x < COLUMNAS; x++)
            {
                if (temporal[y][x])
                {
                    float tx, ty;
                    transformarPunto((float)x, (float)y, centroX, centroY,
                        f, tx, ty);

                    int finalX = (int)std::round(tx);
                    int finalY = (int)std::round(ty);
                    marcarCeldaFigura(finalX, finalY);
                }
            }
        }
    }
}





// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================
int main()
{
    if (!glfwInit())
    {
        std::cout << "Error al inicializar GLFW\n";
        return -1;
    }

    GLFWwindow* ventana = glfwCreateWindow(
        ANCHO_VENTANA,
        ALTO_VENTANA,
        "Paint Grilla con Bresenham e ImGui",
        NULL,
        NULL
    );

    if (!ventana)
    {
        std::cout << "Error al crear ventana\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(ventana);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Error al cargar GLAD\n";
        glfwTerminate();
        return -1;
    }

    // --- INICIALIZAR DEAR IMGUI ---
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(ventana, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // Figuras iniciales del programa original
    calcularCirculoBresenham(25, 25, 6);
    calcularLineaBresenham(5, 2, 44, 14);

    bool mousePresionadoAnterior = false;
    Shader* shaderProgram = nullptr;
    VAO* vaoGrilla = nullptr;
    VBO* vboGrilla = nullptr;
    while (!glfwWindowShouldClose(ventana))
    {
        glfwPollEvents();

        // --- INICIAR FRAME DE IMGUI ---



        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- CREAR VENTANA DE CONTROL IMGUI ---
        ImGui::Begin("Panel de Control - Paint");

        ImGui::Text("Herramientas de dibujo");

        if (ImGui::Button("Pincel"))
        {
            herramientaActual = PINCEL;
            esperandoSegundoPunto = false;
        }

        ImGui::SameLine();
        if (ImGui::Button("Linea"))
        {
            herramientaActual = LINEA;
            esperandoSegundoPunto = false;
        }

        ImGui::SameLine();
        if (ImGui::Button("Circulo"))
        {
            herramientaActual = CIRCULO;
            esperandoSegundoPunto = false;
        }

        ImGui::Separator();

        ImGui::Checkbox("Modo Borrador", &modoBorrar);
        ImGui::ColorEdit3("Color del Pincel", colorPincel);

        if (herramientaActual == PINCEL)
            ImGui::Text("Herramienta actual: Pincel");
        else if (herramientaActual == LINEA)
            ImGui::Text("Herramienta actual: Linea");
        else
            ImGui::Text("Herramienta actual: Circulo");

        if (esperandoSegundoPunto)
            ImGui::Text("Seleccione el segundo punto en la grilla");

        ImGui::Separator();
        ImGui::Text("Transformaciones de figuras");

        if (figuras.empty())
        {
            ImGui::Text("Todavia no hay figuras creadas");
        }
        else
        {
            if (figuraSeleccionada < 0 ||
                figuraSeleccionada >= (int)figuras.size())
                figuraSeleccionada = 0;

            std::vector<std::string> nombres;
            for (int i = 0; i < (int)figuras.size(); i++)
            {
                std::string nombre = "Figura " + std::to_string(i + 1);
                if (figuras[i].tipo == 0)
                    nombre += " (Linea)";
                else
                    nombre += " (Circulo)";
                nombres.push_back(nombre);
            }

            if (ImGui::BeginCombo("Seleccionar figura",
                nombres[figuraSeleccionada].c_str()))
            {
                for (int i = 0; i < (int)nombres.size(); i++)
                {
                    bool seleccionada = (figuraSeleccionada == i);
                    if (ImGui::Selectable(nombres[i].c_str(), seleccionada))
                        figuraSeleccionada = i;

                    if (seleccionada)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            Figura& f = figuras[figuraSeleccionada];

            ImGui::SliderFloat("Mover X", &f.moverX, -25.0f, 25.0f);
            ImGui::SliderFloat("Mover Y", &f.moverY, -25.0f, 25.0f);
            ImGui::SliderFloat("Escala", &f.escala, 0.2f, 3.0f);
            ImGui::SliderFloat("Rotacion (grados)", &f.angulo, -180.0f, 180.0f);

            if (ImGui::Button("Restablecer transformacion"))
            {
                f.moverX = 0.0f;
                f.moverY = 0.0f;
                f.escala = 1.0f;
                f.angulo = 0.0f;
            }

            ImGui::SameLine();
            if (ImGui::Button("Eliminar figura"))
            {
                figuras.erase(figuras.begin() + figuraSeleccionada);
                figuraSeleccionada = figuras.empty() ? -1 : 0;
            }
        }

        ImGui::Separator();

        if (ImGui::Button("Limpiar Grilla"))
        {
            for (int y = 0; y < FILAS; y++)
                for (int x = 0; x < COLUMNAS; x++)
                    grilla[y][x] = false;

            figuras.clear();
            figuraSeleccionada = -1;
            esperandoSegundoPunto = false;
        }

        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::End();

        // --- GESTIÓN DEL MOUSE PARA PINTAR O CREAR FIGURAS ---
        bool mousePresionado =
            glfwGetMouseButton(ventana, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

        // Se trabaja solamente con el primer frame del clic.
        if (mousePresionado && !mousePresionadoAnterior &&
            !io.WantCaptureMouse)
        {
            double mouseX, mouseY;
            glfwGetCursorPos(ventana, &mouseX, &mouseY);

            int gx = (int)(mouseX / TAMANO_CELDA);
            int gy = (int)((ALTO_VENTANA - 1 - mouseY) / TAMANO_CELDA);

            if (gx >= 0 && gx < COLUMNAS && gy >= 0 && gy < FILAS)
            {
                if (herramientaActual == PINCEL)
                {
                    marcarCelda(gx, gy, !modoBorrar);
                }
                else
                {
                    if (!esperandoSegundoPunto)
                    {
                        primerX = gx;
                        primerY = gy;
                        esperandoSegundoPunto = true;
                    }
                    else
                    {
                        Figura nueva;
                        nueva.tipo = (herramientaActual == LINEA) ? 0 : 1;
                        nueva.x1 = primerX;
                        nueva.y1 = primerY;
                        nueva.x2 = gx;
                        nueva.y2 = gy;
                        nueva.moverX = 0.0f;
                        nueva.moverY = 0.0f;
                        nueva.escala = 1.0f;
                        nueva.angulo = 0.0f;

                        figuras.push_back(nueva);
                        figuraSeleccionada = (int)figuras.size() - 1;
                        esperandoSegundoPunto = false;
                    }
                }
            }
        }

        mousePresionadoAnterior = mousePresionado;

        // Actualizar las figuras para aplicar sus transformaciones
        actualizarFiguras();

        // Renderizar ImGui
        ImGui::Render();

        // --- RENDERIZADO OPENGL ---
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        std::vector<float> verticesRelleno;
        std::vector<float> verticesFiguras;
        std::vector<float> verticesLineas;

        for (int y = 0; y < FILAS; y++)
        {
            for (int x = 0; x < COLUMNAS; x++)
            {
                float xMinimo = x * TAMANO_CELDA;
                float yMinimo = y * TAMANO_CELDA;
                float xMaximo = xMinimo + TAMANO_CELDA;
                float yMaximo = yMinimo + TAMANO_CELDA;

                float ndcX1, ndcY1, ndcX2, ndcY2;
                transformarACoordenadasNDC(
                    xMinimo, yMinimo, ndcX1, ndcY1
                );
                transformarACoordenadasNDC(
                    xMaximo, yMaximo, ndcX2, ndcY2
                );

                // Celdas pintadas manualmente
                if (grilla[y][x])
                {
                    verticesRelleno.push_back(ndcX1);
                    verticesRelleno.push_back(ndcY1);
                    verticesRelleno.push_back(ndcX2);
                    verticesRelleno.push_back(ndcY1);
                    verticesRelleno.push_back(ndcX2);
                    verticesRelleno.push_back(ndcY2);

                    verticesRelleno.push_back(ndcX2);
                    verticesRelleno.push_back(ndcY2);
                    verticesRelleno.push_back(ndcX1);
                    verticesRelleno.push_back(ndcY2);
                    verticesRelleno.push_back(ndcX1);
                    verticesRelleno.push_back(ndcY1);
                }

                // Celdas pertenecientes a líneas y círculos
                if (grillaFiguras[y][x])
                {
                    verticesFiguras.push_back(ndcX1);
                    verticesFiguras.push_back(ndcY1);
                    verticesFiguras.push_back(ndcX2);
                    verticesFiguras.push_back(ndcY1);
                    verticesFiguras.push_back(ndcX2);
                    verticesFiguras.push_back(ndcY2);

                    verticesFiguras.push_back(ndcX2);
                    verticesFiguras.push_back(ndcY2);
                    verticesFiguras.push_back(ndcX1);
                    verticesFiguras.push_back(ndcY2);
                    verticesFiguras.push_back(ndcX1);
                    verticesFiguras.push_back(ndcY1);
                }

                // Bordes de las celdas de la grilla
                verticesLineas.push_back(ndcX1);
                verticesLineas.push_back(ndcY1);
                verticesLineas.push_back(ndcX2);
                verticesLineas.push_back(ndcY1);

                verticesLineas.push_back(ndcX2);
                verticesLineas.push_back(ndcY1);
                verticesLineas.push_back(ndcX2);
                verticesLineas.push_back(ndcY2);

                verticesLineas.push_back(ndcX2);
                verticesLineas.push_back(ndcY2);
                verticesLineas.push_back(ndcX1);
                verticesLineas.push_back(ndcY2);

                verticesLineas.push_back(ndcX1);
                verticesLineas.push_back(ndcY2);
                verticesLineas.push_back(ndcX1);
                verticesLineas.push_back(ndcY1);
            }
        }

        // Matriz identidad
        float matrizTransformacion[16] =
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        if (shaderProgram == nullptr)
        {
            shaderProgram = new Shader("Shaders/default.vert", "Shaders/default.frag");

            float dummyVertice[2] = { 0.0f, 0.0f };
            vaoGrilla = new VAO();
            vaoGrilla->Bind();

            vboGrilla = new VBO(dummyVertice, sizeof(dummyVertice));
            vaoGrilla->LinkVBO(*vboGrilla, 0);

            vaoGrilla->Unbind();
            vboGrilla->Unbind();
        }

        shaderProgram->Activate();
        vaoGrilla->Bind();
        vboGrilla->Bind();

        int ubicacionTransformacion =
            glGetUniformLocation(shaderProgram->ID, "uTransformacion");

        glUniformMatrix4fv(
            ubicacionTransformacion, 1, GL_FALSE, matrizTransformacion
        );

        // 1. Dibuja las celdas del pincel y borrador
        if (!verticesRelleno.empty())
        {
            glBufferData(
                GL_ARRAY_BUFFER,
                verticesRelleno.size() * sizeof(float),
                verticesRelleno.data(),
                GL_DYNAMIC_DRAW
            );

            int ubicacionColor =
                glGetUniformLocation(shaderProgram->ID, "uColor");

            glUniform3f(
                ubicacionColor,
                colorPincel[0],
                colorPincel[1],
                colorPincel[2]
            );

            glDrawArrays(
                GL_TRIANGLES, 0,
                (GLsizei)(verticesRelleno.size() / 2)
            );
        }

        // 2. Dibuja las figuras creadas con Bresenham
        if (!verticesFiguras.empty())
        {
            glBufferData(
                GL_ARRAY_BUFFER,
                verticesFiguras.size() * sizeof(float),
                verticesFiguras.data(),
                GL_DYNAMIC_DRAW
            );

            int ubicacionColor =
                glGetUniformLocation(shaderProgram->ID, "uColor");

            glUniform3f(
                ubicacionColor,
                colorPincel[0],
                colorPincel[1],
                colorPincel[2]
            );

            glDrawArrays(
                GL_TRIANGLES, 0,
                (GLsizei)(verticesFiguras.size() / 2)
            );
        }

        // 3. Dibuja las líneas de la grilla
        if (!verticesLineas.empty())
        {
            glBufferData(
                GL_ARRAY_BUFFER,
                verticesLineas.size() * sizeof(float),
                verticesLineas.data(),
                GL_DYNAMIC_DRAW
            );

            int ubicacionColor =
                glGetUniformLocation(shaderProgram->ID, "uColor");

            glUniform3f(
                ubicacionColor, 0.25f, 0.25f, 0.25f
            );

            glDrawArrays(
                GL_LINES, 0,
                (GLsizei)(verticesLineas.size() / 2)
            );
        }

        vboGrilla->Unbind();
        vaoGrilla->Unbind();

        // Renderizar los elementos gráficos de ImGui encima de OpenGL
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(ventana);
    }
    // --- LIMPIEZA DE OPENGL ---
    if (vaoGrilla != nullptr)
    {
        vaoGrilla->Delete();
        delete vaoGrilla;
        vaoGrilla = nullptr;
    }

    if (vboGrilla != nullptr)
    {
        vboGrilla->Delete();
        delete vboGrilla;
        vboGrilla = nullptr;
    }

    if (shaderProgram != nullptr)
    {
        shaderProgram->Delete();
        delete shaderProgram;
        shaderProgram = nullptr;
    }
    // --- LIMPIEZA DE IMGUI ---
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}