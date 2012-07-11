CPP=g++

zoov:
	$(CPP) zoov.cpp -std=c++0x -DNDEBUG -O3 -I. -I/usr/include/opencv  -lml -lhighgui -lcv -lcxcore  -I./ -lpthread -lSDL -lvlc -lSDL_ttf -o zoov


clean:
	rm -rf *.o zoov
