CXX = g++ -static

all: blank.exe blankd.exe blanko.exe blankod.exe blanka.exe blankad.exe blankn.exe blanknd.exe \
     test.exe testd.exe testo.exe testod.exe testa.exe testad.exe testn.exe \
     macro.exe macrod.exe macroo.exe macrood.exe macroa.exe macroad.exe macron.exe

clean:
	-rm -rf *.o *.exe *.a

result.txt: clean all
	ls -l *.exe > result.txt

blank.exe: blank.cpp odstream.cpp
	$(CXX) -Wall -O2 -o $@ $^
blankd.exe: blank.cpp odstream.cpp
	$(CXX) -Wall -O2 -o $@ -DDEBUG $^

blankn.exe: blank.cpp
	$(CXX) -Wall -O2 -o $@ $^
blanknd.exe: blank.cpp
	$(CXX) -Wall -O2 -o $@ -DDEBUG $^

blanko.exe: blank.cpp odstream.o
	$(CXX) -Wall -O2 -o $@ $^
blankod.exe: blank.cpp odstream.o
	$(CXX) -Wall -O2 -o $@ -DDEBUG $^

blanka.exe: blank.cpp libodstream.a
	$(CXX) -Wall -O2 -o $@ $^
blankad.exe: blank.cpp libodstream.a
	$(CXX) -Wall -O2 -o $@ -DDEBUG $^

libodstream.a: odstream.o
	$(AR) r $@ $^

test.exe: test.cpp odstream.cpp
	$(CXX) -Wall -O2 -o $@ $^
testd.exe: test.cpp odstream.cpp
	$(CXX) -Wall -O2 -o $@ -DDEBUG $^

testn.exe: test.cpp
	$(CXX) -Wall -O2 -o $@ $^
testnd.exe: test.cpp
	$(CXX) -Wall -O2 -o $@ -DDEBUG $^

testo.exe: test.cpp odstream.o
	$(CXX) -Wall -O2 -o $@ $^
testod.exe: test.cpp odstream.o
	$(CXX) -Wall -O2 -o $@ -DDEBUG $^

testa.exe: test.cpp libodstream.a
	$(CXX) -Wall -O2 -o $@ $^
testad.exe: test.cpp libodstream.a
	$(CXX) -Wall -O2 -o $@ -DDEBUG $^

macro.exe: macro.cpp odstream.cpp
	$(CXX) -Wall -O2 -o $@ $^
macrod.exe: macro.cpp odstream.cpp
	$(CXX) -Wall -O2 -o $@ -DDEBUG $^

macron.exe: macro.cpp
	$(CXX) -Wall -O2 -o $@ $^
macrond.exe: macro.cpp
	$(CXX) -Wall -O2 -o $@ -DDEBUG $^

macroo.exe: macro.cpp odstream.o
	$(CXX) -Wall -O2 -o $@ $^
macrood.exe: macro.cpp odstream.o
	$(CXX) -Wall -O2 -o $@ -DDEBUG $^

macroa.exe: macro.cpp libodstream.a
	$(CXX) -Wall -O2 -o $@ $^
macroad.exe: macro.cpp libodstream.a
	$(CXX) -Wall -O2 -o $@ -DDEBUG $^
