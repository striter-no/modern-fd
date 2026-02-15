all:
	gcc -o ./bin/main ./code/main.c -Icode/mfd/include -Lcode/mfd/lib
run:
	./bin/main
clean:
	rm -rf ./bin/*
