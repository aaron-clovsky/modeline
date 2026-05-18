# Yet Another Modeline Calculator

## DESCRIPTION

Yet Another Modeline Calculator is a C89 library for modeline calculation.

## CREDIT

Based on [Switchres](https://github.com/antonioginer/switchres) by Chris Kennedy, Antonio Giner, Alexandre Wodarczyk and Gil Delescluse

## MOTIVATION

[Switchres](https://github.com/antonioginer/switchres) is able to calculate modelines from minimal information (width, height and refresh rate) as well as adjust the geometry of  the calculated modeline (horizontal size, horizontal shift and vertical shift) through its ```--calc``` and ```--geometry``` options respectively .

If you need a tool for modeline calculation you should use [Switchres](https://github.com/antonioginer/switchres).

If you need a lightweight C/C++ library for modeline calculation you should consider using this library.

Switchres is rather large and includes a lot of code to do many other things, while this library includes only calculation code and reduces the entire interface to a single function mimicking a call to the ```switchres``` command line utility.

This library is also C89 compatible for maximum portability, and has been lightly retouched to be more readable.

Currently only a single monitor is defined: ```MODELINE_MONITOR_GENERIC_15KHZ```, which This is based on ```"generic_15"```.

## NOTES

- ```make style``` is implemented to keep code within style guidelines,
it requires clang-format-21, to install run:
```
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 21
sudo apt install clang-format-21
```

- ```make lint``` is implemented to help check for warnings it
requires: gcc, g++, clang, clang++ and cppcheck

## LICENSE
This software is licensed under [GPLv3](https://www.gnu.org/licenses/gpl-3.0.html).
