"""ours.py - every function in the compiled objects, with what it refers to.

Loading decodes each object once and caches the result by what decoding
reads of the object (load_object, content_key), keyed also by DECODER,
which a change to how objects are decoded must bump.
"""
import os
import pickle

import objfuncs

DECODER = 4     # 2: switch tables found through the relocation on the displacement; 3: a same-section address modulo 2**32, another function's entry as fptr; 4: a relocated branch leaves the function


class OurFunc:
    """The parts of objfuncs.Function the matcher and the audit need, picklable."""
    __slots__ = ('tu', 'symbol', 'name', 'order', 'size', 'insns', 'strings', 'imports', 'calls',
                 'datarefs', 'static', 'decoded', 'refs', 'lines')

    def __init__(self, tu, order, f):
        self.tu = tu                    # 'D2Common/Environment' (object path without .obj)
        self.symbol = f.symbol          # mangled
        self.name = f.name              # qualified, readable
        self.order = order              # position of its COMDAT in the object
        self.size = len(f.code)
        self.insns = len(f.decoded.insns)
        self.static = f.static
        self.refs = f.refs              # (offset, kind, target)
        self.decoded = f.decoded
        self.strings = [t for _o, k, t in f.refs if k == 'string']
        self.imports = [t for _o, k, t in f.refs if k == 'import']
        self.calls = [t for _o, k, t in f.refs if k == 'call']
        self.datarefs = [t for _o, k, t in f.refs if k == 'data']
        self.lines = f.lines            # (offset, source file, line) from /Z7

    def line_at(self, off):
        """(source file, line) of the instruction at `off`, or None."""
        best = None
        for o, fn, ln in self.lines:
            if o <= off:
                best = (fn, ln)
            else:
                break
        return best

    @property
    def source_line(self):
        """The first line of its own .cpp it was compiled from: where it is
        defined. None for code from headers only (inline functions, templates)."""
        own = self.tu.lower() + '.cpp'
        lines = [ln for _o, fn, ln in self.lines if fn.lower().replace('\\', '/').endswith(own)]
        return min(lines) if lines else None

    @property
    def layout_key(self):
        """Sort key for the order of the TU's functions in the original. The
        generator writes a file's functions in address order, so the source
        line predicts the original layout; the object's COMDAT order does not:
        VC8 compiles a caller only after the same-TU functions it calls, and
        emits a function calling one defined further down (or an inline
        helper) at the end of the object. Functions with no line of their own
        go last, in object order."""
        ln = self.source_line
        return (0, ln, self.order) if ln is not None else (1, 0, self.order)

    @property
    def key(self):
        return self.tu + '|' + self.symbol


def content_key(path):
    """A hash of what decoding reads of the object at `path`: every section
    but the debug ones (name, flags, bytes, relocations), the symbol table
    without the debug sections' aux records, and the line tables of
    .debug$S as decoded. A recompile of the same code changes the time stamp
    in the header, and a rebuilt precompiled header the signatures in
    .debug$S and .debug$T; neither changes this key."""
    import hashlib
    import coffobj
    o = coffobj.CoffObject(open(path, 'rb').read(), path)
    h = hashlib.sha256()
    debug = set()
    for s in o.sections[1:]:
        if s.name.startswith('.debug'):
            debug.add(s.number)
            h.update(repr((s.number, s.name)).encode('latin1'))
            continue
        h.update(repr((s.number, s.name, s.flags, s.size,
                       [(r.offset, r.symbol, r.type) for r in s.relocs])).encode('latin1'))
        h.update(s.data)
    for y in o.symbols:
        if y is not None:
            h.update(repr((y.index, y.name, y.value, y.section, y.type, y.storage,
                           None if y.section in debug else y.aux)).encode('latin1'))
    h.update(repr(sorted(objfuncs.line_tables(o).items())).encode('utf-8', 'surrogateescape'))
    return h.hexdigest()


