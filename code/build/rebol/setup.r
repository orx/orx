REBOL [
  title:        "orx setup"
  author:       "iarwain@orx-project.org"
  date:         26-Sep-2015
  file:         %setup.r
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
env-variable:   "ORX"
env-path:       %code
platform-data:  compose/deep [
  "windows"   [premake "windows"                                                                config ["gmake" "codelite" "codeblocks" "vs2013" "vs2015" "vs2017"]                                                                             env-msg "Please restart your favorite IDE before using orx."]
  "mac"       [premake "mac"                                                                    config ["gmake" "codelite" "codeblocks" "xcode4"                  ]                                                                             env-msg "Please logout/login to refresh your environment if you're using an IDE."]
  "linux"     [premake (either find to-string system/platform/2 "x64" ["linux64"] ["linux32"])  config ["gmake" "codelite" "codeblocks"                           ]   deps ["freeglut3-dev" "libsndfile1-dev" "libopenal-dev" "libxrandr-dev"]  env-msg "Please logout/login to refresh your environment if you're using an IDE."]
]


; Inits
begin: now/time
skip-hook: false

switch platform: lowercase to-string system/platform/1 [
  "macintosh" [platform: "mac"]
]
platform-info: platform-data/:platform

change-dir root: system/options/path
attempt [write build-file ""]

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
  _
]
print ["== Checking version: [" extern "]"]
either req-ver = cur-ver [
  print ["== [" cur-ver "] already installed, skipping!"]
] [
  print ["== [" req-ver "] needed, current [" cur-ver "]"]


  ; Updates cache
  unless empty? system/options/args [
    print ["== Overriding cache [" cache "] => [" cache: dirize to-file system/options/args/1 "]"]
    skip-hook: true
  ]


  ; Updates cache
  local: rejoin [cache req-ver %.zip]
  remote: replace rejoin host tag req-ver
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
  loop-until [wait 0.5 attempt [rename rejoin [temp load temp] extern]]
  attempt [delete-dir temp]
  print ["== [" req-ver "] installed!"]


  ; Installs premake
  premake-path: dirize rejoin [premake-root platform-info/premake]
  premake: first read premake-path
  premake-file: read premake-path/:premake
  foreach [type folder] builds [
    if exists? folder [
      print ["== Copying [" premake "] to [" folder "]"]
      write folder/:premake premake-file
      unless platform = "windows" [
        call/shell/wait reform ["chmod +x" folder/:premake]
      ]
    ]
  ]


  ; Stores version
  write cur-file req-ver
]


; Sets environment variable
new-env: (get-env env-variable) != env-path: to-string to-local-file clean-path root/:env-path
print ["== Setting environment: [" env-variable "=" env-path "]"]
set-env env-variable env-path
either platform = "windows" [
  call/shell/wait reform ["setx" env-variable env-path]
] [
  env-home: to-rebol-file dirize get-env "HOME"
  env-prefix: rejoin ["export " env-variable "="]
  env-files: reduce [
    env-home/.bashrc
    env-home/.profile
  ]
  foreach env-file env-files [
    env-content: either exists? env-file [to-string read env-file] [copy ""]
    parse env-content [
      thru env-prefix start: [to newline | to end] stop: (change/part start env-path stop)
      | to end start: (insert start rejoin [newline env-prefix env-path newline])
    ]
    attempt [write env-file env-content]
  ]
]


; Runs premake
premake-path: dirize rejoin [premake-root platform-info/premake]
premake: first read premake-path
print ["== Generating build files for [" platform "]"]
foreach config platform-info/config [
  print ["== Generating [" config "]"]
  foreach [type folder] builds [
    if exists? folder [
      in-dir rejoin [root folder] [
        command: rejoin ["./" premake " " config]
        either platform = "windows" [
          call/wait command
        ] [
          call/shell/wait command
        ]
      ]
    ]
  ]
]
print ["== You can now build orx in [" builds/code/:platform "]"]


; Mercurial hook
if exists? hg [
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

  ; Creates build file
  build-version: copy ""
  call/shell/wait/output {hg log -l 1 --template "{rev}"} build-version
  unless empty? build-version [
    attempt [write build-file reform ["#define __orxVERSION_BUILD__" build-version]]
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
          _
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
          call/shell/wait reform ["chmod +x" hook-path]
        ]
      ] [
        print ["== Git hook [" hook "] already installed"]
      ]
    ]
  ]
]


; Done!
if find platform-info 'deps [
  print newline
  print ["== IMPORTANT - Make sure the following libraries are installed on your system:"]
  foreach lib platform-info/deps [print ["==[" lib "]"]]
  print newline
]
all [
  new-env
  find platform-info 'env-msg
  print [newline "== IMPORTANT - New environment detected:" platform-info/env-msg newline]
]
end: now/time
print ["== [" (end - begin) "] Setup successful!"]
