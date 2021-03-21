REBOL [
  title: {Init}
  author: {iarwain@orx-project.org}
  date: 15-Aug-2017
  file: %init.r
]

; Variables
params: [
  name        {Project name (relative or full path)}                                  _       _
  archive     {orxArchive support (resources can be stored inside ZIP files)}         -       []
  c++         {Create a C++ project instead of C}                                     +       []
  imgui       {Dear ImGui support (https://github.com/ocornut/imgui)}                 -       [+c++]
  nuklear     {Nuklear support (https://github.com/immediate-mode-ui/nuklear)}        -       []
  scroll      {C++ convenience layer with config-object binding}                      -       [+c++]
]
platforms:  [
  {windows}   [config [{gmake} {codelite} {codeblocks} {vs2015} {vs2017} {vs2019}]    premake %premake4.exe   setup %setup.bat    script %init.bat    ]
  {mac}       [config [{gmake} {codelite} {codeblocks} {xcode4}                  ]    premake %premake4       setup %./setup.sh   script %./init.sh   ]
  {linux}     [config [{gmake} {codelite} {codeblocks}                           ]    premake %premake4       setup %./setup.sh   script %./init.sh   ]
]
source-path: %../template/
extern: %../../../extern/

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
extension?: function [
  {Is an extension?}
  name [word! text!]
] [
  attempt [
    result: false
    switch third find params to-word name [
      '+ '- [result: true]
    ]
  ]
  result
]
apply-template: function [
  {Replaces all templates with their content}
  content [text! binary!]
] [
  for-each [var condition] [
    template    [not extension? entry]
    +extension  [all [extension? entry get entry]]
    -extension  [all [extension? entry not get entry]]
  ] [
    set var append copy [{-=dummy=-}] collect [for-each entry templates [if do bind condition binding-of 'entry [keep reduce ['| to-text entry]]]]
  ]
  clean-chars: charset [#"0" - #"9" #"a" - #"z" #"A" - #"Z" #"_"]
  template-rule: [(sanitize: no) begin-template: {[} opt [{!} (sanitize: yes)] copy value template {]} end-template: (
      value: copy get load trim value
      if sanitize [parse value [some [clean-chars | char: skip (change char #"_")]]]
      end-template: change/part begin-template value end-template
    ) :end-template
  ]
  in-bracket: charset [not #"]"]
  bracket-rule: [{[} any [bracket-rule | in-bracket] {]}]
  extension-rule: [
    begin-extension:
    remove [
      {[} (erase: no)
      some [
        [ [ {+} -extension | {-} +extension] (erase: yes)
        | [ {+} +extension | {-} -extension]
        ]
        [{ } | {^M^/} | {^/}]
      ]
    ]
    any
    [ template-rule
    | bracket-rule
    | remove {]} end-extension: break
    | skip
    ]
    if (erase) opt [if (full-line) remove opt [{^M^/} | {^/}]] (remove/part begin-extension end-extension) :begin-extension
  ]
  parse content [
    (full-line: yes)
    any
    [ extension-rule
    | template-rule
    | {^/} (full-line: yes)
    | skip (full-line: no)
    ]
  ]
  content
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
templates: append collect [
  for-each [param desc default deps] params [keep param]
] [date code-path]

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

  for-each [param desc default deps] params [
    prin rejoin [
      { }
      case [
        extension? param [
          rejoin [{[+/-} param {]}]
        ]
        default [
          rejoin [{[} param {]}]
        ]
        true [
          param
        ]
      ]
    ]
  ]
  print [newline]
  for-each [param desc default deps] params [
    print rejoin [
      {  - } param {: } desc
      case [
        extension? param [
          rejoin [{=[} either default = '+ [{yes}] [{no}] {]} either empty? deps [{}] [rejoin [{, triggers [} deps {]}]] {, optional}]
        ]
        default [
          rejoin [{=[} default {], optional}]
        ]
        true [
          {, required}
        ]
      ]
    ]
  ]
  quit
]

; Processes params
either all [
  system/options/args
  not find system/options/args {help}
  not find system/options/args {-h}
  not find system/options/args {--help}
] [
  use [interactive? args value +extensions -extensions] [
    +extensions: copy [] -extensions: copy []
    either interactive?: zero? length? args: copy system/options/args [
      print {== No argument, switching to interactive mode}
      for-each [param desc default deps] params [
        either extension? param [
          until [
            any [
              empty? value: ask rejoin [{ * [Extension] } param {: } desc {? (} either default = '+ [{yes}] [{no}] {)}]
              logic? value: get load trim value
            ]
          ]
          append either logic? value [
            either value [+extensions] [-extensions]
          ] [
            either default = '+ [+extensions] [-extensions]
          ] param
        ] [
          until [
            any [
              not empty? set param trim ask rejoin [{ * } desc {? }]
              set param default
            ]
          ]
        ]
      ]
    ] [
      for-each [param desc default deps] params [
        case [
          extension? param [
            use [extension] [
              case [
                extension: find args rejoin ['+ param] [
                  append +extensions param
                  remove extension
                ]
                extension: find args rejoin ['- param] [
                  append -extensions param
                  remove extension
                ]
                true [
                  if default = '+ [append +extensions param]
                ]
              ]
            ]
          ]
          not tail? args [
            set param args/1
            args: next args
          ]
          true [
            usage/message [{Not enough arguments:} mold system/options/args]
          ]
        ]
      ]
      if not tail? args [
        usage/message [{Too many arguments:} mold system/options/args]
      ]
    ]

    ; Handles extensions dependencies
    use [test-extensions extension-group extension] [
      until [
        test-extensions: copy +extensions
        for-each param test-extensions [
          for-each dep fourth find params param [
            extension-group: either #"+" = first to-text dep [+extensions] [-extensions]
            extension: to-word next to-text dep
            if not find extension-group extension [
              append extension-group extension
              log [{== [} param {] triggers [} dep {]}]
            ]
          ]
        ]
        test-extensions = +extensions
      ]
      either empty? test-extensions: intersect +extensions -extensions [
        for-each [param desc default deps] params [
          if extension? param [
            set param either find +extensions param [true] [false]
          ]
        ]
      ] [
        usage/message [{Aborting, the following extensions have been both required and prohibited: [} form test-extensions {]}]
      ]
    ]
  ]
] [
  usage
]

; Locates source
source-path: clean-path rejoin [first split-path system/options/script source-path]

; Runs setup if premake isn't found
if not exists? source-path/:premake-source [
  log [{New orx installation found, running setup!}]
  attempt [delete-dir source-path/:extern/]
  in-dir source-path/../../.. [
    call/shell platform-info/setup
  ]
]

; Retrieves project name
if dir? name: clean-path local-to-file name [clear back tail name]

; Inits project directory
either exists? name [
  log [{[} file-to-local name {] already exists, overwriting!}]
] [
  until [
    attempt [make-dir/deep name]
    exists? name
  ]
]
change-dir name/..
set [path name] split-path name

; Logs info
log [
  {Initializing [} name {] in [} file-to-local path {] with extensions [}
  do [
    use [extensions] [
      remove-each template extensions: copy templates [any [not extension? template not get template]]
      if not empty? extensions [form extensions]
    ]
  ]
  {]}
]

; Copies all files
log {== Creating files:}
build: _
reeval copy-files: function [
  from [file!]
  to [file!]
] [
  for-each file read from [
    src: from/:file
    if all [
      not empty? dst: to-file apply-template to-text file
      dst != %/
    ] [
      dst: to/:dst
      if file = %build/ [
        set 'build dst
      ]
      either dir? src [
        make-dir/deep dst
        copy-files src dst
      ] [
        log/only [{  +} file-to-local dst]
        write dst apply-template read src
      ]
    ]
  ]
] source-path name

; Creates build projects
if build [
  in-dir build [
    write platform-info/premake read source-path/:premake-source
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
