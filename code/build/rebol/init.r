REBOL [
    title: "Init"
    author: "iarwain@orx-project.org"
    date: 15-Aug-2017
    file: %init.r
]

; Variables
source: %../template/
destination: %./
params: reduce [
    'name           {Project name}      no
    'destination    {Destination path}  yes
]

; Usage
usage: does [
    prin [{Usage:} system/options/script]

    print rejoin [
        newline newline
        map-each [param desc optional] params [
            prin rejoin [{ } either optional [rejoin [{[} param {]}]] [param]]
            rejoin [{= } param {: } desc either optional [{, optional}] [{}] newline]
        ]
    ]
    quit
]

; Process params
either system/options/args [
    use [param] [
        param: params
        foreach arg system/options/args [
            either tail? param [
                print [{Too many arguments:} mold system/options/args newline]
                usage
            ] [
                set param/1 arg
                param: skip param 3
            ]
        ]
        unless any [
            tail? param
            param/3
        ] [
            print [{Not enough arguments:} mold system/options/args newline]
            usage
        ]
    ]
] [
    usage
]

