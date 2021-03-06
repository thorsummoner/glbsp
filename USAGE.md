
USING glBSP
===========

by Andrew Apted.  JULY 2007


Running glBSP
-------------

There are two versions of glBSP: normal 'glbsp' (the text version),
and 'glBSPX' (offering a graphical interface).  This documentation
applies to the text version only.  Under Windows, the text version
must be run from the command shell (cmd.exe).

There are two main ways of using glBSP:

  1. Create the GL nodes and store them in a separate file with the
     GWA extension.  This is called "GWA mode".

  2. Create the GL nodes and store them in the WAD file itself, just
     like how a normal node builder works.

These ways are described in more detail below.


1. GWA Mode
-----------

For general use, when you just want to use some wad (e.g. FOO.WAD)
with a compliant OpenGL DOOM port, then this method is the recommended
way.  It is also the simplest method, just type:

   glbsp foo.wad

and then glBSP will create the file "foo.gwa".  A compliant OpenGL
DOOM port (such as EDGE) will notice the GWA file automatically and
load it appropriately.  Nothing more needs to be done.

You can also specify multiple wads here, even wildcards should
work.  For example:

   glbsp foo.wad fred.wad simon*.wad

Note!  You should not give these GWA files to the DOOM port using
the command line (i.e. with the -file option), doing so will confuse
it and it'll probably crash.  The port should notice the GWA file
automatically.

Note 2: when using GWA mode, the output file does not contain any
information for things, linedefs, sidedefs, sectors, reject, or
blockmap, all it contains is the GL node information.  This is
where the name "GWA" comes from, it is short for "GL WAD" (in
case you were wondering).


2. Non-GWA Mode
---------------

This method is intended for WAD authors, so they can create a single
WAD file for distribution that contains all the needed information.
Using this method looks like this:

   glbsp foo1.wad -o foo2.wad

and then glBSP will build the GL nodes and output the new file
"foo2.wad" containing them.  It will also build the normal node
information if it is absent.


Response Files
--------------

New in version 2.20 is support for response files.  These are
files containing a list of options.  You specify the response
file by prefixing it with '@'.  For example:

   glbsp @arg_list.rsp

The "@arg_list.rsp" on the command line will be replaced with
the contents of that file.  New-line characters are treated like
spaces.  Recursion (using '@' inside a response file) is not
supported.


Option Descriptions
-------------------

The following is a list of all the available options.
Each option has a short form and a long form.

```
  -q -quiet     Quieter output.  Information about each level (like
                the number of linedefs, blockmap size, etc) is not
                displayed when this option is given, and a few other
                messages are skipped.  Important messages, like
                failure to build a certain level, are still shown.

  -f -fast      Allows glBSP to cheat a bit and re-use the original node
                information to create the GL nodes, doing it much faster.
                Use this option to enable this feature.  The message
                "Using original nodes to speed things up" will be shown.

                The downside to reusing the original nodes is that they
                may not be as good as the ones glBSP normally creates,
                e.g. the special checks to minimise slime-trails don't
                kick in, and the -factor value doesn't have any effect.

  -w -warn      Shows extra warning messages, which detail various
                non-serious problems that glBSP has while analysing the
                level structure.  Often these warnings show a real
                problem in the level (e.g. a non-closed sector or
                invalid sidedef), so they are worth checking now and
                then.

  -n -normal    glBSP usually detects if the normal node info (i.e.
                the non-GL variety) is present: when yes, it is left
                untouched, otherwise glBSP creates it.  This option
                forces glBSP to replace the normal node data with
                newly constructed nodes.

  -c -factor <num>
                Sets the cost assigned to seg splits.  Factor can be
                any number from 1 to 32, and larger values make seg
                splits more costly (and thus glBSP tries harder to
                avoid them), but smaller values produce better BSP
                trees.  See the section below for more info.  The
                default value is known to be a good compromise.

  -p -pack      Pack sidedefs, by detecting which sidedefs are
                identical and removing the duplicates, producing a
                smaller PWAD.

                NOTE: this will make your level a lot harder to edit!
                Therefore this is most useful when producing your final
                WAD for public release.

  -xr -noreject
                Normally glBSP will create an simple REJECT map for
                each level.  This options prevents any existing
                REJECT map, such as one time-consumingly built by
                a dedicated reject builder, from being clobbered.
```

These options are rarely needed:

