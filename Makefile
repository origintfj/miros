SHELL := /bin/bash

all:
	pushd kernel ; make ; popd
	pushd apps ; make ; popd
	pushd fat32 ; make ; popd
	cp fat32/fat32fs.x .

clean:
	pushd kernel ; make clean ; popd
	pushd apps ; make clean ; popd
	pushd fat32 ; make clean ; popd
