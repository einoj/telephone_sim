all: telephone_sim.cpp
	g++ telephone_sim.cpp -o telephone_sim -lm -lSDL2 --std=c++2b
run: all
	./telephone_sim
clean:
	rm telephone_sim
