# CS335: COMPILER DESIGN



## Group Information

```
#Group 26
```

## Project Members

| Name    | Roll | EmailID |
| -------- | ------- | ------- |
| Divyansh | 210355 | divyansh21@iitk.ac.in |               
| Rajeev Kumar | 210815 | rajeevks21@iitk.ac.in |             
| Sandeep Nitharwal | 210921 | nsandeep21@iitk.ac.in

                 

TA-`Vinay Agrawal`

# SANKLAk : A python Compiler

## Abstract

**SANKLAK** is a compiler for statically typed subset of python programming language. The lexer and the parser for this compiler is written using `Flex` and `Bison` utility while the semantic actions, helper functions and linkages are implemented in `C++`.

## Running Instructions

### Generating dot output
All the source codes are present in the `src` directory. Running the `Makefile` generate the executable `python`. When provided with an input python program, the following commands will generate an output dot file in the `src/` directory:
```
$ cd src
$ make
$ ./python -i test.py -o output.dot
```
Following command line arguments are supported by the executable:
```
Usage: ./python [options]     

Commands:
----------
-h, --help 					             Show help page
-i, --input <input_file_name> 			 Give input file
-o, --output <output_file_name>			 Redirect dot file to output file
-v, --verbose 					         Outputs the entire derivation in command line
```

The input python program can also be passed directly without the `-i` or `--input` flag. By default, the generated output file is named `file.dot`.

### Generating the AST graph

Further, to generate `svg/png/pdf` graph of the AST tree, run the following command:
```
$ dot -T<svg/png/pdf> output.dot -o AST.<svg/png/pdf>
```
***
A successful execution example can be:

```
$ cd src
$ make
$ ./python test.py
$ dot -Tpdf file.dot -o AST.pdf
$ make clean
```
