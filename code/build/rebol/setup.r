REBOL [
  title:        {Setup}
  author:       {iarwain@orx-project.org}
  date:         26-Sep-2015
  file:         %setup.r
]

; Default settings
tag:            <version>
hosts:          [[{https://orx-project.org/extern/} tag {.zip}] [{https://codeload.github.com/orx/orx-extern/zip/} tag]]
extern:         %extern/
cache:          %cache/
temp:           %.temp/
premake-root:   dirize %extern/premake/bin
builds:         [code %code/build tutorial %tutorial/build orxfontgen %tools/orxFontGen/build orxcrypt %tools/orxCrypt/build]
hg:             %.hg/
hg-hook:        {update.orx}
git:            %.git/
git-hooks:      [%post-checkout %post-merge]
build-file:     %code/include/base/orxBuild.h
env-variable:   {ORX}
env-path:       %code
platform-data:  compose/deep [
  windows     [premake {windows}                                              config [{gmake} {codelite} {codeblocks} {vs2017} {vs2019} {vs2022}]                                               env-msg {Please restart your favorite IDE before using orx.}]
  mac         [premake {mac}                                                  config [{gmake} {codelite} {codeblocks} {xcode4}                  ]                                               env-msg {Please logout/login to refresh your environment if you're using an IDE.}]
  linux       [premake (pick [{linux64} {linux32}] system/build/arch = 'x64)  config [{gmake} {codelite} {codeblocks}                           ]   deps [{libgl1-mesa-dev} {libxrandr-dev}]    env-msg {Please logout/login to refresh your environment if you're using an IDE.}]
]

; Inits
begin: now/time
new-env: skip-env: skip-hook: false
switch platform: system/build/os [
  macos [platform: 'mac]
]
platform-info: platform-data/:platform

change-dir root: system/options/path
attempt [write build-file {}]

; Should override cache?
unless empty? args: system/options/args [
  wrap [
    digits: charset [#"0" - #"9"]
    foreach arg args [
      either arg/1 = #"-" [
        case [
          parse next arg [(level: 1) [{debug} | {dbg} | {d}] opt [{:} copy level some digits (level: min 5 load level)]] [
            print [{== Setting debug level [} level {]}]
            system/options/quiet: false
            foreach [sub _] system/options/log [system/options/log/:sub: level]
          ]
          true [
            print [{== Ignoring unknown argument [} arg {]}]
          ]
        ]
      ] [
        print [{== Overriding cache [} cache {] => [} cache: dirize to-rebol-file arg {]}]
        skip-env: skip-hook: true
      ]
    ]
  ]
]

; Checks version
req-file: %.extern
cur-file: extern/.version
req-ver: trim/all read/string req-file
cur-ver: if exists? cur-file [trim/all read/string cur-file]
print [{== Checking version: [} extern {]}]
either req-ver = cur-ver [
  print [{== [} cur-ver {] already installed, skipping!}]
] [
  print [{== [} req-ver {] needed, current [} cur-ver {]}]

  ; Updates cache
  either exists? local: rejoin [cache req-ver %.zip] [
    print [{== [} req-ver {] found in cache!}]
  ] [
    attempt [make-dir/deep cache]
    print [{== [} req-ver {] not in cache}]
    system/schemes/http/spec/timeout: system/schemes/https/spec/timeout: 60
    either foreach host hosts [
      either attempt [
        print [{== Fetching [} remote: to-url replace rejoin host tag req-ver {]} newline {== Please wait!}]
        write root/:local read remote
      ] [break/return true] [print [{== Not found!}]]
      false
    ] [
      print [{== [} req-ver {] cached!}]
    ] [
      print [{== [} req-ver {] not found, aborting!}]
      quit
    ]
  ]

  ; Clears current version
  if exists? extern [
    print [{== Deleting [} extern {]}]
    until [wait 0.5 delete-dir extern not exists? extern]
  ]

  ; Decompresses
  delete-dir temp
  print [{== Decompressing [} local {] => [} extern {]}]
  wait 0.5
  foreach [file data] load local [
    either dir? file [
      mkdir/deep temp/:file
    ] [
      write temp/:file data/2
    ]
  ]
  until [wait 0.5 attempt [rename rejoin [temp load temp] extern]]
  delete-dir temp
  print [{== [} req-ver {] installed!}]

  ; Installs premake
  premake-path: dirize rejoin [premake-root platform-info/premake]
  premake: first read premake-path
  premake-file: read premake-path/:premake
  foreach [type folder] builds [
    if exists? folder [
      print [{== Copying [} premake {] to [} folder {]}]
      write folder/:premake premake-file
      unless platform = 'windows [
        call/wait/shell form reduce [{chmod +x} folder/:premake]
      ]
    ]
  ]

  ; Stores version
  write cur-file req-ver
]

; Sets environment variable
either skip-env [
  print {== Skipping environment setup}
] [
  new-env: (get-env env-variable) != env-path: to-string to-local-file clean-path root/:env-path
  print [{== Setting environment: [} env-variable {=} env-path {]}]
  set-env env-variable env-path
  either platform = 'windows [
    call/wait/shell/output form reduce [{setx} env-variable env-path] none
  ] [
    env-home: to-rebol-file dirize get-env {HOME}
    foreach [env-file env-prefix mandatory] reduce [
      env-home/.profile                     rejoin [{export } env-variable {=}]   true
      env-home/.bashrc                      rejoin [{export } env-variable {=}]   true
      env-home/.bash_profile                rejoin [{export } env-variable {=}]   false
      env-home/.zshrc                       rejoin [{export } env-variable {=}]   false
      env-home/.zprofile                    rejoin [{export } env-variable {=}]   false
      env-home/.config/fish/fish_variables  rejoin [{SETUVAR } env-variable {:}]  false
    ] [
      if any [mandatory exists? env-file] [
        parse env-content: any [attempt [to-string read env-file] copy {}] [
          thru env-prefix start: [to newline | to end] stop: (change/part start env-path stop)
        | to end start: (insert start rejoin [newline env-prefix env-path newline])
        ]
        attempt [write env-file env-content]
      ]
    ]
  ]
]

; Runs premake
premake-path: dirize rejoin [premake-root platform-info/premake]
premake: first read premake-path
print [{== Generating build files for [} platform {]}]
foreach config platform-info/config [
  print [{== Generating [} config {]}]
  foreach [type folder] builds [
    if exists? folder [
      in-dir rejoin [root folder] [
        command: rejoin [{./} premake { } config]
        either platform = 'windows [
          call/wait/output command none
        ] [
          call/wait/shell/output command none
        ]
      ]
    ]
  ]
]
print [{== You can now build orx in [} builds/code/:platform {]}]
print [{== For more details, please refer to [ https://orx-project.org/wiki/ ]}]

; Mercurial hook
if exists? hg [
  either skip-hook [
    print {== Skipping Mercurial hook installation}
  ] [
    either find to-string read hgrc: hg/hgrc hg-hook [
      print {== Mercurial hook already installed}
    ] [
      print {== Installing mercurial hook}
      write/append hgrc rejoin [
        newline
        {[hooks]}
        newline
        hg-hook { = } to-local-file any [find/tail system/options/boot root system/options/boot] { } any [find/tail system/options/script root system/options/script]
        newline
      ]
    ]
  ]

  ; Creates build file
  build-version: copy {}
  call/wait/shell/output {hg log -l 1 --template "{rev}"} build-version
  unless empty? build-version [
    attempt [write build-file form reduce [{#define __orxVERSION_BUILD__} build-version]]
  ]
]

; Git hooks
if exists? git [
  either skip-hook [
    print {== Skipping Git hook installation}
  ] [
    foreach hook git-hooks [
      hook-content: rejoin [
        newline
        any [find/tail system/options/boot root system/options/boot] { } git-script: any [find/tail system/options/script root system/options/script]
        newline
      ]
      hook-path: git/hooks/:hook
      either all [
        exists? hook-path
        not empty? read hook-path
      ] [
        hook-file: unless find to-string read hook-path git-script [hook-content]
      ] [
        hook-file: rejoin [
          {#!/bin/sh}
          newline
          hook-content
        ]
      ]

      either hook-file [
        attempt [make-dir/deep first split-path hook-path]
        print [{== Installing git hook [} hook {]}]
        write/append hook-path hook-file
        unless platform = 'windows [
          call/wait/shell form reduce [{chmod +x} hook-path]
        ]
      ] [
        print [{== Git hook [} hook {] already installed}]
      ]
    ]
  ]

  ; Creates build file
  build-version: copy {}
  call/wait/shell/output {git rev-list --count HEAD} build-version
  unless empty? trim/all build-version [
    attempt [write build-file form reduce [{#define __orxVERSION_BUILD__} build-version]]
  ]
]

; Done!
if find platform-info 'deps [
  print newline
  print [{==^(1b)[31m IMPORTANT - Make sure the following libraries (or equivalent) are installed on your system^(1b)[39m:}]
  foreach lib platform-info/deps [print [{==[^(1b)[33m} lib {^(1b)[39m]}]]
]
all [
  new-env
  find platform-info 'env-msg
  print [newline {==^(1b)[32m IMPORTANT - New environment detected^(1b)[39m:} platform-info/env-msg newline]
]
end: now/time
print [{== [} (end - begin) {] Setup successful!}]
