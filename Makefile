########################################################################
#
# Makefile: Makefile for axpdfium
#
#     Copyright (C) 2015 Yak! / Yasutaka ATARASHI
#
#     This software is distributed under the terms of a zlib/libpng
#     License.
#
#     $Id: d0f2b92c2e64c3ecc337917efe561e377d1fb69b $
#
########################################################################

NAME=axpdfium
VER=0_01

.PHONY: bump dist release tag dtag retag release clean

# FIXME: implement
#bump:
#	./verbump.sh $(VER) $(NAME).{txt,rc,cpp}

dist: Release/$(NAME).spi
	-rm -rf source source.zip $(NAME)-$(VER).zip disttemp
	mkdir source
	git archive --worktree-attributes master | tar xf - -C source
	(cd source; zip -r ../source.zip *)
	-rm -rf source
	mkdir disttemp
	cp Release/$(NAME).spi $(NAME).txt source.zip disttemp
	cp -R LICENSES disttemp
	(cd disttemp; zip -r ../$(NAME)-$(VER).zip *)
	-rm -rf disttemp

tag:
	git tag $(NAME)-$(VER)

dtag:
	-git tag -d $(NAME)-$(VER)

retag: dtag tag

release:
	make bump
	-git a -u
	-git commit -m 'Released as v'$(subst _,.,$(VER))'.'
	make tag strip dist

clean:
	-rm -rf *.o *.a *.spi *.ro release
