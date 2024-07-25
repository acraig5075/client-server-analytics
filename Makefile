# Compiler and linker flags
CXX = g++
CXXFLAGS = -std=c++20 -I/usr/include -I/usr/local/include -Wall
LDFLAGS = -L/usr/lib -L/usr/local/lib -Lbin -lmysqlcppconn -l:utilities.a
LDFLAGS_QUERY = -L/usr/lib -lmysqlcppconn

# Paths to source files
UTILITIES_SRC = Utilities/pch.cpp Utilities/Commands.cpp Utilities/Analytics.cpp Utilities/SqlUtils.cpp
CLIENT_SRC = Client/Client.cpp
SERVER_SRC = ServerBA/ProducerConsumer.cpp ServerBA/ServerBA.cpp 
QUERY_SRC = Query/Query.cpp

# Object files
LIB_OBJ = $(UTILITIES_SRC:.cpp=.o)
CLIENT_OBJ = $(CLIENT_SRC:.cpp=.o)
SERVER_OBJ = $(SERVER_SRC:.cpp=.o)
QUERY_OBJ = $(QUERY_SRC:.cpp=.o)

# Name of the library
UTILITIES_LIB = bin/utilities.a

# Name of the executables
CLIENT_EXE = bin/client
SERVER_EXE = bin/serverba
QUERY_EXE = bin/query

.PHONY: all clean utilities client server query

all: utilities client server query

utilities: $(LIB_OBJ)
	ar rcs $(UTILITIES_LIB) $^

client: $(CLIENT_OBJ)
	$(CXX) $(CLIENT_OBJ) -o $(CLIENT_EXE) $(LDFLAGS)

server: $(SERVER_OBJ)
	$(CXX) $(SERVER_OBJ) -o $(SERVER_EXE) $(LDFLAGS)

query: $(QUERY_OBJ)
	$(CXX) $(QUERY_OBJ) -o $(QUERY_EXE) $(LDFLAGS_QUERY)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(LIB_OBJ) $(CLIENT_OBJ) $(SERVER_OBJ) $(QUERY_OBJ) $(UTILITIES_LIB) $(CLIENT_EXE) $(SERVER_EXE) $(QUERY_EXE)
