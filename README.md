# Group-5-Programming-A

## Compile

To compile the program, use the following command:

```
gcc predictor2.c -o predictor2.exe -lm
```

## How to use

### Using a CSV File:

Run the program by giving the file name as an argument:

```
predictor2.exe data.csv
```

### Using Standard Input (stdin):

You can also use pipe or redirect:

```
type data.csv | predictor2.exe
```

or

```
predictor2.exe < data.csv
```

> **Note:** Replace `data.csv` with the name of the file you are using.

## Plotting Feature with GNUPlot

This program has visualization features using **GNUPlot**, if installed on your system.

### How to install GNUPlot

1. Download GNUPlot from the following link:
   [https://sourceforge.net/projects/gnuplot/](https://sourceforge.net/projects/gnuplot/)

2. Add the GNUPlot directory to the system **PATH** variable so that it can be run from the terminal:
   ![Add GNUPlot PATH](https://i.imgur.com/mQd9m93.png)

3. Complete the installation process as usual.

4. To check if GNUPlot is installed correctly, run:

   ```
   gnuplot --version
   ```

If the above command displays the version, then GNUPlot is successfully installed.

---
