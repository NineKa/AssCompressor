CURL-CONFIG-LIBS = -L/usr/local/Cellar/curl/7.47.1/lib -lcurl -lldap -lz
CURL-CONFIG-CFLAGS = -I/usr/local/Cellar/curl/7.47.1/include

CC = clang -std=c++11 -stdlib=libc++
lib = -lboost_regex -lboost_filesystem -lboost_system -lc++ $(CURL-CONFIG-LIBS)
lib_path = -L/usr/local/Cellar/boost/1.60.0_1/lib 
include_path = -I/usr/local/Cellar/boost/1.60.0_1/include  $(CURL-CONFIG-CFLAGS) -I./src/

FLAG = -DCXX11REGEX -DDEBUG -Wall -Wextra

CXX = ./src/AssCompressorCLI.cc
LIBCXX = ./src/AssConverterLib.cc
OBJ = AssCompressorCLI

$(OBJ) : $(CXX)
	$(CC) $(lib_path) $(include_path) $(lib) $(CXX) -o $(OBJ)

clean:
	rm $(OBJ) 