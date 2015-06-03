Lexical analyzer
=======

Lexical analyzer of interpreter of imperative language LUA.

TODO
Lexikální analyzátor provádí odstranění komentářů a bílých znaků. Vstupem lexikálního
analyzátoru je zdrojový program a výstupem jsou tokeny.
Lexikální analyzátor pro projekt IFJ je implementován pomocí konečného automatu. Automat
přechází mezi stavy a pokud neskončí v koncovém stavu, ohlásíme chybu lexikálního
analyzátoru.
Popis implementace:
Lexikální analyzátor si umí správně spočítat řádky a vypsat na kterém řádku k chybě došlo.
Pokud se ve zdrojovém textu vyskytnou ASCII znaky s hodnotou menší než 32, na stderr se
vypíše varování o nalezeném znaku a znak se ignoruje. Samozřejmě vyjma znaků: Backspace,
Line Feed (LF), Form Feed (FF), Carriage Return (CR), atd.
Konečný automat pro lexikální analyzátor obsahuje celkem 41 stavů. Z prostorových důvodů
je rozdělen na logické celky a je zbaven detailů.


