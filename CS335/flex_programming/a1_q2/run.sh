
flex scan.l
gcc scan.c lex.yy.c -o scanner
echo "----------------"
echo "    Input       "
echo "----------------"
echo ""
cat $1
echo ""
echo "----------------"
echo "    Output      "
echo "----------------"
echo ""
./scanner < $1

