REBOL [
    title:      "orx setup"
    author:     "iarwain@orx-project.org"
    date:       26-Sep-2015
    file:       %setup.r
]


; Default settings
tag:            "<version>"
host:           rejoin ["https://bitbucket.org/orx/orx-extern/get/" tag ".zip"]
extern:         %extern/
cache:          %cache/
temp:           %.temp/
premake-root:   dirize extern/premake/bin
builds:         ['code %code/build 'tutorial %tutorial/build 'orxfontgen %tools/orxFontGen/build 'orxcrypt %tools/orxCrypt/build]
hg:             %.hg/
hg-hook:        "update.orx"
git:            %.git/
git-hooks:      [%post-checkout %post-merge]
platform-data:  [
    "windows"   ['premake "windows" 'config ["gmake" "codelite" "vs2012" "vs2013"] 'hgrc %hgrc                                                                           ]
    "mac"       ['premake "mac"     'config ["gmake" "codelite" "xcode4"         ] 'hgrc %.hgrc                                                                          ]
    "linux"     ['premake "linux32" 'config ["gmake" "codelite"                  ] 'hgrc %.hgrc 'deps ["freeglut3-dev" "libsndfile1-dev" "libopenal-dev" "libxrandr-dev"]]
]


; Inits
begin: now/time
skip-hook: false
platform: lowercase to-string system/platform/1
if platform = "macintosh" [platform: "mac"]
platform-info: platform-data/:platform

change-dir system/options/home

delete-dir: func [
    {Deletes a directory including all files and subdirectories.}
    dir [file! url!]
    /local files
] [
    if all [
        dir? dir
        dir: dirize dir
        attempt [files: load dir]
    ] [
        foreach file files [delete-dir dir/:file]
    ]
    attempt [delete dir]
]


; Checks version
req-file: %.extern
cur-file: extern/.version
req-ver: read/lines req-file
cur-ver: either exists? cur-file [
    read/lines cur-file
] [
    none
]
either req-ver = cur-ver [
    print ["== [" cur-ver "] already installed, skipping!"]
] [
    print ["== [" req-ver "] needed, current [" cur-ver "]"]


    ; Updates cache
    if system/options/args [
        print ["== Overriding cache [" cache "] => [" cache: dirize to-file system/options/args/1 "]"]
        skip-hook: true
    ]


    ; Updates cache
    local: rejoin [cache req-ver '.zip]
    remote: replace host tag req-ver
    either exists? local [
        print ["== [" req-ver "] found in cache!"]
    ] [
        attempt [make-dir/deep cache]
        print ["== [" req-ver "] not in cache"]
        print ["== Fetching [" remote "]" newline "== Please wait!"]
        write system/options/home/:local read to-url remote
        print ["== [" req-ver "] cached!"]
    ]


    ; Clears current version
    if exists? extern [
        print ["== Deleting [" extern "]"]
        attempt [delete-dir extern]
    ]


    ; Decompresses
    do system/script/path/rebzip.r
    attempt [delete-dir temp]
    print ["== Decompressing [" local "] => [" extern "]"]
    wait 0.5
    unzip/quiet temp local
    until [wait 0.5 attempt [rename rejoin [temp load temp] extern]]
    attempt [delete-dir temp]
    print ["== [" req-ver "] installed!"]


    ; Installs premake
    premake-path: dirize rejoin [premake-root platform-info/premake]
    premake: read premake-path
    premake-file: read premake-path/:premake
    forskip builds 2 [
        print ["== Copying [" premake "] to [" builds/2 "]"]
        write builds/2/:premake premake-file
        if not platform = "windows" [
            call reform ["chmod +x" builds/2/:premake]
        ]
    ]


    ; Stores version
    write cur-file req-ver
]


; Runs premake
premake-path: dirize rejoin [premake-root platform-info/premake]
premake: read premake-path
print ["== Generating build files for [" platform "]"]
foreach config platform-info/config [
    print ["== Generating [" config "]"]
    forskip builds 2 [
        change-dir rejoin [system/options/home builds/2]
        call/wait rejoin ["./" premake " " config]
    ]
]
change-dir system/options/home
print ["== You can now build orx in [" builds/code/:platform "]"]


; Mercurial hook
if exists? hg [
    either skip-hook [
        print "== Skipping Mercurial hook installation"
    ] [
        hgrc: rejoin [hg platform-info/hgrc]
        hgrc-file: to-string read hgrc

        either find hgrc-file hg-hook [
            print "== Mercurial hook already installed"
        ] [
            print "== Installing mercurial hook"
            write hgrc append hgrc-file rejoin [
                newline
                "[hooks]"
                newline
                hg-hook
                " = "
                to-local-file either hook-rel: find/tail system/options/boot system/options/home [
                    hook-rel
                ] [
                    system/options/boot
                ]
                " "
                system/options/script
                newline
            ]
        ]
    ]
]


; Git hooks
if exists? git [
    either skip-hook [
        print "== Skipping Git hook installation"
    ] [
        foreach hook git-hooks [
            hook-content: rejoin [
                newline
                either hook-rel: find/tail system/options/boot system/options/home [
                    hook-rel
                ] [
                    system/options/boot
                ]
                " "
                system/options/script
                newline
            ]
            hook-path: git/hooks/:hook
            either all [
                exists? hook-path
                not empty? read hook-path
            ] [
                hook-file: either find hook-file: to-string read hook-path system/options/script [
                    none
                ] [
                    append hook-file hook-content
                ]
            ] [
                hook-file: rejoin [
                    "#!/bin/sh"
                    newline
                    hook-content
                ]
            ]

            either hook-file [
                print ["== Installing git hook [" hook "]"]
                write hook-path hook-file
                if not platform = "windows" [
                    call reform ["chmod +x" hook-path]
                ]
            ] [
                print ["== Git hook [" hook "] already installed"]
            ]
        ]
    ]
]


; Done!
if platform-info/deps [
    print newline
    print ["== IMPORTANT - Make sure the following libraries are installed on your system:"]
    foreach lib platform-info/deps [print ["==[" lib "]"]]
    print newline
]
end: now/time
print ["== [" (end - begin) "] Setup successfull!"]
