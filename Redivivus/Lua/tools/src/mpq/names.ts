/**
 * What a file name in a mod archive may be. Storm folds ASCII case and treats '/' as '\',
 * so names are compared upper-cased and written with backslashes; everything a name could
 * mean something else by on a player's disk is refused.
 */

import { MpqError } from './error.ts';

export const LISTFILE = '(listfile)';

/** Storm's own bookkeeping files. Only the listfile is part of the profile, and it is never listed. */
export const SPECIAL_FILES = ['(listfile)', '(attributes)', '(signature)', '(user data)', '(patch_metadata)'];

export const MAX_NAME_LENGTH = 200;

/** Printable ASCII without the characters Windows refuses in a file name, ';' (a listfile separator) and '/'. */
const NAME_CHARS = /^[\x20-\x7e]+$/;
const FORBIDDEN_CHARS = /[/:*?"<>|;]/;
const DEVICE = /^(CON|PRN|AUX|NUL|COM[0-9]|LPT[0-9])(\..*)?$/;

/** The form names are compared in: two names with the same key are the same file to Storm. */
export function nameKey(name: string): string {
  return name.toUpperCase();
}

/** A path as a packer finds it on disk, with the separators an archive uses. */
export function normalizeName(path: string): string {
  return path.replace(/\//g, '\\');
}

/** Throws the first reason a name is not allowed in a mod archive. */
export function checkName(name: string, maxLength = MAX_NAME_LENGTH): void {
  if (!name.length) throw new MpqError('bad_name', 'a file name is empty', name);
  if (name.length > maxLength) throw new MpqError('name_too_long', `a file name is longer than ${maxLength} characters`, name.slice(0, 80));
  if (!NAME_CHARS.test(name)) throw new MpqError('bad_name', 'a file name may hold printable ASCII only', name);
  const forbidden = FORBIDDEN_CHARS.exec(name);
  if (forbidden) {
    throw forbidden[0] === ':' && /^[A-Za-z]:/.test(name)
      ? new MpqError('absolute_path', 'a file name may not start with a drive', name)
      : new MpqError('bad_name', `a file name may not hold '${forbidden[0]}'; directories are separated with '\\'`, name);
  }
  if (name.startsWith('\\')) throw new MpqError('absolute_path', 'a file name may not start with a separator', name);
  if (SPECIAL_FILES.includes(name.toLowerCase())) throw new MpqError('special_file', `${name} is reserved for the archive itself`, name);
  for (const part of name.split('\\')) {
    if (part === '') throw new MpqError('bad_name', 'a file name may not hold an empty path segment', name);
    if (part === '.' || part === '..') throw new MpqError('path_traversal', "a file name may not hold a '.' or '..' segment", name);
    if (part.endsWith('.') || part.endsWith(' ') || part.startsWith(' ')) {
      throw new MpqError('bad_name', 'a path segment may not start with a space or end with a space or a dot', name);
    }
    if (DEVICE.test(part.toUpperCase())) throw new MpqError('reserved_name', `${part} is a device name on Windows`, name);
  }
}
