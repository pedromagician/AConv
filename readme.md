# AConv – Amiga/Atari/Windows/ZX Conversion Utility

AConv is a command‑line tool for converting text between various retro and modern character encodings, including **Amiga**, **Atari**, **MS-DOS**, **Windows** and **ZX Spectrum** formats.

It uses a flexible dictionary‑based mapping system supporting byte sequences, Unicode strings, escape sequences, and reversible conversions.
AConv also includes a powerful encoding detection engine based on dictionary matching and text heuristics.

## ✨ Features
- Conversion modes:
  - Byte -> Byte
  - Byte -> Windows
  - Windows -> Byte
  - Windows ‑> Windows
- Dictionary‑driven mapping with support for:
  - byte sequences
  - Windows strings
  - escape sequences
  - string‑to‑string mappings (e.g., ZX BasinC \a → á)
- Automatic encoding detection (-detect) using:
  - dictionary scoring
  - text heuristics
  - n‑gram analysis
  - penalty for unknown characters
- Supports dictionary masks (e.g., `dict\*.dict`)
- Optional dictionary inversion (-invert)
- Raw byte dump mode (-decdump, -hexdump)
- Default keymap generator (-generatekeymap)
- Configurable error handling for unknown characters
- Clipboard input and output support
- Output formats:
  - ANSI
  - UTF‑8
  - UTF‑8 with BOM

## 📁 Dictionary Format

AConv uses human‑readable .dict files with the following sections:

- [meta] – name, author, version
- [settings] – number base, separator, code page, error handling
- [dictionary] – actual mapping rules

Supported mapping types:
  - byte-byte
  - byte-escape sequence
  - byte-string
  - string-byte
  - string-escape sequence
  - string-string
  - escape sequence-byte
  - escape sequence-escape sequence
  - escape sequence-string

## 🔍 Encoding Detection

AConv can automatically detect the most likely encoding of a file using:

- dictionary match scoring
- longest‑match byte scanning
- string‑pattern scanning
- text quality heuristics
- n‑gram analysis
- confidence scoring

Usage:

```CMD
AConv.exe -detect -input file.txt -dict dict\*.dict
```

Example output:

```
PC-Latin 2 => score 1090, confidence 0.74
Amiga E2 => score 1082, confidence 0.73
ZX(Win) for BasinC => score 424, confidence 0.29
```

## 🛠️ Conversion Modes

Select conversion type using -type or -t:

- b2b (Byte → Byte)
- b2w (Byte → Windows)
- w2b (Windows → Byte)
- w2w (Windows → Windows)

Example:

```CMD
AConv.exe -i input.txt -o output.txt -k amiga.dict -t w2b

AConv.exe -i input.txt -o output.txt -k amiga.dict -t b2w -ansi
```

## 📜 Command‑Line Options


| Switch | Description |
|--------|-------------|
| `-help`, `-h`, `-?` | Displays this help information. |
| `-input`, `-i` | Specifies the source file name. |
| `-output`, `-o` | Specifies the destination file name. If omitted, the output file name is generated as 'inputFileName.out'. |
| `-inputClipboard`, `-ic` | Specifies that the input data should be read from the system clipboard instead of a file. |
| `-outputClipboard`, `-oc` | Specifies that the converted output should be written to the system clipboard instead of a file. |
| `-decdump`, `-dd` | Write the value of each byte from the input file to the output file in text form as a number in decimal notation. |
| `-hexdump`, `-hd` | Write the value of each byte from the input file to the output file in text form as a number in hexadecimal notation. |
| `-detect`, `-d` | Detect the encoding of the input file. |
| `-generatekeymap`, `-g` | Generates a default character map and saves it to 'keyMap.dict'. |
| `-keymap`, `-k` | Specifies a single character map file used for conversion. This option applies only when converting. Default: 'keyMap.dict'. |
| `-dict` | Specifies dictionary files used for encoding detection. Supports masks (e.g., 'dict\*.dict') and typically expands to multiple files. |
| `-type`, `-t` | Specifies the conversion type:<br><br>Values:<br>- b2b (byte2byte)<br>- b2w (byte2windows)<br>- w2b (windows2byte)<br>- w2w (windows2windows) |
| `-invert`, `-inv` | Inverts the character map. (Useful mainly for Windows-to-Windows or Byte-to-Byte conversions) |
| `-format`, `-f` | Sets the output file format (Windows mode) to:<br><br>Values:<br>- ansi<br>- utf8 (utf8nobom)<br>- bom (utf8+bom, utf8bom, utf8withbom) |
| `-errorlevel`, `-e` | Controls how unknown characters are handled :<br><br>2(default) – Replace unknown characters with the 'error character' and display a warning<br>1 – Do not replace, display a warning only<br>0 – Do not replace and do not display warnings |
| `-errorcharacter`, `-c` | Specifies the replacement character used for unknown characters. The default replacement character is '?'. |


## 📦 Examples

1. Detect encoding

```CMD
AConv.exe -d -i text.txt -dict dict\*.dict
```

2. Convert Windows → Amiga

```CMD
AConv.exe -i story.txt -o story.ami -k amiga.dict -t w2b
```

3. Dump bytes

```CMD
AConv.exe -i file.bin -o dump.txt -decdump
```

4. Generate default keymap

```CMD
AConv.exe -g
```

## 📝 Notes

- Dictionaries are modular — you can add your own retro encodings.
- Encoding detection works best with multiple dictionaries in the dict\ folder.
- ZX BasinC escape mappings require [string-string] dictionaries.

## 📜 License

Free to use.
Retro Rulez.
