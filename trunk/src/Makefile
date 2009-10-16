CC?=gcc

OBJ=main.o server.o topology.o
PROTOCOL_OBJ=protocol/text.o
PROGRAM=pubsubsrv

all: ${OBJ} ${PROTOCOL_OBJ}
	${CC} ${OBJ} ${PROTOCOL_OBJ} -o ${PROGRAM}
clean:
	rm -vf ${OBJ} ${PROTOCOL_OBJ} ${PROGRAM}
