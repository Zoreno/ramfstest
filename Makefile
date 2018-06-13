##
# Project Title
#
# @file
# @version 0.1

all: create_image read

create_fs: create.c
	gcc create.c -o create_fs

create_image: create_fs
	./create_fs ramfs.img testfile testfile2 create.c dummy

read: read.c
	gcc read.c -o read

clean:
	rm -f create_fs

# end
