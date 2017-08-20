REBOL [
    title: {Init}
    author: {iarwain@orx-project.org}
    date: 15-Aug-2017
    file: %init.r
]

; Variables
source: %../template/
premake: %premake4
params: reduce [
    'name           {Project name}      none
    'destination    {Destination path}  %./
]
platforms:  [
    {windows}   [config [{gmake} {codelite} {vs2013} {vs2015} {vs2017}]]
    {mac}       [config [{gmake} {codelite} {xcode4}                  ]]
    {linux}     [config [{gmake} {codelite}                           ]]
]

; Helpers
delete-dir: funct [
    {Deletes a directory including all files and subdirectories.}
    dir [file! url!]
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
log: funct [
    message [string! block!]
    /only
    /no-break
] [
    unless only [
        prin [{[} now/time {] }]
    ]
    either no-break [prin message] [print message]
]

; Inits
switch platform: lowercase to-string system/platform/1 [
    {macintosh} [platform: {mac}]
]
platform-info: platforms/:platform

; Usage
usage: func [
    /message content [block! string!]
] [
    if message [
        prin {== }
        print content
        print {}
    ]

    prin [{== Usage:} system/options/boot system/options/script]

    print rejoin [
        newline newline
        map-each [param desc default] params [
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
        use [arg] [
            arg: system/options/args
            foreach [param desc default] params [
                either tail? arg [
                    either default [
                        set param default
                    ] [
                        usage/message [{Not enough arguments:} mold system/options/args]
                    ]
                ] [
                    set param arg/1
                    arg: next arg
                ]
            ]
        ]
    ]
] [
    usage
]

; Locates source and destination
change-dir root: system/options/path
source: clean-path/dir join first split-path join root system/options/script source
unless exists? destination: to-rebol-file destination [
    make-dir/deep destination
]
change-dir destination
destination: what-dir

; Creates project directory
if exists? name: to-rebol-file name [
    log [{[} name {] already exists, erasing!}]
    delete-dir name
]
log [{Initializing [} name {]}]
make-dir/deep name

; Copies all files
log {== Creating files:}
template: {[orx]}
build: none
do copy-files: funct [
    from [file!]
    to [file!]
] [
    foreach file read from [
        src: from/:file
        dst: replace to/:file template name
        if file = %build/ [
            set 'build dst
        ]
        either dir? src [
            make-dir/deep dst
            copy-files src dst
        ] [
            log/only [{  +} dst]
            write dst replace/all read src template name
        ]
    ]

] source destination/:name

; Creates build projects
if build [
    log [{Generating build files for [} platform {]:}]
    change-dir build
    foreach config platform-info/config [
        log/only [{  *} config]
        call/wait reform [root/:premake config]
    ]
]

; Ends
log {Init successful!}
