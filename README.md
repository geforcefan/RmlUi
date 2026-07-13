# RmlUi layout performance repro

One text change in a document with nested flexbox makes RmlUi reformat the whole
document, and that single reformat gets pathologically slow with deep flex nesting.
This is a static document and one `ElementText::SetText` per slider tick.

## Build and run

```
cmake -S . -B build
cmake --build build -j
```

You get two binaries built from the same source, one against upstream RmlUi master and
one against the fix branch
[layout-measure-cache](https://github.com/geforcefan/RmlUi/tree/layout-measure-cache):

```
./build/repro_upstream
./build/repro_fixed
```

Both run 100 text mutations at startup and print the timing, so the difference is in
the console before you touch anything. After that, drag the slider. Its change event
sets the readout text next to it, which is the only mutation in the whole document.
The `Context::Update` time is printed every 120 frames while you drag.

## Numbers on my machine (Apple M2 Pro, Release)

The document has a slider panel and 66 static input rows in three columns, 877
elements, 8 flex containers deep (app, panel, row, group, field, control, input, body).

| binary | 100 text mutations at startup | `FormatIndependent` calls per layout pass |
| --- | --- | --- |
| repro_upstream | 201.7 ms per mutation | 868446 |
| repro_fixed | 3.1 ms per mutation | 10431 |

Counted with temporary counters in `FormattingContext::FormatIndependent`, on master:

* one single text element is formatted 2880 times in a single layout pass
* only 13929 of the 868446 calls have distinct (element, box, containing block) inputs
* the counts are identical on every pass
* flex sizing measures every auto-sized item by formatting its whole subtree: shrink-to-fit width, max-content size, hypothetical cross size
* nothing is reused, nested flex multiplies the measure passes per level

In our application: dragging a value input or typing drops the frame rate from 120 to
single digits, a single committed keypress produces a visible hitch.
