Cinmix 0.22 | BIOS i386 | 1920x1080 preferred

Sketch has been replaced by a Cinmix-native Pinta-style painter: pencil, brush, eraser, line, rectangle, fill, palette, undo/redo, and disk save with Ctrl+S. This is not the unmodified upstream Pinta application, which requires .NET and GTK. An isolated ring-3 worker handles bounded brush-stamp requests through a protected mailbox. Existing CinmixFS disk images remain compatible. Keep your old disk if it contains saved data; the new image is blank. Details and attribution: README.md, third_party/PINTA-SOURCE.txt, third_party/PINTA-LICENSE.txt.
