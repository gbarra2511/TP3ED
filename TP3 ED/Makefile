CXX      = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Iinclude -O2

SRCDIR = src
OBJDIR = obj
BINDIR = bin
TARGET = $(BINDIR)/tp3.out

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJDIR)/*.o $(TARGET)

# Build instrumentado para a analise experimental (mesmo codigo-fonte,
# compilado com -DTP_STATS). NAO usar na submissao: use 'make all'.
stats:
	$(MAKE) clean
	$(MAKE) CXXFLAGS="-std=c++11 -Wall -Wextra -Iinclude -O2 -DTP_STATS"

.PHONY: all clean stats
