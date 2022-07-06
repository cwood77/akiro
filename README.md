# akiro

## Summary

Akiro[^1] is a simple Windows-based backup tool that periodically makes background copies of folders.  The goal here is to be simple, and open, storing stuff in human-readable formats and locations.  Akiro is intended to be a safety net for casual use, not some mission-critical, bullet-proof vault.

## How to...

### ...setup?
There is no installer.  Partially because (1) I'm lazy, (2) I distrust installers, (3) it's easy to setup yourself.
1. Copy the akiro executables to your machine.
1. Put that location in your `PATH` variable.
1. Configure the command "akiro start" to run at start-up.
1. Examine/edit the configuration file `akiro.txt` to view/edit configuration values.  Whenever you make changes here, you must issue `akiro stop` and `akiro start` for them to take effect.

### ...kick the tires?
`akiro status`

### ...turn it off, or pause it?
`akiro stop` and `akiro start`

### ...restore something?
First, you prolly want to see what versions are available:

`akiro timestamps "C:\Users\cwood\Documents\ReallyImportantProject"`

When you find a timestamp you want to examine, do

`akiro restore "C:\Users\cwood\Documents\ReallyImportantProject" "F:\place-to-write-to" 20220706-104421`

where `20220706-104421` is the timestamp you picked.

**note** Akiro *never* writes to the original source location, it only ever reads.  Even in a `restore` operation, Akiro restores to a different location than it read from.  This gives you the user the ability to double-check Akiro's work.

### ...configure settings?
All settings are in `akiro.txt`, which uses comments to explain what settings are available.  Remember to issue `akiro stop` and `akiro start` if you change this file.

An example akiro.txt file:
```
# lines that begin with # are comments, and are placed here to document the format.
#
#   # you can make multiple monitor statements
#   # monitoring is _always_ recursive to all subfolders, etc.
#   # so don't put a monitor statement underneath another!
# monitor-absolute-path: C:\users\cwood\Documents\ReallyImportantProject
#   frequency-in-minutes: 5
#
#   # There is only one backup statement
# backup:
#    absolute-path: F:\data\akiro
#    # you can set this to zero to keep everything
#    oldest-version-to-keep-in-days: 365
#
```

## How Akiro works
1. Akiro is composed to three executables: a command-line interface, a background backup process, and a background compaction process, which communicate over shared memory with named events.
   - Akiro is *not* a Windows service.  This means the background process has to be run either manually, or added to the Start Up group in Windows.
1. Akiro monitors folders you configure.  Periodically (which is configurable), Akiro recursively descends the given folder and copies all files to (configurable) staging location.
1. Seperately, Akiro files the staging data in the  backup location by deduping.  This process also maintains a journal file.  This means Akiro may actually make a copy during the backup process, then delete the copy (if it's a duplicate) later.  These steps are seperated so the copy can be as quick as possible.
   - The 'backup location', is just a plain folder, with a navigatable structure.
   - The 'journal' file is human-readable CSV (comma seperated values) viewable in a spreadsheet application.
1. Akiro provides tooling for recreating the source folder at a given moment in time
1. Akiro provides tooling for eliminating old versions of files

[^1]: Akiro was the chronicler of Conan in *Conan the Barbarian*.
