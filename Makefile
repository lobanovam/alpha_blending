all:
	g++ -c -mavx2 -Ofast alphablend.cpp -o alphablend.o 
	g++ alphablend.o -o sfml-app -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
	./sfml-app

clear:
	rm -f *.o