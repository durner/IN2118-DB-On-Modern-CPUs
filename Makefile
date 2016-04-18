clean:
			rm -rf ./bin

gen:
			@mkdir -p bin
			g++ -g -O3 -Werror utils/gen.cpp -o bin/gen

sort:
			@mkdir -p bin
			g++ -Wall -march=native -std=c++11 -g -O3 -o bin/sort src/external_merge_sort.cpp
