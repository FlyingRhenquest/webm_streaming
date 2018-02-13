CPPFLAGS += --std=c++11
CPPFLAGS += -I../cppcodec/cppcodec
CPPFLAGS += -I../served/src
LIBFLAGS += ../served/lib/libserved.a
LIBFLAGS += -lre2 -lboost_system -lpthread

objs = server.o

.c.o:
	g++ -c ${CPPFLAGS} $<

all: ${objs}
	g++ -o server ${objs} $(LIBFLAGS)
	mkdir images

clean:
	rm -rf server *~ ${objs} images
