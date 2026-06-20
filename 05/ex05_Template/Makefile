MPIC=mpic++

.PHONY: build
build: ./bin/ring_allreduce

.PHONY: clean
clean:
	rm -f ./bin/*

.PHONY: rebuild
rebuild: clean build

./bin/ring_allreduce: ./src/ring_allreduce.cpp
	$(MPIC) -std=c++20 -O3 -o $@ $^
