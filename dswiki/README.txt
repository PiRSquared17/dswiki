 DSwiki (v0.1)
====================

DSwiki is an offline viewer for Wiki markup, for example for the
Wikipedia, Wikibooks, Wiktionaries, ...

DSwiki is NOT a web browser!

 Why DSwiki?
=============

DSwiki is optimal for all people with a Nintendo DS that like to
have their information "portable", but cannot access the Internet
directly. So far, no Program was very good in displaying the
Wikipedia on the DS. NewDictS can display an uncompressed dump, but
this dump is outdated, and the Wikipedia has grown dramatically
since then, so compression was absolutely necessary. In addition,
most homebrew programs can't handle UTF-8.

 DSwiki is based on
====================

-- NewDictS       <http://tvgame360.com.tw/viewthread.php?tid=21913&extra=&page=1>
                    Thanks to sesa for graphics-/unicode- and font-routines

-- Wiki2Touch     <http://code.google.com/p/wiki2touch/>
                    Thanks to Tom Haukap for the file format and index routines

-- libbzip2       <http://bzip.org/>

-- UCS fonts      <http://www.cl.cam.ac.uk/~mgk25/ucs-fonts.html>
                    Thanks to Markus Kuhn

-- some other fonts

 Features
==========

-- Get the complete(!) Wikipedia on a Nintendo DS. The English language
   edition fits on a 4GB storage device, the German edition is just 1.1GB.
-- Quite good handling of internal links. Alternative display text
   and appended letters are supported to create the same look as
   in the online version.
-- Very good Unicode/UTF8-Support (as much - or as little - as the
   underlying font covers).
-- Fast and easy searchable index.
-- A cache to navigate quickly the history of visited pages.

 Planned Features
==================

-- Much better markup display
-- Bookmarks
-- Improved Search (advanced input)
-- Navigation by document structure (jump to headlines or
   jump from link to link as in text browsers)
-- "Find as you type" inside an article
-- Memory saving font handling, switching between fonts on the fly
-- Customization

 Use
=====

At the moment DSwiki can't do more than display mostly unformatted
wiki markup of articles (but word and line breaks and a proportional
font make it easy readable).

DSwiki works as follows:

A random article will be loaded at the start.

      Up/Down: Scroll one line backward/forward

   Left/Right: Scroll one page backward/forward

       Stylus: Follow a link on the bottom screen

          L/R: Go one page back/forward in history

            A: Go to another random article

            X: Index-assisted search on/off

               Articles names (no unicode yet) can be entered
               on the keyboard.

               Touching the red button clears the input field.

               The D-Pad navigates through the index.
               The L/R-buttons do bigger steps.

               The current highlighted article name can be loaded
               either by
                * touching the green button
                * pressing A
                * touching Enter on the virtual keyboard

       Select: Choose another available wiki


 Installation
==============

1. Copy DSwiki.nds somewhere on your flash card, DLDI-patch it if
   necessary.

2. Create dumps of your favorite wiki or download already prepared ones.
   Visit <http://code.google.com/p/dswiki/wiki/Dumps> for a list.

3. Copy all files with the extensions

     ifo
     idx
     ao1
     ao2
     db?    [ ? = one letter, typically "a" or "b", "z" is used in older versions ]

   into /dswiki. This directory must exist in the root directory of your flash card.

 Bugs/Issues
=============

-- In some cases, image captions are not parsed correctly (no caption given, links
   in the caption).



Have Fun!
OlliPolli