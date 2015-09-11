self category: 'Program Function'.
self function [
main
    OSIO write: 'Hello World
' offset: 0 size: 12 to: OSIO stdout
    "FileStream stdout nextPutAll: 'Hello World'; lf"
    .
]

