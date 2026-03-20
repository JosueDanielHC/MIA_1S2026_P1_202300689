# ExtreamFS — Proyecto 1 MIA

## Información General

- **Nombre del proyecto:** ExtreamFS
- **Descripción:** Simulación de sistema de archivos EXT2 con backend en C++ y frontend web.
- **Curso:** Manejo e Implementación de Archivos (MIA)
- **Carnet:** 202300689
- **Universidad:** Universidad de San Carlos de Guatemala (USAC)
- **Año:** 2026

Simulación de sistema de archivos EXT2. Backend en C++, frontend en HTML/CSS/JS, comunicación por API HTTP.

## Requisitos

- **Compilar backend:** CMake 3.14+ y compilador C++17 (g++, clang++)
- **Ejecutar frontend:** abrir `frontend/index.html` en el navegador (o servir la carpeta con un servidor estático)

## Compilar y ejecutar

```bash
# Compilar
./scripts/build.sh
# o manualmente:
mkdir -p build && cd build && cmake .. && make

# Ejecutar servidor (puerto 8080)
./build/extreamfs
```

Luego abrir `frontend/index.html` y usar el botón **Ejecutar** (el backend debe estar corriendo en `http://localhost:8080`).

## Estructura del proyecto

- `backend/` — Servidor C++, analizador, comandos, discos, filesystem, reportes, utilidades.
- `frontend/` — Interfaz web (entrada/salida de comandos, carga de scripts .smia).
- `disks/` — Carpeta para discos virtuales `.mia`.
- `scripts/` — Scripts de construcción y utilidades.

## API

- **POST /execute** — Cuerpo JSON: `{"input": "comando aquí"}`. Respuesta: `{"output": "respuesta"}`.
