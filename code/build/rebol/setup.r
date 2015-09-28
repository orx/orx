REBOL [
    title:      "orx setup"
    author:     "iarwain@orx-project.org"
    date:       26-Sep-2015
    file:       %setup.r
]


; Default settings
host:           https://bitbucket.org/orx/orx-extern/get/
extern:         %extern/
cache:          %cache/
temp:           %.temp/


; Misc
change-dir system/options/home

delete-dir: func [
    {Deletes a directory including all files and subdirectories.}
    dir [file! url!]
    /local files
][
    if all [
        dir? dir
        dir: dirize dir
        attempt [files: load dir]
    ][
        foreach file files [delete-dir dir/:file]
    ]
    attempt [delete dir]
]


; Up-to-date?
req-file: %.extern
cur-file: extern/.version
req-ver: read/lines req-file
cur-ver: either exists? cur-file [
    read/lines cur-file
][
    none
]
either req-ver = cur-ver [
    print ["== [" cur-ver "] already present, quitting!"]
    quit
][
    print ["== [" req-ver "] needed, current [" cur-ver "]"]
]


; Updates host
if system/options/args [
    host: to-url system/options/args/1
]


; Updates cache
local: rejoin [cache req-ver '.zip]
remote: rejoin [host req-ver '.zip]
either exists? local [
    print ["== [" req-ver "] found in cache!"]
][
    attempt [make-dir cache]
    print ["== [" req-ver "] not in cache"]
    print ["== Downloading from [" host "], please wait!"]
    call reform [to-local-file system/options/boot system/script/path/download.r remote system/options/home/:local]
    while [not exists? local][
        prin "."
        wait 0.5
    ]
    print newline
    print ["== [" req-ver "] cached!"]
]


; Clears current version
if exists? extern [
    print ["== Deleting [" extern "]"]
    attempt [delete-dir extern]
]


; Decompress
do system/script/path/rebzip.r
attempt [delete-dir temp]
print ["== Decompressing [" local "] => [" extern "]"]
wait 0.5
unzip/quiet temp local
wait 0.5
rename rejoin [temp load temp] extern
attempt [delete-dir temp]


; Stores version
write cur-file req-ver
print ["== [" req-ver "] installed!"]
