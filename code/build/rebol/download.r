REBOL [
    title: "Download"
    author: "iarwain@orx-project.org"
    date: 26-Sep-2015
    file: %download.r
]

; Download file
write to-file system/options/args/2 read to-url system/options/args/1
