# akiro

## Summary

Akiro[^1] is a simple Windows-based backup tool that periodically makes background copies of folders.

## How to...

### ...setup?

### ...kick the tires?

### ...restore something?

## How Akiro works
1. Akiro is composed to two executables: a command-line interface and a background process, which communicate over shared memory with named events.
1. Akiro is *not* a Windows service.  This means the background process has to be run either manually, or added to the Start Up group in Windows.
1. Akiro monitors folders you configure.  Periodically (which is configurable), Akiro recursively descends the given folder and copies all files to (configurable) backup locations.
1. Seperately, Akiro "compacts" the backup locations.  "Compaction" means Akiro removes duplicate files and maintains a journal file.  This means Akiro may actually make a copy during the backup, then delete it during compaction.  These steps are seperated so the copy can be as quick as possible.
1. The 'backup location', is just a plain folder, with a navigatable structure.
1. The 'journal' file is human-readable CSV (comma seperated values) viewable in a spreadsheet application.
1. Akiro provides tooling for recreating the source folder at a given moment in time
1. Akiro provides tooling for eliminating old versions of files

[^1]: Akiro was the chronicler of Conan in *Conan the Barbarian*.
