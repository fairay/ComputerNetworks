CPPFLAGS=-g -pthread -I/sw/include/root
LDFLAGS=-g
LDLIBS=-L/sw/lib/root -lCore

SRCS=server.cpp client.cpp http.cpp
OBJDIR = build
OBJS=$(subst .cpp,.o,$(SRCS))
EXES=server client

all: $(OBJDIR)/server $(OBJDIR)/client

$(OBJDIR)/server: $(OBJDIR)/server.o $(OBJDIR)/http.o
	g++ $(LDFLAGS) -o $@ $^

$(OBJDIR)/client: $(OBJDIR)/client.o
	g++ $(LDFLAGS) -o $@ $^

(%.o): %.cpp
	g++ $(CPPFLAGS) -c $<

$(OBJDIR)/%.o: %.cpp
	g++ $(CPPFLAGS) -c $< -o $@

RM=rm -f
clean:
	$(RM) $(OBJDIR)/*
