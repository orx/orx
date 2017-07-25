REBOL [
    title:      "orx setup"
    author:     "iarwain@orx-project.org"
    date:       26-Sep-2015
    file:       %setup.r
]


; Default settings
tag:            <version>
host:           ["https://bitbucket.org/orx/orx-extern/get/" tag ".zip"]
extern:         %extern/
cache:          %cache/
temp:           %.temp/
premake-root:   dirize extern/premake/bin
builds:         [code %code/build tutorial %tutorial/build orxfontgen %tools/orxFontGen/build orxcrypt %tools/orxCrypt/build]
hg:             %.hg/
hg-hook:        "update.orx"
git:            %.git/
git-hooks:      [%post-checkout %post-merge]
build-file:     %code/include/base/orxBuild.h
platform-data:  [
    "windows"   [premake "windows" config ["gmake" "codelite" "vs2013" "vs2015" "vs2017"]                                                                          ]
    "mac"       [premake "mac"     config ["gmake" "codelite" "xcode4"                  ]                                                                          ]
    "linux"     [premake "linux32" config ["gmake" "codelite"                           ] deps ["freeglut3-dev" "libsndfile1-dev" "libopenal-dev" "libxrandr-dev"]]
]


; Inits
begin: now/time
skip-hook: false
platform: lowercase to-string system/platform/1
switch platform [
    "macintosh" [platform: "mac"]
    "linux"     [if find to-string system/platform/2 "x64" [platform-data/:platform/premake: "linux64"]]
]
platform-info: platform-data/:platform

root: system/options/path
change-dir root

delete-dir: func [
    "Deletes a directory including all files and subdirectories."
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
print ["== Checking version: [" extern "]"]
print either req-ver = cur-ver [
    ["== [" cur-ver "] already installed, skipping!"]
] [
    ["== [" req-ver "] needed, current [" cur-ver "]"]


    ; Updates cache
    if system/options/args [
        print ["== Overriding cache [" cache "] => [" cache: dirize to-file system/options/args/1 "]"]
        skip-hook: true
    ]


    ; Updates cache
    local: rejoin [cache req-ver %.zip]
    remote: to-string replace reduce host tag req-ver
    either exists? local [
        print ["== [" req-ver "] found in cache!"]
    ] [
        attempt [make-dir/deep cache]
        print ["== [" req-ver "] not in cache"]
        print ["== Fetching [" remote "]" newline "== Please wait!"]
        write root/:local read to-url remote
        print ["== [" req-ver "] cached!"]
    ]


    ; Clears current version
    if exists? extern [
        print ["== Deleting [" extern "]"]
        attempt [delete-dir extern]
    ]


    ; Decompresses
    attempt [delete-dir temp]
    do system/script/path/rebzip.r
    print ["== Decompressing [" local "] => [" extern "]"]
    wait 0.5
    unzip/quiet temp local
    until [wait 0.5 attempt [rename rejoin [temp load temp] extern]]
    attempt [delete-dir temp]
    print ["== [" req-ver "] installed!"]


    ; Installs premake
    premake-path: dirize join premake-root platform-info/premake
    premake: read premake-path
    premake-file: read premake-path/:premake
    foreach [type folder] builds [
        if exists? folder [
            print ["== Copying [" premake "] to [" folder "]"]
            write folder/:premake premake-file
            unless platform = "windows" [
                call reform ["chmod +x" folder/:premake]
            ]
        ]
    ]


    ; Stores version
    write cur-file req-ver
]


; Runs premake
premake-path: dirize join premake-root platform-info/premake
premake: read premake-path
print ["== Generating build files for [" platform "]"]
foreach config platform-info/config [
    print ["== Generating [" config "]"]
    foreach [type folder] builds [
        if exists? folder [
            change-dir rejoin [root folder]
            call/wait rejoin ["./" premake " " config]
            change-dir root
        ]
    ]
]
print ["== You can now build orx in [" builds/code/:platform "]"]


; Mercurial hook
either exists? hg [
    either skip-hook [
        print "== Skipping Mercurial hook installation"
    ] [
        hgrc: hg/hgrc
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
                to-local-file either hook-rel: find/tail system/options/boot root [
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
    ; Removes build file
    if exists? build-file [
        attempt [delete build-file]
    ]
] [
    ; Creates build file placeholder
    unless exists? build-file [
        attempt [write build-file ""]
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
                either hook-rel: find/tail system/options/boot root [
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
                unless platform = "windows" [
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
print ["== [" (end - begin) "] Setup successful!"]
