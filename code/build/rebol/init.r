REBOL [
  title:      {Init}
  author:     {iarwain@orx-project.org}
  date:       15-Aug-2017
  file:       %init.r
]

; Variables
params: compose/deep [
  name        {Project name (relative or absolute path)}                                        (none)    (none)
  bundle      {Automatic resource encryption & packaging}                                       +         []
  cheat       {Secret pass/cheat code support}                                                  -         []
  c++         {Create a C++ project instead of a C one}                                         +         []
  imgui       {Dear ImGui (GUI) support (https://github.com/ocornut/imgui)}                     -         [+c++]
  inspector   {Object debug GUI inspector}                                                      -         [+imgui]
  mod         {Audio MOD (Amiga), XM & S3M support}                                             -         []
  movie       {Movie (MPEG-1) support}                                                          -         []
  noisetex    {Noise texture generation support}                                                -         []
  nuklear     {Nuklear (GUI) support (https://github.com/immediate-mode-ui/nuklear)}            -         []
  python      {Python support (https://pocketpy.dev)}                                           -         []
  remote      {Web-served resources support, HTTP/1.1 only, proof of concept)}                  -         []
  scroll      {C++ convenience layer with config-object binding}                                +         [+c++]
  sndh        {Audio SNDH (Atari ST) support}                                                   -         [+c++]
]
platforms:  [
  windows     [config [{gmake} {codelite} {codeblocks} {vs2017} {vs2019} {vs2022}]    premake %premake4.exe   setup {setup.bat}   script %init.bat    ]
  mac         [config [{gmake} {codelite} {codeblocks} {xcode4}                  ]    premake %premake4       setup {./setup.sh}  script %./init.sh   ]
  linux       [config [{gmake} {codelite} {codeblocks}                           ]    premake %premake4       setup {./setup.sh}  script %./init.sh   ]
]
source-path: %../template/
extern: %../../../extern/

; Helpers
log: function [
  message [string! char! block!]
  /only
  /no-break
] [
  unless only [
    prin [{== }]
  ]
  do either no-break [:prin] [:print] reform message
]
extension?: function [
  {Is an extension?}
  name [word! string!]
] [
  not none? attempt [find [+ -] third find params to-word name]
]
apply-template: function [
  {Replaces all templates with their content}
  content [string! binary!]
] [
  foreach [var condition] [
    template    [not extension? entry]
    +extension  [all [extension? entry get entry]]
    -extension  [all [extension? entry not get entry]]
  ] [
    set var append copy [{-=dummy=-}] collect [foreach entry templates [if with context? 'entry condition [keep reduce ['| to-string entry]]]]
  ]
  clean-chars: charset [#"0" - #"9" #"a" - #"z" #"A" - #"Z" #"_"]
  template-rule: [(sanitize: no) begin-template: {[} opt [{!} (sanitize: yes)] [{=} (value: copy {}) opt [copy value template] {]} (override: false) | copy value template {]}] end-template: (
      if find templates value: load to-string trim value [value: copy get value]
      if sanitize [parse value [some [clean-chars | char: skip (change char #"_")]]]
      end-template: change/part begin-template value end-template
    ) :end-template
  ]
  in-bracket: charset [not #"]"]
  bracket-rule: [{[} any [bracket-rule | in-bracket] {]}]
  extension-rule: [
    begin-extension:
    remove [
      {[} (erase: yes)
      opt [{=} (override: false)]
      some [
        [ [ {+} -extension | {-} +extension]
        | [ {+} +extension | {-} -extension] (erase: no dynamic: true)
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
    opt [if (erase) opt [if (full-line) remove opt [{^M^/} | {^/}]] (append dynamic: copy/part content begin-extension to-string take/part begin-extension end-extension)]
    :begin-extension
  ]
  parse content [
    (full-line: yes dynamic: none override: true)
    any
    [ extension-rule
    | template-rule
    | {^/} (full-line: yes)
    | skip (full-line: no)
    ]
  ]
  reduce [content dynamic override]
]

; Inits
change-dir root: system/options/path
code-path: {..}
date: to-string now/date
switch platform: system/platform [
  macos [platform: 'Mac code-path: to-local-file root/code]
]
platform-info: platforms/:platform
premake-source: rejoin [%../ platform-info/premake]
templates: append collect [
  foreach [param desc default deps] params [keep param]
] [date code-path]

; Usage
usage: function [
  /message content [string!]
] [
  if content [
    log rejoin [content newline]
  ]

  log/no-break reform [{Usage:} to-local-file clean-path rejoin [system/options/script/../../../.. {/} platform-info/script]]
  foreach [param desc default deps] params [
    unless extension? param [log/only/no-break rejoin [{ } param]]
  ]
  log/only rejoin [{ [{+/-extension} ...]} newline]

  param-max-length: 6 + first find-max map-each param extract params 4 [length? param]
  desc-max-length: first find-max map-each param extract next params 4 [length? param]
  foreach [param desc default deps] params [
    log/only rejoin [
      {  } param: rejoin [{[} pick reduce [default {!}] extension? param {] } param] append/dup copy {} { } param-max-length - length? param desc
      pick reduce [{} rejoin [append/dup copy {} { } desc-max-length - length? desc { -> [} deps {]}]] empty? deps
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
    +extensions: copy [] -extensions: copy [] y: yes n: no
    either interactive?: zero? length? args: copy system/options/args [
      log {No argument, switching to interactive mode}
      foreach [param desc default deps] params [
        either extension? param [
          until [
            any [
              empty? value: ask rejoin [{ * [Extension] } param {: } desc {? (} pick [{yes} {no}] default = '+ {)}]
              logic? value: attempt [get load trim value]
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
      foreach [param desc default deps] params [
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
            usage/message reform [{Not enough arguments:} mold system/options/args]
          ]
        ]
      ]
      unless tail? args [
        usage/message reform [{Too many arguments:} mold system/options/args]
      ]
    ]
    ; Handles extensions dependencies
    use [test-extensions extension-group extension] [
      until [
        test-extensions: copy +extensions
        foreach param test-extensions [
          foreach dep fourth find params param [
            extension-group: either #"+" = first to-string dep [+extensions] [-extensions]
            extension: to-word next to-string dep
            unless find extension-group extension [
              append extension-group extension
              log [{[} param {] triggers [} dep {]}]
            ]
          ]
        ]
        test-extensions = +extensions
      ]
      either empty? test-extensions: intersect +extensions -extensions [
        foreach [param desc default deps] params [
          if extension? param [
            set param not none? find +extensions param
          ]
        ]
      ] [
        usage/message reform [{Aborting, the following extensions have been both required and prohibited: [} form test-extensions {]}]
      ]
    ]
  ]
] [
  usage
]

; Locates source
source-path: clean-path rejoin [first split-path system/options/script source-path]

; Runs setup if premake isn't found
unless exists? source-path/:premake-source [
  log {New orx installation found, running setup!}
  delete-dir source-path/:extern
  in-dir source-path/../../.. [
    call/wait/shell platform-info/setup
  ]
]

; Retrieves project name
if dir? name: clean-path to-rebol-file name [clear back tail name]

; Inits project directory
action: either exists? name [
  log [{[} to-local-file name {] already exists, updating!}]
  {Updating}
] [
  until [
    attempt [make-dir/deep name]
    exists? name
  ]
  {Creating}
]
change-dir name/..
set [path name] split-path name

; Logs info
log [
  {Initializing [} name {] in [} to-local-file path {] with extensions [}
  do [
    use [extensions] [
      remove-each template extensions: copy templates [any [not extension? template not get template]]
      unless empty? extensions [form extensions]
    ]
  ]
  {]}
]

; Copies all files
log reform [action {files:}]
build: none
do copy-files: function [
  from [file!]
  to [file!]
] [
  foreach file read from [
    src: from/:file
    set [dst stripped allow-override] apply-template to-string file
    case/all [
      all [
        not empty? dst: trim to-file dst
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
          set [content dynamic] apply-template read src
          if any [
            not exists? dst
            allow-override
          ] [
            log/only reform [either exists? dst [{  !}] [{  +}] to-local-file dst]
            write dst content
          ]
        ]
      ]
      all [
        string? stripped
        exists? stripped: to/(trim to-file stripped)
      ] [
        log/only reform [{  -} to-local-file stripped]
        delete-dir stripped
      ]
    ]
  ]
] source-path name false

; Creates build projects
if build [
  in-dir build [
    write platform-info/premake read source-path/:premake-source
    unless platform = 'windows [
      call/wait/shell form reduce [{chmod +x} platform-info/premake]
    ]
    log [{Generating build files for [} platform {]:}]
    foreach config platform-info/config [
      log/only reform [{  *} config]
      call/wait/shell/output rejoin [{"} to-local-file clean-path platform-info/premake {" } config] none
    ]
  ]
]

; Ends
log {Init successful!}
