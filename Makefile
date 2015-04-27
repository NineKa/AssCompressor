CC=clang++
H=./src/AssCompressor.h ./src/BilibiliCommentManager.h ./src/RapidXML.h ./src/Struct.h
CPP=./src/AssCompressor.cpp ./src/BilibiliCommentManager.cpp ./src/AssCompressorCLI.cpp
#FLAG=-Wall -Wextra -ggdb
FLAG=-std=c++11 -O3 -Wall -Wextra -D_withMultiThread -D_WIN32_Font -D_withColorOutput
LIB=-lcurl
EXE=AssCompressorCLI

$(EXE):$(H) $(CPP)
	$(CC) $(FLAG) $(CPP) $(LIB) -o $(EXE)

clean:
	rm $(EXE) $(EXE).exe
