lex scan.l
gcc scan.c lex.yy.c -o scanner

for i in {1..4}; do
	./scanner < ../../Assignments/assign1/problem1/testcases/public$i.knp  > output$i.txt
	echo ""
	echo "-----------"
	echo "Input file:"
	echo "-----------"
	cat ../../Assignments/assign1/problem1/testcases/public$i.knp
	echo ""
	echo "--------------------"
	echo "Output lexeme stream"
	echo "--------------------"
	cat output$i.txt
done
