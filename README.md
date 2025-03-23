# Calcatro
This repository contains the source code and compiled binary for a Ti 84+ CE adopted version of [Balatro](https://playbalatro.com/) game developed for the TI-84 Plus CE graphing calculator using the [CEdev](https://ce-programming.github.io/toolchain/) toolchain. The game is written in C and compiled to run natively on the calculator.

---

## Requirements

- **TI-84 Plus CE Graphing Calculator**
- **TI Connect CE Software** (Official software from Texas Instruments)
- **CEdev Toolchain** (for compiling source code, optional if you're only installing)

---

## Installation Instructions

### Step 1: Download and Install TI Connect CE

If you do not already have it, download the TI Connect CE software from the official TI website:

- https://education.ti.com/en/products/computer-software/ti-connect-ce-software

Install it according to the instructions for your operating system (Windows/macOS).

---

### Step 2: Connect Your Calculator

1. Plug your TI-84 Plus CE into your computer using a USB-to-mini-USB cable.
2. Launch the TI Connect CE software.
3. Ensure the calculator appears in the **Connected Calculators** list. If not, troubleshoot using TI's help documentation.

---

### Step 3: Transfer the Game

1. Download or compile the `.8xp` program file from this repository. The compiled game is  located in the `bin/` directory.
2. In TI Connect CE, click on the **Calculator Explorer** tab (the icon looks like a calculator).
3. Drag the `.8xp` file into the Explorer window, or use the **Actions > Send to Calculators** option.
4. Choose the archive location (`RAM` or `Archive`). Use `Archive` to prevent loss of program when `RAM` is cleared.
5. Click **Send**. Wait until the file transfer completes.

---

## Running the Game on the Calculator Using [Cesium](https://github.com/mateoconlechuga/cesium/)

1. Use Cesium to run the game.
2. Do not forget to archive the game

---

## Development and Compilation (Optional)

If you wish to build the game from source:

1. Set up [CEdev](https://ce-programming.github.io/toolchain/).
2. Clone this repository.
3. Run `make` in the project root to build the binary.
4. The output `.8xp` file will be located in the `bin/` directory.

---

## License

[MIT](LICENSE)

---

## Disclaimer

This software is not affiliated with or endorsed by Texas Instruments. Use at your own risk. Ensure you understand how to reset your calculator in the event of crashes or instability.
