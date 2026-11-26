/**
 * Where a link or an image may point. A renderer never follows the `url` of a node: it asks
 * here and acts on the answer.
 *
 * A link is `external` (http or https, a plain host, no credentials), `package` (a path
 * inside the mod's own archive) or `blocked`. An image is `package` or `blocked`: nothing is
 * ever loaded from another host, and no data is read out of a URL.
 */

import { isAlpha, isDigit, lower } from './bytes';
import { MAX_URL_BYTES } from './limits';

export interface Target {
  kind: 'external' | 'package' | 'blocked';
  /** `external`: the URL, as written. `package`: the path, `/` between directories, no fragment. Otherwise ''. */
  url: string;
  /** `blocked`: why, as one of the words in the README. Otherwise ''. */
  reason: string;
}

/** The longest name an archive holds. */
const MAX_PATH_BYTES = 200;

function blocked(reason: string): Target {
  return { kind: 'blocked', url: '', reason };
}

/** Bytes no destination may hold: they end an attribute, start markup or are read as a `/` by a browser. */
function isUnsafe(c: number): boolean {
  return c === 34 || c === 39 || c === 60 || c === 62 || c === 92 || c === 94 || c === 96 || c === 123 || c === 124 || c === 125;
}

function isHost(host: string): boolean {
  const n = host.length;
  let e = 0;
  while (e < n && (isAlpha(host.charCodeAt(e)) || isDigit(host.charCodeAt(e)) || host.charCodeAt(e) === 46 || host.charCodeAt(e) === 45)) e += 1;
  if (e === 0 || host.charCodeAt(0) === 46 || host.charCodeAt(0) === 45) return false;
  if (e === n) return true;
  if (host.charCodeAt(e) !== 58 || e + 1 >= n || n - e - 1 > 5) return false;
  for (let i = e + 1; i < n; i++) {
    if (!isDigit(host.charCodeAt(i))) return false;
  }
  return true;
}

/** A path inside the archive, or why it is not one. */
function packagePath(path: string): Target {
  const n = path.length;
  if (n === 0) return blocked('empty');
  const parts: string[] = [];
  let from = 0;
  let leading = true;
  for (let i = 0; i <= n; i++) {
    const c = i < n ? path.charCodeAt(i) : 47;
    if (c === 47) {
      const seg = path.substring(from, i);
      from = i + 1;
      if (seg === '..') return blocked('path_traversal');
      if (seg === '.' && leading) {
        // `./` at the start says nothing.
      } else if (seg.length === 0) {
        return blocked(i === 0 ? 'absolute_path' : 'bad_path');
      } else if (seg === '.' || seg.charCodeAt(seg.length - 1) === 46) {
        return blocked('bad_path');
      } else {
        parts.push(seg);
        leading = false;
      }
    } else if (c === 37) {
      return blocked('percent_encoding');
    } else if (c === 58 || c === 42 || c === 59 || c === 63) {
      return blocked('bad_path');
    }
  }
  if (parts.length === 0) return blocked('empty');
  const clean = parts.join('/');
  if (clean.length > MAX_PATH_BYTES) return blocked('too_long');
  return { kind: 'package', url: clean, reason: '' };
}

export function classifyLink(url: string): Target {
  const n = url.length;
  if (n === 0) return blocked('empty');
  if (n > MAX_URL_BYTES) return blocked('too_long');
  let scheme = -1;
  let stop = -1;
  let hash = -1;
  for (let i = 0; i < n; i++) {
    const c = url.charCodeAt(i);
    if (c <= 32 || c === 127) return blocked('control_character');
    if (c >= 128) return blocked('non_ascii');
    if (isUnsafe(c)) return blocked('bad_character');
    if (stop === -1 && (c === 58 || c === 47 || c === 63 || c === 35)) {
      stop = i;
      if (c === 58) scheme = i;
    }
    if (hash === -1 && c === 35) hash = i;
  }
  if (scheme !== -1) {
    const name = lower(url.substring(0, scheme));
    if (name !== 'http' && name !== 'https') return blocked('scheme');
    if (scheme + 2 >= n || url.charCodeAt(scheme + 1) !== 47 || url.charCodeAt(scheme + 2) !== 47) return blocked('bad_host');
    let e = scheme + 3;
    while (e < n && url.charCodeAt(e) !== 47 && url.charCodeAt(e) !== 63 && url.charCodeAt(e) !== 35) {
      if (url.charCodeAt(e) === 64) return blocked('credentials');
      e += 1;
    }
    if (!isHost(url.substring(scheme + 3, e))) return blocked('bad_host');
    return { kind: 'external', url, reason: '' };
  }
  const first = url.charCodeAt(0);
  if (first === 47) return blocked(n > 1 && url.charCodeAt(1) === 47 ? 'protocol_relative' : 'absolute_path');
  if (first === 35) return blocked('fragment');
  if (first === 63) return blocked('query');
  return packagePath(hash === -1 ? url : url.substring(0, hash));
}

/** An image is shown only from the mod's own archive. */
export function classifyImage(url: string): Target {
  const t = classifyLink(url);
  if (t.kind === 'external') return blocked('remote_image');
  return t;
}
