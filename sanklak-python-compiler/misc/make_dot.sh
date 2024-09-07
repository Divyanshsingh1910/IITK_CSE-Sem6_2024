#!/bin/bash
make
echo "compile done"
echo $1

if	[[ "$1" == "-r" ]]; then
	./python $2
	dot -Tpng file.dot -o ast.png
	open $2
	open ast.png
else
	./python $1
	dot -Tpng file.dot -o ast.png
fi

