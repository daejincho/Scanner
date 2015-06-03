Lexical analyser (scanner)
=======

Lexical analyser of interpreter of imperative language LUA.     
http://en.wikipedia.org/wiki/Lexical_analysis

input: source code
output: tokens
-- remove comments and white characters,
-- implemented as finite state machine,
-- finite state machine must finish in finite state, other error,
-- scanner can write line with error,
-- if ASCII character < 32, scanner write to stderr warning, and ignore this character
(exception backspace, Line Feed (LF), Form Feed (FF), Carriage Return (CR), etc.),
