TARGET		:= buse_test
LIBOBJS 	:= buse.o
OBJS		:= $(TARGET:=.o) $(LIBOBJS)
STATIC_LIB	:= libbuse.a

CC		:= /usr/bin/g++
CFLAGS		:= -std=c++98 -Wall -pedantic -Werror -D_DEBUG -g
LDFLAGS		:= -L. -lbuse -lboost_system -lboost_thread

.PHONY: all clean
all: $(TARGET)

$(TARGET): %: %.o $(STATIC_LIB)
	$(CC) -o $@ $< $(LDFLAGS)

$(TARGET:=.o): %.o: %.cpp buse.hpp
	$(CC) $(CFLAGS) -o $@ -c $<

$(STATIC_LIB): $(LIBOBJS)
	ar rcu $(STATIC_LIB) $(LIBOBJS)

$(LIBOBJS): %.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(TARGET) $(OBJS) $(STATIC_LIB)
