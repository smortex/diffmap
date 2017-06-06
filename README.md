# diffmap

Display different blocks between raw-data files.

## Build

Checkout the code, then run CMake and Make:

	% mkdir build
	% cd build
	% cmake ..
	% make

Installation is not required, run `make install` if you want to install to some
standard location, or run `./diffmap` from the source directory.

## Usage

Visually spot different blocks in binary files (files have the same size and
modification date, but one is corrupt).  Each dot represent a block (default
512 bytes, can be changed using e.g. `-b 4096`) holding the same data in each
file, a "X" represent a block with different data.  If both file have different
size, leading "/" or "\" are displayed for each additional blocks.

	% diffmap /tmp/arabxetex.doc.tar.xz /var/jails/texlive.home.sigabrt.org/home/romain/freebsd-texlive/distfiles/mirror/arabxetex.doc.tar.xz
	\\\ /tmp/arabxetex.doc.tar.xz
	/// /var/jails/texlive.home.sigabrt.org/home/romain/freebsd-texlive/distfiles/mirror/arabxetex.doc.tar.xz
	................................................................................
	................................................................................
	................................................................................
	................................................................................
	................................................................................
	................................................................................
	................................................................................
	........................X.......................................................
	...........

By default, `diffmap` will check that it compares two different files and skip
useless checks.  Under some circumstances, for example when comparing versions
of the same file in two snapshots of a ZFS filesystem, this behaviour can be
problematic and can be avoided using `-D`:

	% diffmap -b 1024000 /home/.zfs/snapshot/{2013-11-21_00.10.00--5d,2013-11-25_00.10.00--5d}/romain/mbox                                              
	diffmap: "/home/.zfs/snapshot/2013-11-21_00.10.00--5d/romain/mbox" and "/home/.zfs/snapshot/2013-11-25_00.10.00--5d/romain/mbox" are the same file.  Skipping comparison.

	% diffmap -Db 1024000 /home/.zfs/snapshot/{2013-11-21_00.10.00--5d,2013-11-25_00.10.00--5d}/romain/mbox
	\\\ /home/.zfs/snapshot/2013-11-21_00.10.00--5d/romain/mbox
	/// /home/.zfs/snapshot/2013-11-25_00.10.00--5d/romain/mbox
	................................................................................
	................................................................................
	..........................................................................//////
	///

Here, we can see that this mailbox has grown by ~10 MB between these two snapshots.
