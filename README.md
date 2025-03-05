# Minesweeper Memory Monitor & Overlay

A real-time Minesweeper memory scanner that reads the game’s board from memory and displays it in two modes:

1. Console Mode: Continuously updates the Minesweeper board in the terminal.
2. Overlay Mode: Draws a transparent overlay on the Minesweeper window, marking mine locations.

This project uses Windows API for memory reading and GDI+ for rendering a real-time overlay.

## Features

- Reads Minesweeper board state directly from memory
- Displays live minefield updates in the terminal
- Transparent overlay highlights mines on-screen
- Efficient and optimized memory scanning
- Works with the classic Windows Minesweeper (winmine.exe)


## Demo

### Console Mode Example

```
Minesweeper Monitor - Live View
Width: 9, Height: 9

Minesweeper Board:
........
.1*1....
.12.....
.1*1....
........
```

### Overlay Mode Example

![image](https://github.com/user-attachments/assets/e3d2834d-09e1-4e2e-84c0-051dacfce47e)


## How It Works

The program reads Minesweeper's memory using `ReadProcessMemory()`, then:

- Retrieves game dimensions from memory offsets
- Extracts tile values and mine locations
- Displays the board in the console
- In overlay mode, draws X marks over mines using GDI+

## Code Structure

### Project Layout

```
/src
 ├── MemoryUtils.h/.cpp     # Process memory reading utilities
 ├── MinefieldMonitor.h/.cpp # Console-based board display
 ├── Overlay.h/.cpp         # GDI+ overlay for marking mines
 ├── main.cpp               # Program entry point
 ├── README.md              # This file
```

## Notes

- This only works with classic Minesweeper (winmine.exe) from older Windows versions.
- Running this program requires admin privileges to read Minesweeper’s memory.
- The overlay does not interact with the game; it only visualizes data.

## License

This project is open-source under the MIT License.

Contributions welcome! Feel free to fork and improve the project.

