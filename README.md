# extfs4irix

At the moment, this is simply a proof of concept, the goal is to first write a primitive ext2 file browser/reader in userspace on IRIX/any POSIX system, and then to eventually build a kernel module for IRIX to be able to natively mount and read/write ext2 filesystems.


To build, simply download and run make all, then to run use ./bin/ext2impl (path to ext2 image)