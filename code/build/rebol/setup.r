REBOL [
  title:        "orx setup"
  author:       "iarwain@orx-project.org"
  date:         26-Sep-2015
  file:         %setup.r
]


; Default settings
tag:            <version>
host:           ["https://codeload.github.com/orx/orx-extern/zip/" tag]
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
  "windows"   [premake "windows"                                                              config ["gmake" "codelite" "codeblocks" "vs2015" "vs2017" "vs2019"]                                                                               env-msg "Please restart your favorite IDE before using orx."]
  "mac"       [premake "mac"                                                                  config ["gmake" "codelite" "codeblocks" "xcode4"                  ]                                                                               env-msg "Please logout/login to refresh your environment if you're using an IDE."]
  "linux"     [premake (either find to-text system/platform/2 "x64" ["linux64"] ["linux32"])  config ["gmake" "codelite" "codeblocks"                           ]   deps ["libgl1-mesa-dev" "libsndfile1-dev" "libopenal-dev" "libxrandr-dev"]  env-msg "Please logout/login to refresh your environment if you're using an IDE."]
]


; Inits
begin: now/time
skip-hook: false

switch platform: lowercase to-text system/platform/1 [
  "macintosh" [platform: "mac"]
]
platform-info: platform-data/:platform

change-dir root: system/options/path
attempt [write build-file ""]

delete-dir: function [
  "Deletes a directory including all files and subdirectories."
  dir [file! url!]
] [
  if all [
    dir? dir
    dir: dirize dir
    attempt [files: load dir]
  ] [
    for-each file files [delete-dir dir/:file]
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


  ; Should override cache?
  if not empty? system/options/args [
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
  print ["== Decompressing [" local "] => [" extern "]"]
  wait 0.5
  unzip/quiet temp local
  until [wait 0.5 attempt [rename rejoin [temp load temp] extern]]
  attempt [delete-dir temp]
  print ["== [" req-ver "] installed!"]


  ; Installs premake
  premake-path: dirize rejoin [premake-root platform-info/premake]
  premake: first read premake-path
  premake-file: read premake-path/:premake
  for-each [type folder] builds [
    if exists? folder [
      print ["== Copying [" premake "] to [" folder "]"]
      write folder/:premake premake-file
      if not platform = "windows" [
        call/shell form reduce ["chmod +x" folder/:premake]
      ]
    ]
  ]


  ; Stores version
  write cur-file req-ver
]


; Sets environment variable
new-env: (get-env env-variable) != env-path: to-text file-to-local clean-path root/:env-path
print ["== Setting environment: [" env-variable "=" env-path "]"]
set-env env-variable env-path
either platform = "windows" [
  call/shell form reduce ["setx" env-variable env-path]
] [
  env-home: local-to-file dirize get-env "HOME"
  for-each [env-file env-prefix mandatory] reduce [
    env-home/.bashrc                      rejoin ["export " env-variable "="]   true
    env-home/.profile                     rejoin ["export " env-variable "="]   true
    env-home/.zshrc                       rejoin ["export " env-variable "="]   false
    env-home/.config/fish/fish_variables  rejoin ["SETUVAR " env-variable ":"]  false
  ] [
    if any [mandatory exists? env-file] [
      parse env-content: any [attempt [to-text read env-file] copy ""] [
        thru env-prefix start: [to newline | to end] stop: (change/part start env-path stop)
      | to end start: (insert start rejoin [newline env-prefix env-path newline])
      ]
      attempt [write env-file env-content]
    ]
  ]
]


; Runs premake
premake-path: dirize rejoin [premake-root platform-info/premake]
premake: first read premake-path
print ["== Generating build files for [" platform "]"]
for-each config platform-info/config [
  print ["== Generating [" config "]"]
  for-each [type folder] builds [
    if exists? folder [
      in-dir rejoin [root folder] [
        command: rejoin ["./" premake " " config]
        either platform = "windows" [
          call command
        ] [
          call/shell command
        ]
      ]
    ]
  ]
]
print ["== You can now build orx in [" builds/code/:platform "]"]
print ["== For more details, please refer to [" https://orx-project.org/wiki/ "]"]


; Mercurial hook
if exists? hg [
  either skip-hook [
    print "== Skipping Mercurial hook installation"
  ] [
    hgrc: hg/hgrc

    either find read hgrc hg-hook [
      print "== Mercurial hook already installed"
    ] [
      print "== Installing mercurial hook"
      write/append hgrc rejoin [
        newline
        "[hooks]"
        newline
        hg-hook
        " = "
        file-to-local either hook-rel: find/tail system/options/boot root [
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
  call/shell/output {hg log -l 1 --template "{rev}"} build-version
  if not empty? build-version [
    attempt [write build-file form reduce ["#define __orxVERSION_BUILD__" build-version]]
  ]
]


; Git hooks
if exists? git [
  either skip-hook [
    print "== Skipping Git hook installation"
  ] [
    for-each hook git-hooks [
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
        hook-file: either find hook-file: to-text read hook-path system/options/script [
          _
        ] [
          hook-content
        ]
      ] [
        hook-file: rejoin [
          "#!/bin/sh"
          newline
          hook-content
        ]
      ]

      either hook-file [
        attempt [make-dir/deep first split-path hook-path]
        print ["== Installing git hook [" hook "]"]
        write/append hook-path hook-file
        if not platform = "windows" [
          call/shell form reduce ["chmod +x" hook-path]
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
  print ["==^(1b)[31m IMPORTANT - Make sure the following libraries are installed on your system^(1b)[39m:"]
  for-each lib platform-info/deps [print ["==[^(1b)[33m" lib "^(1b)[39m]"]]
]
if all [
  new-env
  find platform-info 'env-msg
] [
  print [newline "== IMPORTANT - New environment detected:" platform-info/env-msg newline]
]
end: now/time
print ["== [" (end - begin) "] Setup successful!"]
