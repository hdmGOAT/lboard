build: out_dir
	echo "Building lboard"
	gcc -o out/lboard src/main.c

out_dir:
	mkdir out

clean: 
	rm out/*
