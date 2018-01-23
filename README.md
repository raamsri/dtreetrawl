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
  -f, --follow-symlink      Follow symbolic links
  -T, --no-tent             Output only the summary(dstat), no other entries
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
 - follow symlink
 - hash exclude name
 - hash exclude content
 - hash symlink ref name
 - hash directory entry names
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

`dtreetrawl -f -F --hash .`
```
...
... //clipped
...
/home/lab/dtreetrawl/README.md
	Base name                    : README.md
	Level                        : 1
	Type                         : regular file
	Referent name                : 
	File size                    : 7483 bytes
	I-node number                : 305946
	No. directory entries        : 0
	Permission (octal)           : 0644
	Link count                   : 1
	Ownership                    : UID=0, GID=0
	Preferred I/O block size     : 4096 bytes
	Blocks allocated             : 16
	Last status change           : Sat, 20 Jan 18 14:33:11 +0530
	Last file access             : Sun, 21 Jan 18 21:42:24 +0530
	Last file modification       : Sat, 20 Jan 18 14:33:11 +0530
	Hash                         : 

Stats for /home/lab/dtreetrawl:
	Elapsed time                 : 0.063725 s
	Start time                   : Mon, 22 Jan 18 17:07:57 +0530
	Follow Symlink               : yes
	Exclude name hash            : no
	Exclude content hash         : yes
	Hash symlink ref name        : no
	Hash directory entry names   : no
	Root hash                    : 6ddf338bc4fabe53a5eadf8bf8ff26d8
	Hash type                    : md5
	Depth                        : 6
	Size                         : 851355 bytes
	Entries                      : 241
	Directories                  : 103
	Regular files                : 138
	Symlinks                     : 0
	Block devices                : 0
	Char devices                 : 0
	Sockets                      : 0
	FIFOs/pipes                  : 0
```

#### JSON output

`dtreetrawl -f -F --hash -j .`
```
...
... //clipped
...
			},
			{
				"path":		"/home/lab/dtreetrawl/README.md",
				"basename":	"README.md",
				"level":	"1",
				"type":		"regular file",
				"refname":	"",
				"size":		"6680 bytes",
				"inode":	"304693",
				"ndirent":	"0",
				"permission":	"0644",
				"nlink":	"1",
				"ownership":	"uid:0, gid:0",
				"blksize":	"4096 bytes",
				"nblocks":	"16",
				"ctime":	"Mon, 22 Jan 18 17:12:25 +0530",
				"atime":	"Mon, 22 Jan 18 17:12:25 +0530",
				"mtime":	"Mon, 22 Jan 18 17:12:25 +0530",
				"hash":		""
			},
		{}
		],
		"dstat": [
			{
				"root_path":			"/home/lab/dtreetrawl",
				"elapsed":			"0.090811",
				"start_time":			"Mon, 22 Jan 18 17:17:45 +0530",
				"follow_symlink":		"yes",
				"hash_exclude_name":		"no",
				"hash_exclude_content":		"yes",
				"hash_symlink":			"no",
				"hash_dirent":			"no",
				"hash":				"6ddf338bc4fabe53a5eadf8bf8ff26d8",
				"hash_type":			"md5",
				"nlevel":			"6",
				"nsize":			"850712 bytes",
				"nentry":			"241",
				"ndir":				"103",
				"nreg":				"138",
				"nlnk":				"0",
				"nblk":				"0",
				"nchr":				"0",
				"nsock":			"0",
				"nfifo":			"0"
			},
...
... //clipped
...
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



`dtreetrawl -f -F -t -d ":" --hash .`
```
...
... //clipped
...
::/home/lab/dtreetrawl/Makefile:Makefile:1:REG::498:305832:0:0644:1:0,0:4096:8:1515853283:1516553956:1515853283:::
::/home/lab/dtreetrawl/dtreetrawl:dtreetrawl:1:REG::44424:304696:0:0755:1:0,0:4096:88:1516620913:1516620917:1516620913:::
::/home/lab/dtreetrawl/README.md:README.md:1:REG::7463:304693:0:0644:1:0,0:4096:16:1516621215:1516621215:1516621215:::
:/home/lab/dtreetrawl:0.013780:1516621221:yes:no:yes:no:no:6ddf338bc4fabe53a5eadf8bf8ff26d8:md5:6:851335:241:103:138:0:0:0:0:0:

```


#### Dependencies
Linux, GCC & GLib-2.0

#### To compile and install from source(preferred):
```
make clean
make
make install

# To uninstall
make uninstall
```

#### To install from RPM(centos):
```
# Pull the RPM
wget https://github.com/six-k/dtreetrawl/releases/download/dtreetrawl-1.0/dtreetrawl-1.0-1.el7.centos.x86_64.rpm

# Pull the public key
wget https://github.com/six-k/dtreetrawl/releases/download/dtreetrawl-1.0/RPM-GPG-KEY-dtreetrawl

# Verify signature
rpm --import ./RPM-GPG-KEY-dtreetrawl
rpm -K -v ./dtreetrawl-1.0-1.el7.centos.x86_64.rpm
# should get: dtreetrawl-1.0-1.el7.centos.x86_64.rpm: rsa sha1 (md5) pgp md5 OK

# Install
yum localinstall dtreetrawl-1.0-1.el7.centos.x86_64.rpm
```

#### To run the pre compiled binary, clone or fork this repo and `cd` to the directory:
`./dtreetrawl -h`

```./dtreetrawl --hash -d ":" -c "sha1" -- /stairway/to/heaven /eastblue/southblue/allblue```

To run this by name, symlink this to one of the shell's search PATH:
```ln -s /path/to/dtreetrawl/dtreetrawl /usr/local/bin/dtreetrawl```

#### Acknowledgements
Folks at [Fractalio](https://fractalio.com) have been kind enough to test this
tool at production sites and reporting bugs.
[Purushotham](https://github.com/purushotham-s) & 
[Gugan](https://github,com/GuGu910) have been a huge help.
