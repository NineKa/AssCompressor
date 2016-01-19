CC = clang -std=c++11 -stdlib=libc++
lib = -lcurl -lboost_regex -lboost_filesystem -lboost_system -lc++
lib_path = 
include_path = -I./src/

FLAG = -DCXX11REGEX -DDEBUG -Wall -Wextra

CXX = ./src/AssCompressorCLI.cc
LIBCXX = ./src/assConverterLib.cc
OBJ = AssCompressorCLI

$(OBJ) : $(CXX)
	$(CC) $(lib_path) $(include_path) $(lib) $(CXX) -o $(OBJ)

lib : $(LIBCXX)
	$(CC) $(include_path) $(FLAG) -c $(LIBCXX)
	$(CC) $(lib_path) $(lib) $(FLAG) -shared assConverterLib.o -o assConverterLib.dylib
	rm assConverterLib.o
	cp ./docs/library.xml ./

clean:
	rm $(OBJ) assConverterLib.dylib library.xml