def load_object(objdir, rel):
    """Every function of one object, decoded. Cached by what decoding reads
    of the object (content_key), not by its time or its whole bytes: beside
    it (<obj>.fn.pkl) and in the shared cache (objcache.py), so an object a
    build restored or recompiled to the same code is not decoded again.
    <obj>.fn.sum pairs the object's file hash with its content key, so an
    object unchanged since the last load is not parsed for the key either.
    ORACLE_NOCACHE=1 decodes afresh."""
    import objcache
    path = os.path.join(objdir, rel)
    tu = rel[:-4]
    cache = path[:-4] + '.fn.pkl'
    sumfile = path[:-4] + '.fn.sum'
    fast = objcache.sha_file(path)
    key = None
    try:
        with open(sumfile) as fh:
            was, k = fh.read().split()
        if was == fast:
            key = k
    except (OSError, ValueError):
        pass
    if key is None:
        key = objcache.sha_bytes('fn', str(DECODER), tu, content_key(path))
        tmp = '%s.%d.tmp' % (sumfile, os.getpid())
        with open(tmp, 'w') as fh:
            fh.write('%s %s\n' % (fast, key))
        os.replace(tmp, sumfile)
    if objcache.READ:
        for p in (cache, os.path.join(objcache.get('fn', key) or '', 'fn.pkl')):
            if os.path.exists(p):
                try:
                    with open(p, 'rb') as fh:
                        stamp, fns = pickle.load(fh)
                    if stamp == key:
                        if p != cache:
                            objcache.install(p, cache)
                        return fns
                except Exception:
                    pass                # half-written by a process killed mid-write: decode again
    fns = [OurFunc(tu, k, f) for k, f in enumerate(objfuncs.functions(path))]
    # Written aside and renamed, so a reader sees the old cache or the new one.
    tmp = '%s.%d.tmp' % (cache, os.getpid())
    with open(tmp, 'wb') as fh:
        pickle.dump((key, fns), fh, protocol=pickle.HIGHEST_PROTOCOL)
    os.replace(tmp, cache)
    objcache.put('fn', key, {'fn.pkl': cache})
    return fns


def object_keys(objdir):
    """(rel, content hash) of every completed object, as load_all reads them:
    what anything derived from all our functions depends on."""
    import objcache
    out = []
    for dirpath, dirs, files in os.walk(objdir):
        dirs.sort()                     # the same order on every filesystem
        for f in sorted(files):
            if f.endswith('.obj') and f[:-4] + '.d' in files:
                p = os.path.join(dirpath, f)
                out.append((os.path.relpath(p, objdir), objcache.sha_file(p)))
    return sorted(out)


def load_all(objdir):
    """Every function of every completed object. build.py removes an object
    and its .d before compiling and writes the .d only on success, so an
    object without one is half-written or failed, and is skipped."""
    out = []
    for dirpath, dirs, files in os.walk(objdir):
        dirs.sort()                     # the same order on every filesystem
        for f in sorted(files):
            if f.endswith('.obj') and f[:-4] + '.d' in files:
                rel = os.path.relpath(os.path.join(dirpath, f), objdir)
                out.extend(load_object(objdir, rel))
    return out


def link_order(tus):
    """Our TUs in the order recon/runs/build-vc8.sh hands their objects to
    the linker: grouped into the module libraries of Source/modules.cmake in
    the executable's link order, each library's TUs through sort(1)
    (modules.py), whether the build links the libraries or the objects."""
    import modules
    from oracle_config import REPO
    return modules.link_order(tus, REPO)


def link_kept(fns):
    """The functions the link keeps: of an external COMDAT that several
    objects define (an inline function, a template instantiation: one copy
    in every object that uses it, all selectany), the copy of the first
    object in link order, as link.exe keeps the first definition it reads
    and discards the rest. A static function is its object's own and always
    kept. The copies are not interchangeable: objects of other directories
    compile with other flags (/GS-, /O1)."""
    rank = {t: i for i, t in enumerate(link_order(f.tu for f in fns))}
    first = {}
    for f in fns:
        if not f.static and (f.symbol not in first or rank[f.tu] < rank[first[f.symbol].tu]):
            first[f.symbol] = f
    return [f for f in fns if f.static or first[f.symbol] is f]


_fingerprints = {}


def fingerprint(f):
    """A hash of everything a comparison reads of f: its symbol, its decoded
    instructions (address and bytes) and its references. Two functions with
    the same fingerprint compare the same against any original function."""
    k = id(f)
    v = _fingerprints.get(k)
    if v is None or v[0] is not f:
        import hashlib
        h = hashlib.sha256()
        h.update(f.symbol.encode('utf-8', 'surrogateescape') + b'\0')
        for i in f.decoded.insns:
            h.update(i.addr.to_bytes(4, 'little') + len(i.bytes).to_bytes(1, 'little') + i.bytes)
        h.update(repr(f.refs).encode('utf-8', 'surrogateescape'))
        v = _fingerprints[k] = (f, h.hexdigest())
    return v[1]


