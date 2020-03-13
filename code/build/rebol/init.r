REBOL [
  title: {Init}
  author: {iarwain@orx-project.org}
  date: 15-Aug-2017
  file: %init.r
]

; Variables
source: %../template/
extern: %../../../extern/
params: [
   name       {Project name (relative or full path)}   _
]
platforms:  [
  {windows}   [config [{gmake} {codelite} {codeblocks} {vs2015} {vs2017} {vs2019}]    premake %premake4.exe   setup %setup.bat    script %init.bat    ]
  {mac}       [config [{gmake} {codelite} {codeblocks} {xcode4}                  ]    premake %premake4       setup %./setup.sh   script %./init.sh   ]
  {linux}     [config [{gmake} {codelite} {codeblocks}                           ]    premake %premake4       setup %./setup.sh   script %./init.sh   ]
]
templates: [
  name
  date
  code-path
]

; Helpers
delete-dir: function [
  {Deletes a directory including all files and subdirectories.}
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
log: func [
  message [text! block!]
  /only
  /no-break
] [
  if not only [
    prin [{[} now/time {] }]
  ]
  either no-break [prin message] [print reeval message]
]
to-template: func [
  {Gets template literal}
  name [word! text!]
] [
  rejoin [{[} name {]}]
]

; Inits
change-dir root: system/options/path
code-path: {..}
date: to-text now/date
switch platform: lowercase to-text system/platform/1 [
  {macintosh} [platform: {mac} code-path: file-to-local root/code]
]
platform-info: platforms/:platform
premake-source: rejoin [%../ platform-info/premake]

; Usage
usage: func [
  /message [block! text!]
] [
  if message [
    prin {== }
    print reeval message
    print {}
  ]

  prin [{== Usage:} file-to-local clean-path rejoin [system/options/script/../../../.. "/" platform-info/script]]

  print rejoin [
    newline newline
    for-each [param desc default] params [
      prin rejoin [{ } either default [rejoin [{[} param {]}]] [param]]
      rejoin [{  = } param {: } desc either default [rejoin [{=[} default {], optional}]] [{, required}] newline]
    ]
  ]
  quit
]

; Processes params
either system/options/args [
  either (length? system/options/args) > ((length? params) / 3) [
    usage/message [{Too many arguments:} mold system/options/args]
  ] [
    use [interactive? arg] [
      if interactive?: zero? length? arg: system/options/args [
        print {== No argument, switching to interactive mode}
      ]
      for-each [param desc default] params [
        case [
          not tail? arg [
            set param arg/1
            arg: next arg
          ]
          interactive? [
            until [
              any [
                not empty? set param trim ask rejoin [{ * } desc {? }]
                set param default
              ]
            ]
          ]
          default [
            set param default
          ]
          true [
            usage/message [{Not enough arguments:} mold system/options/args]
          ]
        ]
      ]
    ]
  ]
] [
  usage
]

; Locates source
source: clean-path rejoin [first split-path system/options/script source]

; Runs setup if premake isn't found
if not exists? source/:premake-source [
  log [{New orx installation found, running setup!}]
  attempt [delete-dir source/:extern]
  in-dir source/../../.. [
    call/shell platform-info/setup
  ]
]

; Retrieves project name
if dir? name: clean-path local-to-file name [clear back tail name]

; Inits project directory
either exists? name [
  log [{[} file-to-local name {] already exists, overwriting!}]
] [
  make-dir/deep name
]
change-dir name/..
set [path name] split-path name
log [{Initializing [} name {] in [} file-to-local path {]}]

; Copies all files
log {== Creating files:}
build: _
reeval copy-files: function [
  from [file!]
  to [file!]
] [
  for-each file read from [
    src: from/:file
    dst: replace to/:file to-template 'name name
    if file = %build/ [
      set 'build dst
    ]
    either dir? src [
      make-dir/deep dst
      copy-files src dst
    ] [
      log/only [{  +} file-to-local dst]
      buffer: read src
      for-each template templates [
        replace/all buffer to-template template get template
      ]
      write dst buffer
    ]
  ]
] source name

; Creates build projects
if build [
  in-dir build [
    write platform-info/premake read source/:premake-source
    if not platform = {windows} [
      call/shell form reduce [{chmod +x} platform-info/premake]
    ]
    log [{Generating build files for [} platform {]:}]
    for-each config platform-info/config [
      log/only [{  *} config]
      call/shell rejoin [{"} file-to-local clean-path platform-info/premake {" } config]
    ]
  ]
]

; Ends
log {Init successful!}
