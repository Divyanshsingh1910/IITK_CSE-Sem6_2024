make -e
./python $1
dot -Tpng file.dot -o ast.png
open ast.png
