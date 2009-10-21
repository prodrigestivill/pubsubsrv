include ../config.mk

OBJ=main.o server.o topology.o
PROTOCOL_OBJ=$(addsuffix .o, $(basename $(wildcard protocol/*.c)))
PROGRAM=pubsubsrv

all: ${OBJ} ${PROTOCOL_OBJ}
	${CC} ${OBJ} ${PROTOCOL_OBJ} -o ${PROGRAM}
clean:
	rm -vf ${OBJ} ${PROTOCOL_OBJ} ${PROGRAM}
