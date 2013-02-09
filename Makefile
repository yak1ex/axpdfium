all: libodstream.a

clean:
	-rm -rf *.o *.a

libodstream.a: odstream.o
	$(AR) r $@ $^
