TARGET_DNS_STUB = DNS_Stub
TARGET_DNS_PERF = DNS_Perf
TARGET_DNS_MODULE = libdns.a

DNS = ./DNSmodule

.PHONY: all clean install

all: $(TARGET_DNS_MODULE) $(TARGET_DNS_STUB) $(TARGET_DNS_PERF)

DNSConstants.o: $(DNS)/DNSConstants.c
	gcc -c $(DNS)/DNSConstants.c

List.o: $(DNS)/List.c $(DNS)/List.h
	gcc -c $(DNS)/List.c $(DNS)/List.h

Query.o: $(DNS)/Query.c $(DNS)/Query.h
	gcc -c $(DNS)/Query.c $(DNS)/Query.h

$(TARGET_DNS_MODULE): Query.o List.o DNSConstants.o
	ar rc libdns.a Query.o List.o DNSConstants.o

DNSStubServer.o: DNSStubServer.c
	gcc -c DNSStubServer.c

DNSPerf.o: DNSPerf.c
	gcc -c DNSPerf.c

$(TARGET_DNS_STUB): DNSStubServer.o $(TARGET_DNS_MODULE)
	gcc DNSStubServer.c -L. -ldns -o $(TARGET_DNS_STUB)

$(TARGET_DNS_PERF): DNSPerf.o $(TARGET_DNS_MODULE)
	gcc DNSPerf.c -L. -ldns -o $(TARGET_DNS_PERF)

clean:
	rm -rf *.o
	rm -rf *.gch
	rm ./$(TARGET_DNS_PERF) ./$(TARGET_DNS_STUB) $(TARGET_DNS_MODULE)

install:
	mkdir bin
	install ./$(TARGET_DNS_PERF) ./bin
	install ./$(TARGET_DNS_STUB) ./bin
