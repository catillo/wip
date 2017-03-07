CPP_SRCS=main.cpp millis.cpp util.cpp
C_SRCS=ard.c

CPP_OBJECTS=$(subst .cpp,.o,$(CPP_SRCS))
C_OBJECTS=$(subst .c,.o,$(C_SRCS))
OBJS=${CPP_OBJECTS}
OBJS+=${C_OBJECTS}

.c.o:
	g++ -c  $< -o $@

.cpp.o:
	g++ -c  $< -o $@

corbot: ${OBJS}
		g++ -Wall -o corbot ${OBJS}

all: corbot

clean:
        rm -f corbot ${OBJS}
