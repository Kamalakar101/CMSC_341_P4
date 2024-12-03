CXX = g++
CXXFLAGS = -Wall -g

proj4: filesys.o mytest.cpp
		$(CXX) $(CXXFLAGS) filesys.o mytest.cpp -o proj4

filesys.o: filesys.cpp  filesys.h
		$(CXX) $(CXXFLAGS) -c filesys.cpp

run:
		./proj4

val:
		valgrind ./proj4

##Use this to clean out your directory
clean:
		rm *~
		rm *.

##Use this when you want to debug your whole project
debug1:
		gdb gdbarg1 --args ./proj4

##Use this when you want to submit your files (from your project directory)
submit:
		cp filesys.h filesys.cpp mytest.cpp ~/cs341proj/proj4