```
  -v1 .. -v5    Specify the version of the "GL Nodes" spec to use.
                The default is -v2.  Using -v1 produces an obsolete
                format.  Giving -v3 or -v5 will force certain lumps
                to use the new formats, but is only useful for testing
                since glBSP will automatically switch to V5 format
                when certain limits are exceeded.

  -m -mergevert
                Merge duplicate vertices at the same location into a
                single vertex.  This is usually safe, but is not done
                by default because some engines (e.g. Risen3D) need the
                duplicate vertices to stay separate for special effect.

  -y -windowfx  Lets glBSP detect and handle the "One-Sided Window"
                mapping trick.  This can cause problems in some engines
                so it is disabled by default.

  -u -prunesec  Removes any unused sectors found in the level.  Only
                works when normal nodes are being built.  This is
                not done by default since some scripting languages
                (Fragglescript, old versions of RTS) refer directly
                to the sector numbers and removing even one will
                change all the numbering.

  -b -maxblock <num>
                Sets the limit of the number of blocks the BLOCKMAP may
                contain before we truncate it.  Default is 16000.  When
                the level is too large to fit, glBSP will truncate the
                blockmap, so it covers less area on the level.  This
                means that in the parts it doesn't cover (at the outer
                edges) there is no collision detection: you can walk
                through walls and other objects and bullets/missiles
                don't hit anything.  On very large but sparse levels,
                using a larger value (e.g. 30000) may help.

                A more serious problem is when the blockmap overflows.
                The blockmap created would be invalid, and could crash
                the DOOM engine when used.  glBSP will create an empty
                blockmap instead, causing modern ports to build their
                own blockmap.

  -xp -noprog   Turn off the progress indicator.

  -xn -nonormal
                Forces glBSP to not create the normal node information
                when it detects that it is absent.

  -xu -noprune  Prevents glBSP from removing zero-length linedefs and
                unused sidedefs.
```

ZDBSP Format Nodes
------------------

When the normal nodes overflow, older versions of glBSP would simply
write out the invalid node data.  glBSP 2.20 and higher will write
out the node data in the ZDBSP format (originally created for the
ZDoom engine).


Interaction with other tools
----------------------------

As far as I know, none of the various WAD tools that exist (such
as DSHRINK, CLEANWAD, DEUTEX, etc..) are "glBSP aware", they will
rearrange or even remove some of the special GL entries, and
everything goes pear shaped.

When using a reject building tool (such as RMB), you need to give
the -noreject to glBSP to prevent the good REJECT data from being
overwritten.

*** DO NOT: ***

  + Run DSHRINK on your map WADs at any time!

  + Run CLEANWAD on your map WADs *after* you have compiled your GL
    friendly nodes!  (The GL nodes will be removed).

  + Use Wintex/DEUSF to merge map WADs with GL friendly nodes in them!
    (The GL node entries will be rearranged, causing problems).


How glBSP works
---------------

A node builder works like this: say you are looking at your level in
the automap or in the level editor.  The node builder needs to pick a
line (stretching to infinity) that divides the whole map in two halves
(can be rough).  Now cover up one half with your hand, and repeat the
process on the remaining half.  The node builder keeps doing this
until the areas that remain are convex (i.e. none of the walls can
block the view of any other walls when you are inside that area).

Those infinite lines are called "partition lines", and when they cross
a linedef, the linedef has to be split.  Each split piece is called a
"seg", and every split causes more segs to be created.  Having fewer
segs is good: less to draw & smaller files, so partition lines are
chosen so that they minimise splits.  The "-factor" value controls how
costly these splits are.  Higher values cause the node builder to try
harder to avoid splits.

So, each "partition" line splits an area (or "space") of the level
into *two* smaller spaces.  This is where the term "Binary Space
Partition" (BSP) comes from.

Another thing: having a good BSP tree is also important, and helps for
faster rendering & smaller files.  Thus the node builder also tries to
make the BSP tree good (by making it balanced, and keeping it small).
If the "-factor" value is too high, it will care too much about the
splits, and probably make a bad BSP tree.  How good the BSP tree is
can be gauged by the output line that reads:

   Heights of left and right subtrees = (12,24)

Lower values are better (the BSP tree is smaller), and values that
are closer together are also better (the BSP is more balanced).


Differences to BSP 2.3
----------------------

As mentioned in the readme file, glBSP was originally based on BSP 2.3.
Most of the code has been rewritten, however, and some features of BSP
were changed or abandoned.  Features that are different:

  +  When the output file is not specified (i.e. no -o option), then
     the default output file will be a GWA file with the same name.
     Under BSP 2.3, the default output file would be "tmp.wad".

  +  All code for doing visplane checks has been removed.  It was very
     complex stuff, and for OpenGL DOOM ports this checking is not
     needed.  Thus glBSP does not accept the following options that
     BSP 2.3 supports: -thold, -vp, -vpmark, -vpwarn.

  +  glBSP works on big-endian platforms (like MacOS X).

  +  The "just for a grin" special feature where a linedef with tag
     999 would cause an angle adjustment was removed.

  +  glBSP has Hexen support.

  +  glBSP compresses the blockmap, and can pack sidedefs.

  +  glBSP has a much more modular architecture, and can even serve as
     a plug-in for other programs.

  +  The GUI version, glBSPX, is completely new.



---------------------------------------------------------------------------

