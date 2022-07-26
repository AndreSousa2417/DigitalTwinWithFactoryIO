execmake : mes.cpp open62541.o
		gcc mes.cpp open62541.o -o exec -lstdc++
