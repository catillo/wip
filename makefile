CPP_SRCS=main.cpp millis.cpp util.cpp ard.cpp
C_SRCS=

CPP_OBJECTS=$(subst .cpp,.o,$(CPP_SRCS))
C_OBJECTS=$(subst .c,.o,$(C_SRCS))
OBJS=${CPP_OBJECTS}
OBJS+=${C_OBJECTS}

.c.o:
	gcc -DLINUX_COMPILE -Wall -Werror -g -c  $< -o $@

.cpp.o:
	g++ -DLINUX_COMPILE -Wall -Werror -g -c  $< -o $@

corbot: ${OBJS}
		g++ -g -Werror -Wall -o corbot ${OBJS}

all: corbot

clean:
	rm -f corbot ${OBJS}
