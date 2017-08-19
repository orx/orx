REBOL [
    title: "Init"
    author: "iarwain@orx-project.org"
    date: 15-Aug-2017
    file: %init.r
]

; Variables
source: %../template/
params: reduce [
    'name           {Project name}      none
    'destination    {Destination path}  %./
]

; Usage
usage: func [
    /message content [block! string!]
] [
    if message [
        print content
        print {}
    ]

    prin [{Usage:} system/options/boot system/options/script]

    print rejoin [
        newline newline
        map-each [param desc default] params [
            prin rejoin [{ } either default [rejoin [{[} param {]}]] [param]]
            rejoin [{= } param {: } desc either default [rejoin [{=[} default {], optional}]] [{, required}] newline]
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
