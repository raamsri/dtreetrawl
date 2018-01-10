# dtreetrawl

`dtreetrawl` trawls/traverses directory tree or file path to collect stats of every entry in the tree.

```
Usage:
  dtreetrawl [OPTION...] "/trawl/me" [path2,...]

Help Options:
  -h, --help                Show help options

Application Options:
  -t, --terse               Produce a terse output; parsable.
  -j, --json                Output as JSON
  -d, --delim=:             Character or string delimiter/separator for terse output(default ':')
  -l, --max-level=N         Do not traverse tree beyond N level(s)
  --hash                    Enable hashing(default is MD5).
  -c, --checksum=md5        Valid hashing algorithms: md5, sha1, sha256, sha512.
  -R, --only-root-hash      Output only the root hash. Blank line if --hash is not set
  -N, --no-name-hash        Exclude path name while calculating the root checksum
  -F, --no-content-hash     Do not hash the contents of the file
  -s, --hash-symlink        Include symbolic links' referent name while calculating the root checksum
  -e, --hash-dirent         Include hash of directory entries while calculating root checksum
```

Stats of all paths passed as arguments to `dtreetrawl` is printed to the standard output, errors to standard error.
After the stats of trawled entries, a summary of that run for the root path follows. When multiple paths are given as arguments, stats continue to print to the standard output.


`tentry` fields:

 - entry/path name
 - base name of the path
 - level from root
 - file type
 - referent name if it's a symlink
 - file size in bytes
 - inode number
 - number of directory entries
 - permission in octal notation
 - hard link count
 - ownership: user and group id
 - I/O block size
 - allocated blocks
 - last status change (ctime)
 - last file access (atime)
 - last file modification (mtime)
 - hash

`dstat` fields:

 - root path name of this run
 - elapsed time in seconds
 - start time
 - root hash
 - hash type
 - depth
 - total size
 - total entries
 - total directories
 - total regular files
 - total symlinks
 - total block devices
 - total char devices
 - total sockets
 - total FIFOs/pipes

#### Human readable output

`./dtreetrawl --hash -c md5 -- /home/lab/dtreetrawl ../linux-4.14-rc8`
```
...
... //clipped
...
/home/lab/linux-4.14-rc8/CREDITS
        Base name                    : CREDITS
        Level                        : 1
        Type                         : regular file
        Referent name                :
        File size                    : 98443 bytes
        I-node number                : 290850
        No. directory entries        : 0
        Permission (octal)           : 0644
        Link count                   : 1
        Ownership                    : UID=0, GID=0
        Preferred I/O block size     : 4096 bytes
        Blocks allocated             : 200
        Last status change           : Tue, 21 Nov 17 21:28:18 +0530
        Last file access             : Thu, 28 Dec 17 00:53:27 +0530
        Last file modification       : Tue, 21 Nov 17 21:28:18 +0530
        Hash                         : 9f0312d130016d103aa5fc9d16a2437e

Stats for /home/lab/linux-4.14-rc8:
        Elapsed time     : 1.305767 s
        Start time       : Sun, 07 Jan 18 03:42:39 +0530
        Root hash        : 434e93111ad6f9335bb4954bc8f4eca4
        Hash type        : md5
        Depth            : 8
        Total,
                size           : 66850916 bytes
                entries        : 12484
                directories    : 763
                regular files  : 11715
                symlinks       : 6
                block devices  : 0
                char devices   : 0
                sockets        : 0
                FIFOs/pipes    : 0
```
#### Parsable terse output
In the terse output, each `trawlentry`/`tentry`/`tent` is surrounded by the set delimiter/separator twice. That is, say if the delimiter is `:`, then, at the begining and end of the tentry, `::` will be placed; fields within the tentry will be separated by `:`. A successful run is followed by a summary(`dstat`), to differentiate it from the tentries, it will have only a single delimiter at the begining and ending. `trawlentry` starts with a double delimiter, `dstat` starts with a single delimiter.


Fields appear in the same order as listed. Fields that are not applicable are left blank, which leads to adjacent delimiters.

After each run, a new line is printed after dstat(summary entry) to help differentiate the next run for another root path.

Units are not attached to the field values in terse output.

Time values are in UTC as epoch for terse output(local string form for human readable output).

Sizes are all in bytes.

File type notation:

        SOCK  socket
        LNK   symbolic link
        REG   regular file
        BLK   block device
        DIR   directory
        CHR   character device
        FIFO  FIFO/pipes



`./dtreetrawl --hash -t -c md5 -d ":" -- /home/lab/dtreetrawl ../linux-4.14-rc8`
```
...
... //clipped
...
::/home/lab/linux-4.14-rc8/Documentation/misc-devices/isl29003:isl29003:3:REG::1497:297459:0:0644:1:0,0:4096:8:1511279899:1514402607:1511279899:fa7521a2e480f8c623f445a1068264a3::
::/home/lab/linux-4.14-rc8/Documentation/misc-devices/lis3lv02d:lis3lv02d:3:REG::4304:297461:0:0644:1:0,0:4096:16:1511279899:1514402607:1511279899:9a65535bd8f578f80538a0d2446cf571::
::/home/lab/linux-4.14-rc8/Documentation/misc-devices/eeprom:eeprom:3:REG::4183:297457:0:0644:1:0,0:4096:16:1511279899:1514402607:1511279899:06908861c5c16bf6f917a85704b8e316::
::/home/lab/linux-4.14-rc8/Documentation/misc-devices/apds990x.txt:apds990x.txt:3:REG::3552:297454:0:0644:1:0,0:4096:8:1511279899:1514402607:1511279899:567f9502ccf3cdb4a11a7779f72bbfe3::
::/home/lab/linux-4.14-rc8/.cocciconfig:.cocciconfig:1:REG::59:261650:0:0644:1:0,0:4096:8:1511279898:1514402607:1511279898:876b4d70fb8be5f6af3257cae6a9fa87::
::/home/lab/linux-4.14-rc8/Kconfig:Kconfig:1:REG::287:298227:0:0644:1:0,0:4096:8:1511279899:1514402607:1511279899:4d682367bedf5218ad52a2cdaf2ce2fc::
::/home/lab/linux-4.14-rc8/CREDITS:CREDITS:1:REG::98443:290850:0:0644:1:0,0:4096:200:1511279898:1514402607:1511279898:9f0312d130016d103aa5fc9d16a2437e::
:/home/lab/linux-4.14-rc8:0.532712:1515276988:434e93111ad6f9335bb4954bc8f4eca4:md5:8:66850916:12484:763:11715:6:0:0:0:0:

```


#### Dependencies
Linux, GCC & GLib-2.0

#### To run the pre compiled binary, clone or fork this repo and `cd` to the directory:
`./dtreetrawl -h`

```dtreetrawl --hash -d ":" -c "sha1" -- /stairway/to/heaven /eastblue/southblue/allblue```

#### To compile:
```gcc -Wall -Wextra -o dtreetrawl dtreetrawl.c dtreetrawl.h $(pkg-config --cflags --libs glib-2.0)```

To run this by name, symlink this to one of the shell's search PATH:
```ln -s /path/to/dtreetrawl/dtreetrawl /usr/local/bin/dtreetrawl```

#### TODO:
  - Makefile
  - JSON?
  - Make it portable?
