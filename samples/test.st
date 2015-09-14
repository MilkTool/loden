self category: 'Program Function'.

self function [
printString: string
    OSIO write: string offset: 0 size: string size to: OSIO stdout
].

self function [
printLine: line
    self printString: line.
    self printString: '
'
].

self function [
main
    self printLine: 'Hello World'
]

