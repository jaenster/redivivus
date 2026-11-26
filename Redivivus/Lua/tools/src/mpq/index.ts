/**
 * MPQ for Redivivus mods: a strict reader and a deterministic writer for the restricted
 * profile a mod ships in (README.md), plus the low-level pieces of format version 1.
 * No runtime-specific imports: it runs in Node and in a Worker alike.
 */

export { MpqError, type MpqErrorCode, type MpqIssue } from './error.ts';
export { build, hashTableSize, type InputFile } from './build.ts';
export { DEFAULT_LIMITS, list, read, SECTOR_SHIFT, SECTOR_SIZE, validate, type FileInfo, type Limits, type ValidateResult, type Visit } from './profile.ts';
export { checkName, LISTFILE, MAX_NAME_LENGTH, nameKey, normalizeName, SPECIAL_FILES } from './names.ts';
export * as crypt from './crypt.ts';
export * as format from './format.ts';
export { adler32, inflateZlib } from './inflate.ts';
