# Notas de traspaso: modo dual (Game Gear y Neo Geo Pocket)

Objetivo: el mismo modo dual que el Cable Link de gpsp
(https://github.com/Ninoh-FOX/gpsp, `dual.c`, `README-dual.md`). Cada
dispositivo emula las dos máquinas: la suya y una copia oculta de la del
otro. No se transfieren ROMs ni saves a mano: cada jugador conserva su ROM y
su `.srm`, y por la red solo viajan los botones y el tipo de conexión. La
máquina oculta no genera vídeo ni audio.

## Game Gear (Gearsystem): hecho, falta probar en consola

Rama `dual-link` de este repositorio (fork de drhelius/Gearsystem,
upstream 1be9a12). La documentación está en
`platforms/libretro/README-dual.md`.

- `platforms/libretro/libretro_dual.cpp/.h`: la sesión. Es un port de
  `dual.c` de gpsp reducido a 2 consolas y sin hilos.
- `platforms/libretro/libretro.cpp`: hooks (capturar, enlazar, desenlazar,
  botones, reset, hash) y el interfaz netpacket. Solo se registra para
  juegos nativos de Game Gear con la opción `gearsystem_dual_link`
  activada (por defecto sí).
- `platforms/libretro/libretro_link.cpp/.h`: `SetFirst` (en los empates va
  primero el slot 0 en ambas consolas), `SetHeadless`,
  `SaveMachine`/`LoadMachine`.
- `src/Video.*`, `src/Audio.*`, `src/GearsystemCore.*`: `SetHeadless`. Sin
  píxeles ni PSG, pero se mantienen el overflow y la colisión de sprites.
- `platforms/libretro/Makefile`: `platform=miyoomini`.
- `platforms/libretro/dualtest/`: arnés con 2 consolas en un proceso y red
  en memoria, más la ROM de prueba (`makerom.py`).

Probado en x86 con el arnés: latencias de 5 y 40 ms, ROM distinta, reset,
fin de sesión y fallo forzado. Todo correcto. Coste de CPU con la ROM de
prueba en x86: unos +45 % por la máquina oculta.

Pendiente:
1. Probarlo en Miyoo Mini con RetroArch real: velocidad, ¿llega a 60 fps
   con las dos máquinas?
2. Probarlo con juegos reales con link: Columns, Tetris-likes,
   Sonic Drift, etc.
3. Compilarlo para Android (regla de ndk-build, como en gpsp) y probar
   Miyoo Mini contra Android.
4. Nota: con un compilador nativo, en el PC antiguo había que poner
   `CC=gcc CXX=g++ AR=ar`, porque el entorno apuntaba al toolchain de la
   Miyoo.

## Neo Geo Pocket (RACE, https://github.com/libretro/RACE): sin empezar

Análisis hecho:
- RACE no emula el cable. Las llamadas COM del BIOS HLE son stubs
  (`tlcs900h.c`, `doBios` casos 0x14, 0x17, 0x18, 0x19) y no existe la SIO
  del TLCS-900H (SC0BUF/SC0CR/SC0MOD en 0x50-0x53, interrupciones RX/TX).
- El estado es global en C. Para tener dos instancias en el mismo proceso
  hace falta el truco de gpsp: compilar otra vez con un define, enlazar
  parcialmente y renombrar símbolos con objcopy. En ARM usa DrZ80 (asm), que
  también hay que renombrar.
- El RTC ya es determinista en el fork de libretro (bien para el modo dual).

Plan:
1. Primero, el cable a nivel de BIOS HLE (COM init, send, receive, RTS,
   estado y buffers).
2. Después, la segunda instancia.
3. Por último, portar `libretro_dual.cpp`.
