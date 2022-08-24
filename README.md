# akiro

## Summary

Akiro[^1] is a simple Windows-based backup tool that makes background copies of folders.  The goal here is to be simple, open, and invisible.  Openness includes not only open source but also storing stuff in human-readable formats and locations.  Akiro is intended to be a safety net for casual use, not some mission-critical, bullet-proof vault.

## How does Akiro work?
In short:
1. Akiro is composed of multiple executables: a command-line interface, a background compaction process, and background monitor process(es), which communicate over shared memory with named events.
   - Akiro is *not* a Windows service.  This means the background process has to be run either manually, or added to the Start Up group in Windows.
1. One Akiro monitor process is created for each folder tree you configure to backup.  Whenever a change is made anywhere under this tree (i.e. including subfolders), Akiro recursively descends the given folder, copies all files to a staging location, then triggers the compaction process.
   - This monitoring is throttled by a configuration value so snapshots are taken at most every 5 minutes, for example, even if changes occur more frequently.
1. When triggered by monitoring, the Akiro compactor adds the staging data to the backup
   - This process include de-duplicating files to save space
1. Akiro provides command-line tools for recreating a monitored folder at a given moment in time
1. Akiro implements retention policies to prevent the backup from growing forever

Further details on a few of these points are discussed below.

## How to...

### ...setup?
There is no installer.  Partially because (1) I'm lazy, (2) I distrust installers, (3) it's easy to setup yourself.
1. Copy the akiro executables and txt file to your machine.
1. Put that location in your `PATH` variable.
1. Configure the command "akiro start" to run at start-up.
1. Examine/edit the configuration file `akiro.txt` to view/edit configuration values.  Whenever you make changes here, you must issue `akiro stop` and `akiro start` for them to take effect.

### ...kick the tires?
You can ask Akiro what it's doing with `akiro status`.  Here's some example output:
```
C:\>akiro status
akiro - simple background backups
        https://github.com/cwood77/akiro

[ready        ] responsive     -    -- never --    - monitor [on]  (C:\Users\cwood\Desktop\game12 - Alexander)
[ready        ] responsive     - 08/23/22 16:16:11 - monitor [on]  (C:\Users\cwood\Desktop\game11)
[ready        ] responsive     -    -- never --    - monitor [on]  (C:\Users\cwood\Desktop\game13 - zocolypze)
[ready        ] responsive     -    -- never --    - monitor [on]  (C:\Users\cwood\Desktop\game14 - miskU)
[ready        ] responsive     -    -- never --    - monitor [on]  (C:\Users\cwood\Desktop\game15 - dogwater)
[ready        ] responsive     -    -- never --    - monitor [on]  (C:\Users\cwood\Desktop\game18 - populus)
[ready        ] responsive     - 08/24/22 11:32:03 - monitor [on]  (C:\Users\cwood\Desktop\game19 - Cabin in the Woods)
[ready        ] responsive     -    -- never --    - monitor [on]  (C:\Users\cwood\Desktop\game20 - novela)
[ready        ] responsive     -    -- never --    - monitor [on]  (C:\Users\cwood\Desktop\game21 - nostoi)
[ready        ] responsive     -    -- never --    - monitor [on]  (C:\Users\cwood\Desktop\game22 - House on Haunted Hill)
[ready        ] responsive     - 08/24/22 11:32:10 - compactor
```
This displays all the process(es) running, what they're currently doing, when they last did an operation, and for monitors: which folder they're responsible for.

### ...turn it off, or pause it?
`akiro stop` and `akiro start`

### ...restore something?
First, you probably want to see what versions are available:

```
C:\>akiro timestamps C:\Users\cwood\Desktop\game11
akiro - simple background backups
        https://github.com/cwood77/akiro

[[redirecting from file: C:\Users\cwood\AppData\Local\Temp\akiro_timestamps_0
path 'C:\Users\cwood\Desktop\game11' is root key 0
the following timestamps are available = [
   20220710-122223
   20220823-161611
]
done
]]
```

Some if this is technical detail you might want to ignore, but the important bits are that snapshots are available for the requested folder from 20220710-122223 (i.e. July 10th, 2022, 12:22:23pm) and 20220823-161611 (i.e. August 8th, 2022, 4:16:11pm).  All times are in the local timezone.

You can reconstruct the folder as it was at any of those timestamps with `restore`:

```
C:\>akiro restore C:\Users\cwood\Desktop\game11 20220823-161611 C:\akiro-restore-output
akiro - simple background backups
        https://github.com/cwood77/akiro

this could take awhile
[[redirecting from file: C:\Users\cwood\AppData\Local\Temp\akiro_restore_0
using C:\akiro-archive\f\b6ef6c824cfe5e190022f3964081b144 for C:\akiro-restore-output\backup\220812-1604\Text SEARCHED - tilted.bmp
using C:\akiro-archive\f\4068cc17c1a51d9167d1aed9b47204da for C:\akiro-restore-output\backup\220812-1604\Text SEARCHED.bmp
using C:\akiro-archive\f\ffb322394a4c1a050a0879b00360eb38 for C:\akiro-restore-output\backup\220812-1604\alt room bg.bmp
using C:\akiro-archive\f\3f0ac54192ef84a2855b286928c4ba71 for C:\akiro-restore-output\backup\220812-1604\c 0 merfolk.bmp
using C:\akiro-archive\f\2c5542613391ea81d5df177113b06106 for C:\akiro-restore-output\backup\220812-1604\char template sketching.bmp
using C:\akiro-archive\f\b96eb1e18ed756366763d49b863998af for C:\akiro-restore-output\backup\220812-1604\char template.bmp
using C:\akiro-archive\f\4c868897c95b7c9b51bbd1232151b722 for C:\akiro-restore-output\backup\220812-1604\chits - 2.bmp
using C:\akiro-archive\f\7931d33ff8f1b4a72325ceef441bc395 for C:\akiro-restore-output\backup\220812-1604\chits.bmp
         << I'm truncating some output here as it can be large >>
done
]]
```

Note here that Akiro requires that you pass in a path to restore to.  Akiro never writes to folders it's monitoring.

### ...configure settings?
All settings are in `akiro.txt`, which uses comments to explain what settings are available.  Remember to issue `akiro stop` and `akiro start` if you change this file.

The Akiro configuration I use is available with the source, under the root folder, so feel free to check it out.

## How does Akiro store information?
The configuration file will indicate a backup location, which is a folder that Akiro will manage.  Here's a simplified (the real thing is huge) listing of the backup folder on my machine:
```
C:\>tree /f C:\akiro-archive
Folder PATH listing for volume OSDisk
Volume serial number is 000000A4 10A5:7658
C:\AKIRO-ARCHIVE
│   roots.txt
│
├───f
│       000abce007d488ebb7f9d6423d510d84
│       001cece8e0717a942bde650fa86c89c0
│       0021c59ec70df2fc0484b39506c91991
│       003747a063be365b13f62cb0c06b38a5
│       0038dd6a5ef28b96f175528d7d39c934
│
├───s
└───t
    └───0
            20220710-122223
```

Let's walk though each of these peices.
- The `s` subfolder (for **S**taging) is where monitors copy new data to.  This folder is emptied by the compaction operation once the files are cataloged and unique files added to the `f` folder.
- The `roots.txt` file maps each monitored folder to a number.  If you open this file in notepad the mapping is pretty easy to divine.
- The `t` subfolder (for **T**imestamps) contains a folder for each root, under which a file for each timestamp is stored.  These timestamp files are plain text, and describe the folder tree at the time of the capture, listing each file and its hash
- The `f` subfolder (for **F**ile) contains *all* the files backed up.  All files in the backup are placed in this folder, with their hash as their name.  This allows files with the same contents to only be present once in the data.

## How does Akiro keep the backup from growing forever? (Retention Policies)
Akiro occasionally removes files from the backup to prevent consuming too much space.  You can control the frequency and age of files that are removed in the configuration file.

Retention policies are enforced by two operations in succession, *culling* and *pruning*.
- When *culling*, Akiro considers each timestamp and decides whether it should be deleted or not.  If so, it deletes the timestamp file under the **t** folder.
- When *pruning*, Akiro traverses the entire backup, from roots to timestamps to files, and deletes any files that are unreferenced.

Each of these operations can also be executed via the command-line interface.  So, for example, you could manually delete timestamp files yourself, then issue an `akiro prune` to tidy up the **f** folder.

## What's next for Akiro? Or, what's left undone?
**coming soon**

[^1]: Akiro was the chronicler of Conan in *Conan the Barbarian*.