_ids = {}


def scripts_id():
    """Hash of the oracle's scripts: a change to any of them may change any
    derived result, so it invalidates every memo keyed by this."""
    if 'scripts' not in _ids:
        import objcache
        here = os.path.dirname(os.path.abspath(__file__))
        names = sorted(n for n in os.listdir(here) if n.endswith('.py'))
        _ids['scripts'] = objcache.sha_bytes(*[n + objcache.sha_file(os.path.join(here, n)) for n in names])
    return _ids['scripts']


def reference_id():
    """Hash of the original image and its carve: the other side of every comparison."""
    if 'ref' not in _ids:
        import objcache
        from oracle_config import ORACLE, REFCACHE
        _ids['ref'] = objcache.sha_bytes(*[objcache.sha_file(p) for p in (
            os.path.join(ORACLE, 'reference', 'Game.exe'), os.path.join(REFCACHE, 'functions.tsv'))])
    return _ids['ref']


class PairMemo:
    """Results that depend only on one of our functions' code and one
    original function's, kept across runs in <build>/<name>.memo.pkl: keyed by
    the function's content (fingerprint) and the original's start, under
    the reference image and the scripts that compute them. Entries not used
    in a run are dropped when it saves. ORACLE_NOCACHE=1 reads nothing."""

    def __init__(self, name, build):
        import objcache
        self.path = os.path.join(build, name + '.memo.pkl')
        self.stamp = (scripts_id(), reference_id())
        self.old, self.new = {}, {}
        if objcache.READ and os.path.exists(self.path):
            try:
                import pickle
                with open(self.path, 'rb') as fh:
                    stamp, table = pickle.load(fh)
                if stamp == self.stamp:
                    self.old = table
            except Exception:
                pass

    def get(self, f, s, extra=()):
        k = (fingerprint(f), s, extra)
        v = self.old.get(k)
        if v is not None:
            self.new[k] = v
        return v

    def put(self, f, s, v, extra=()):
        self.new[(fingerprint(f), s, extra)] = v
        return v

    def save(self):
        import pickle
        os.makedirs(os.path.dirname(self.path), exist_ok=True)
        tmp = '%s.%d.tmp' % (self.path, os.getpid())
        with open(tmp, 'wb') as fh:
            pickle.dump((self.stamp, self.new), fh, protocol=pickle.HIGHEST_PROTOCOL)
        os.replace(tmp, self.path)


def stage_key(name, *inputs):
    """The key of a whole derivation (funcmap, libmap): its name, the scripts,
    the reference and whatever else it reads, given as strings."""
    import objcache
    return objcache.sha_bytes(name, scripts_id(), reference_id(), *inputs)


def stage_reuse(name, build, key, outputs):
    """True if `outputs` already hold the result for `key` - in place, or
    restored from the shared cache. Then the derivation need not run: its
    result is a function of its inputs, and the key covers them all."""
    import json
    import objcache
    if not objcache.READ:
        return False
    rec = os.path.join(build, name + '.stage.json')
    try:
        with open(rec) as fh:
            r = json.load(fh)
        if r['key'] == key and all(objcache.sha_file(p) == r['outputs'].get(p) for p in outputs):
            return True
    except (OSError, ValueError, KeyError):
        pass
    d = objcache.get('stage', key)
    if d is None:
        return False
    for k, p in enumerate(outputs):
        os.makedirs(os.path.dirname(os.path.abspath(p)), exist_ok=True)
        objcache.install(os.path.join(d, str(k)), p)
    stage_done(name, build, key, outputs)
    return True


def stage_done(name, build, key, outputs):
    import json
    import objcache
    objcache.put('stage', key, {str(k): p for k, p in enumerate(outputs)})
    os.makedirs(build, exist_ok=True)
    tmp = os.path.join(build, '%s.stage.json.%d.tmp' % (name, os.getpid()))
    with open(tmp, 'w') as fh:
        json.dump({'key': key, 'outputs': {p: objcache.sha_file(p) for p in outputs}}, fh)
    os.replace(tmp, os.path.join(build, name + '.stage.json'))
