# Compiler and linker flags
CXX=g++
CXXFLAGS=-std=c++20 -Wall
LDFLAGS=-lmysqlcppconn

# Path to the MySQL Connector/C++ library
MYSQL_CPPCONN_PATH=/usr/local/mysql/lib # Adjust this path according to your setup

# Target executable name
TARGET=myapp

# Source files
SRCS=main.cpp

# Object files
OBJS=$(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) -L$(MYSQL_CPPCONN_PATH)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)




# Compiler and linker flags
CXX = g++
CXXFLAGS = -Iinclude -Wall
LDFLAGS = -L/usr/local/mysql/lib -lmysqlcppconn -L. -lutilities

# Path to the MySQL Connector/C++ library
MYSQL_CPPCONN_PATH=/usr/local/mysql/lib # Adjust this path according to your setup

# Paths to source files
UTILITIES_SRC = Utilities/pch.cpp Utilities/Analytics.cpp Utilities/Commands.cpp Utilities/SqlUtils.cpp Utilities/Utilities.cpp 
CLIENT_SRC = Client/Client.cpp
SERVER_SRC = ServerBA/ProducerConsumer.cpp ServerBA/ServerBA.cpp 
QUERY_SRC = Query/Query.cpp

# Object files
LIB_OBJ = $(UTILITIES_SRC:.cpp=.o)
CLIENT_OBJ = $(CLIENT_SRC:.cpp=.o)
SERVER_OBJ = $(SERVER_SRC:.cpp=.o)
QUERY_OBJ = $(QUERY_SRC:.cpp=.o)

# Name of the library
UTILITIES_LIB = utilities.a

# Name of the executables
CLIENT_EXE = client
SERVER_EXE = serverba
QUERY_EXE = query

.PHONY: all clean utilities client server query

all: utilities client server query

utilities: $(LIB_OBJ)
	ar rcs $(UTILITIES_LIB) $^

client: $(CLIENT_OBJ)
	$(CXX) $(CLIENT_OBJ) -o $(CLIENT_EXE) $(LDFLAGS)

server: $(SERVER_OBJ)
	$(CXX) $(SERVER_OBJ) -o $(SERVER_EXE) $(LDFLAGS) -L$(MYSQL_CPPCONN_PATH)

query: $(QUERY_OBJ)
	$(CXX) $(QUERY_OBJ) -o $(QUERY_EXE) $(LDFLAGS) -L$(MYSQL_CPPCONN_PATH)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(LIB_OBJ) $(MAIN_OBJ) $(UTILITIES_LIB) $(CLIENT_EXE)
