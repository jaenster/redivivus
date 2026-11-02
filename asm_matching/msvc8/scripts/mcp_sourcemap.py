#!/usr/bin/env python3
"""mcp_sourcemap.py - the generator's .cpp.map files, rebuilt from the Ghidra database.

    mcp_sourcemap.py SYMBOLS_TSV [--dry-run]
    mcp_sourcemap.py --changed BEFORE_FUNCMAP [AFTER_FUNCMAP]

funcmap.py's `sourcemap` tier reads `<tu>.cpp.map` next to each source file:
`{"functions": [{"name", "namespace", "address"}, ...]}`, written by
ghidra-reconstruct's codegen/sourcemap.ts. The repository ignores those files,
so a checkout has none, but they hold nothing the Redivivus MCP server does not
serve: `list_symbols` with `type: "FUNCTION"` returns every function's name,
namespace and address, and with a `regex` such as `^(name|name|...)$` those of
a batch of names.

SYMBOLS_TSV is that data, one function per line, `name<TAB>namespace<TAB>address`
(8 hex digits), as an agent collects it through the MCP server. For every
function our objects define, this writes its database address into its TU's
.cpp.map when exactly one database function has its namespace and name. A
function renamed in the database since the tree was generated is not found and
stays with the other tiers; the database is live, the tree a snapshot of it.
Existing .cpp.map files are merged with, not replaced: an entry they already
have for a function is kept.

Every function's symbol is a long fetch; the names evidence/funcmap.tsv.unmapped
lists are a short one, but a partial list moves pairs it does not name. A
function it pins can lie in the original inside another file's code, and
funcmap.py pairs nothing by position in a gap that holds another file's code,
so functions it had paired there by position come out unmapped; a pinned
function can also shift a run of look-alikes paired by position. --changed
lists those: every function BEFORE_FUNCMAP (evidence/funcmap.tsv kept from the
run before) pairs and AFTER_FUNCMAP (by default evidence/funcmap.tsv) does not,
or pairs elsewhere, as `tu<TAB>name` like funcmap.tsv.unmapped. Their symbols,
added to SYMBOLS_TSV, pin them where the database has them.
"""
import collections
import json
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import ours as ourslib  # noqa: E402
from oracle_config import BUILD, ORACLE, REPO  # noqa: E402


def load_symbols(path):
    """(namespace, name) -> [address, ...] from the TSV."""
    out = collections.defaultdict(list)
    for line in open(path, encoding='utf-8'):
        parts = line.rstrip('\n').split('\t')
        if len(parts) != 3 or not parts[0]:
            continue
        name, ns, addr = parts
        try:
            a = int(addr, 16)
        except ValueError:
            continue
        if a not in out[(ns, name)]:
            out[(ns, name)].append(a)
    return out


def load_pairs(path):
    """(symbol, tu) -> (start, tu, name) for every function a funcmap.tsv pairs."""
    out = {}
    for line in open(path, encoding='utf-8'):
        c = line.rstrip('\n').split('\t')
        if len(c) >= 5 and c[0] != 'start':
            out[(c[4], c[3])] = (c[0], c[3], c[2])
    return out


def changed(before, after):
    """Print the functions `before` pairs and `after` does not, or elsewhere."""
    new = load_pairs(after)
    seen = set()
    for key, (start, tu, name) in sorted(load_pairs(before).items(), key=lambda kv: kv[1]):
        if (key in new and new[key][0] == start) or (tu, name) in seen:
            continue
        seen.add((tu, name))
        print('%s\t%s' % (tu, name))


def write_maps(symbols_tsv, dry):
    symbols = load_symbols(symbols_tsv)
    per_tu = collections.defaultdict(dict)
    seen = ambiguous = 0
    for f in ourslib.load_all(os.path.join(BUILD, 'obj')):
        ns, _, leaf = f.name.rpartition('::')
        hits = symbols.get((ns, leaf))
        if not hits:
            continue
        seen += 1
        if len(hits) != 1:
            ambiguous += 1
            continue
        per_tu[f.tu][f.name] = {'name': leaf, 'namespace': ns, 'address': '%08x' % hits[0]}
    written = added = 0
    for tu, entries in sorted(per_tu.items()):
        path = os.path.join(REPO, tu + '.cpp.map')
        doc = {'functions': []}
        if os.path.exists(path):
            try:
                doc = json.load(open(path))
            except ValueError:
                doc = {'functions': []}
        have = {((e.get('namespace') + '::') if e.get('namespace') else '') + e.get('name', '')
                for e in doc.get('functions', [])}
        new = [e for q, e in sorted(entries.items()) if q not in have]
        if not new:
            continue
        doc.setdefault('functions', []).extend(new)
        added += len(new)
        written += 1
        if not dry:
            with open(path, 'w') as fh:
                json.dump(doc, fh, indent=1)
                fh.write('\n')
    print('%d of our functions found in the symbols (%d ambiguous, left out); %d entries added to %d .cpp.map files%s'
          % (seen, ambiguous, added, written, ' (dry run)' if dry else ''))


def main():
    args = [a for a in sys.argv[1:] if a != '--dry-run']
    if args[:1] == ['--changed'] and len(args) in (2, 3):
        changed(args[1], args[2] if len(args) == 3 else os.path.join(ORACLE, 'evidence', 'funcmap.tsv'))
    elif len(args) == 1 and not args[0].startswith('--'):
        write_maps(args[0], '--dry-run' in sys.argv)
    else:
        sys.exit(__doc__)


if __name__ == '__main__':
    main()
