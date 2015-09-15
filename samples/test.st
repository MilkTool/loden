"Kernel methods"
self class: Class.
self category: 'accessing'.

self method [
name
    ^ name
].

"Script context methods"
self class: ScriptContext.
self category: 'accessing'.

self method [
currentClass
    ^ currentClass
].

self method [
category
    ^ currentCategory
].

self method [
globalContextClass
    ^ globalContextClass
].

"The main program"
self category: 'Program Function'.

self function [
printString: string
    OSIO write: string offset: 0 size: string size to: OSIO stdout
].

self function [
printLine: line
    self printString: line;
         printString: '
'
].

self function [
main
    self printLine: 'Hello World';
        printLine: Object name.
]

