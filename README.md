# SIC/XE Asemmbler

## Environment

- OS: MacOS
- Language: Apple clang version 14.0.0

## How To Use

### Compile

```bash
cd scripts && sh build.sh
```

or you can use `make clean && make` command instead

### Run

```bash
cd scripts && sh run.sh
```

or you can use `build/main` to run with options

The default input file is `./src/input.asm` and the result will be in `./results` folder.

### Options

- `-h` : show help
- `-i [input file]` : input file
- `--dir [output directory]` : output directory
- `-p` : print all instructions

## Example

```bash
build/main -i ./src/input.asm -p
```

## Input File

```asm
COPY START 0
FIRST STL RETADR
  LDB #LENGTH
  BASE LENGTH
CLOOP +JSUB RDREC
  LDA LENGTH
  COMP #0
  JEQ ENDFIL
  +JSUB WRREC
  J CLOOP
ENDFIL LDA EOF
  STA BUFFER
  LDA #3
  STA LENGTH
  +JSUB WRREC
  J @RETADR
EOF  BYTE C'EOF'
RETADR RESW 1
LENGTH RESW 1
BUFFER RESB 4096
RDREC CLEAR X
  CLEAR A
  CLEAR S
  +LDT #4096
RLOOP TD INPUT
  JEQ RLOOP
  RD INPUT
  COMPR A,S
  JEQ EXIT
  STCH BUFFER,X
  TIXR T
  JLT RLOOP
EXIT STX LENGTH
  RSUB
INPUT BYTE X'F1'
WRREC CLEAR X
  LDT LENGTH
WLOOP TD OUTPUT
  JEQ WLOOP
  LDCH BUFFER,X
  WD OUTPUT
  TIXR T
  JLT WLOOP
  RSUB
OUTPUT BYTE X'05'
  END FIRST
```

## Output

```asm
Compiling ../src/input.asm
        Symbol Table        
Name            Value
COPY            0
FIRST           0
CLOOP           6
ENDFIL          1A
EOF             2D
RETADR          30
LENGTH          33
BUFFER          36
RDREC           1036
RLOOP           1040
EXIT            1056
INPUT           105C
WRREC           105D
WLOOP           1062
OUTPUT          1076
        Object Code        
H^COPY  ^000000^001077
T^000000^20^17202D^69202D^4B101036^032026^290000^332007^4B10105D^3F2FEC^032010^0F2016
T^000020^10^010003^0F200D^4B10105D^3E2003^454F46
T^001036^1B^B410^B400^B440^75101000^E32019^332FFA^DB2013^A004^332008^57C003
T^001051^1A^B850^3B2FEA^134000^4F0000^F1^B410^774000^E32011^332FFA^53C003
T^00106B^0C^DF2008^B850^3B2FEF^4F0000^05
M^000007^05
M^000014^05
M^000027^05
E^000000
Done!
Check the output files in the results folder.
```
