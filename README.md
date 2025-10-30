@'
Kernel-Sim (Entrega 2) - Simulador de Núcleo de Sistema Operativo Simplificado
-----------------------------------------------------------------------------
Contenido: Planificador RR + SJF (base), gestión de memoria (FIFO & LRU base), productor-consumidor,
CLI completa y exportación CSV.

Compilar en Windows (MinGW / g++):
  g++ -std=c++17 -O2 main.cpp modules/cpu/scheduler.cpp modules/mem/memory.cpp modules/sync/sync.cpp modules/io/io.cpp -o kernel-sim.exe

Ejecutar:
  .\kernel-sim.exe

Archivos principales:
  main.cpp                -> punto de entrada y CLI
  modules/cpu/*           -> scheduler y pcb
  modules/mem/*           -> memory manager
  modules/sync/*          -> productor-consumidor
  modules/io/*            -> IO simulado
  docs/*                  -> documentación mínima
  data/resultados.csv     -> ejemplo de salida CSV
'@ | Out-File -Encoding utf8 README.md
