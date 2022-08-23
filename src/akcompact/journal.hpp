// what operations might I be in the middle of when a crash happens?
//
// compact
// cull
// prune
//
// alternatively, what files might be corrupt?
//   staging.txt -- use sentinel for this
//   rootDb
//   timestamp
//   file
//
// journal start/end of this operations: rootDB file, timestamp file, file
// command at start-up to do a detect/repair
// command to do a strong file repair (compare files against hash)
//
