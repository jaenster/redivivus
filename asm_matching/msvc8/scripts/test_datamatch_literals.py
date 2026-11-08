#!/usr/bin/env python3
"""Tests for datamatch.py's string-literal pairing: which original strings
count as the generator's literals, and how a use of ours is judged.

    python3 -m unittest asm_matching/msvc8/scripts/test_datamatch_literals.py
"""
import os
import struct
import sys
import unittest

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import datamatch as dm  # noqa: E402


class FakeSection:
    def __init__(self, va, vsize):
        self.va, self.vsize = va, vsize

    @property
    def end(self):
        return self.va + self.vsize


class FakePE:
    """An image with .text at 0x401000 and .rdata at 0x6cc000, from a dict of
    address -> bytes and a list of (relocation VA, the address it holds)."""
    base = 0x400000

    def __init__(self, mem, relocs):
        self.mem = dict(mem)
        for va, target in relocs:
            for k, b in enumerate(struct.pack('<I', target)):
                self.mem[va + k] = b
        self.relocs = {va for va, _t in relocs}

    def read(self, va, n):
        return bytes(self.mem.get(va + k, 0) for k in range(n))

    def u32(self, va):
        return struct.unpack('<I', self.read(va, 4))[0]

    def cstring(self, va, limit=256):
        b = self.read(va, limit)
        return b[:b.find(b'\0')] if b'\0' in b else b

    def section(self, name):
        return {'.text': FakeSection(0x1000, 0x2cb000), '.rdata': FakeSection(0x2cc000, 0x39000)}[name]

    def section_at(self, va):
        return None

    def put(self, va, data):
        for k, b in enumerate(data):
            self.mem[va + k] = b


def item(addr, size, name, typ='char[8]', kind='data', ns='', refs=()):
    it = dm.Item()
    it.addr, it.size, it.extent, it.name, it.aliases = addr, size, size, name, ''
    it.scope, it.ns, it.owner, it.type, it.kind, it.refs, it.parent = '', ns, '', typ, kind, list(refs), None
    return it


RO = [(0x6cc000, 0x705000)]


class Shape(unittest.TestCase):
    def test_terminated_and_padded(self):
        self.assertEqual(dm.string_literal_shape(b'Russian\0'), (b'Russian', None))
        self.assertEqual(dm.string_literal_shape(b'ab\0\0'), (b'ab', None))

    def test_rejects(self):
        self.assertIsNotNone(dm.string_literal_shape(b'abcd')[1])            # no terminator
        self.assertIsNotNone(dm.string_literal_shape(b'a\0b\0')[1])          # two strings
        self.assertIsNotNone(dm.string_literal_shape(b'ab\0\0\0\0\0\0')[1])  # larger than padded to 4
        self.assertIsNotNone(dm.string_literal_shape(b'\0\0\0\0')[1])        # empty

    def test_pooled_extent(self):
        self.assertEqual([dm.pooled_extent(n) for n in (0, 3, 4, 7)], [4, 4, 8, 8])

    def test_label(self):
        self.assertEqual(dm.string_label(0x6cc838, 'Diablo 2 EXE'), 's_Diablo_2_EXE_006cc838')


class Candidates(unittest.TestCase):
    def setUp(self):
        self.pe = FakePE({}, [])
        self.pe.put(0x6cc7a4, b'Russian\0')
        self.pe.put(0x6cc838, b'Diablo 2 EXE\0\0\0\0')
        self.pe.put(0x6cc900, b'abc\0')
        self.pe.put(0x6cc910, b'two\0str\0')
        self.pe.put(0x6cca00, b'Writable\0')

    def test_typed_and_untyped(self):
        items = [item(0x6cc7a4, 8, 's_Russian_006cc7a4', ns='Fog::Src::ErrorManager')]
        strings = [(0x6cc838, 12, 'string', 's_Diablo_2_EXE_006cc838')]
        lits = dm.literal_candidates(items, strings, self.pe.read, set(), RO)
        self.assertEqual([(l.addr, l.text, l.source) for l in lits],
                         [(0x6cc7a4, b'Russian', 'typed'), (0x6cc838, b'Diablo 2 EXE', 'untyped')])

    def test_paired_by_name_is_not_a_literal(self):
        items = [item(0x6cc7a4, 8, 's_Russian_006cc7a4')]
        self.assertEqual(dm.literal_candidates(items, [], self.pe.read, {0x6cc7a4}, RO), [])

    def test_two_strings_in_one_item_is_a_table(self):
        items = [item(0x6cc910, 8, 'gaPair')]
        self.assertEqual(dm.literal_candidates(items, [], self.pe.read, set(), RO), [])

    def test_label_inside_keeps_it(self):
        # A label inside the string: code reaches a suffix, so the generator keeps the datum.
        items = [item(0x6cc7a4, 8, 's_Russian_006cc7a4'), item(0x6cc7a8, 4, 's_ian_006cc7a8', kind='interior')]
        lits = dm.literal_candidates(items, [], self.pe.read, set(), RO)
        self.assertNotIn(0x6cc7a4, [l.addr for l in lits])

    def test_outside_read_only(self):
        items = [item(0x6cca00, 9, 'gszWritable', typ='char[9]')]
        self.assertEqual(dm.literal_candidates(items, [], self.pe.read, set(), [(0x6cc000, 0x6cca00)]), [])

    def test_not_a_string_type(self):
        items = [item(0x6cc900, 4, 'gdwMagic', typ='uint')]
        self.assertEqual(dm.literal_candidates(items, [], self.pe.read, set(), RO), [])

    def test_generator_report_overrides_the_rule(self):
        items = [item(0x6cc7a4, 8, 's_Russian_006cc7a4'), item(0x6cc900, 4, 's_abc_006cc900', typ='char[4]')]
        lits = dm.literal_candidates(items, [], self.pe.read, set(), RO, generator={0x6cc900})
        self.assertEqual([l.addr for l in lits], [0x6cc900])


class Judge(unittest.TestCase):
    def test_match_at_every_use(self):
        st, how, _n = dm.judge_literal(b'Russian', [('f', {b'Russian', b'x'}, set()), ('t+44', {b'Russian'}, set())],
                                       set())
        self.assertEqual((st, how), ('MATCH', 'use'))

    def test_other_content_is_string(self):
        st, _h, notes = dm.judge_literal(b'Windows XP', [('f', {b'Windows XQ'}, set())], set())
        self.assertEqual(st, 'STRING')
        self.assertIn('Windows XQ', notes[0])

    def test_one_use_differs(self):
        st, _h, _n = dm.judge_literal(b'abcdef', [('f', {b'abcdef'}, set()), ('g', {b'abcdeg'}, set())], set())
        self.assertEqual(st, 'STRING')

    def test_the_originals_other_string_is_not_a_misspelling(self):
        # Our function names "MacOS X", which the original function names too:
        # that is the other string, so "MacOS" is missing, not misspelt.
        st, _h, _n = dm.judge_literal(b'MacOS', [('f', {b'MacOS X'}, {b'MacOS X', b'MacOS'})], set())
        self.assertEqual(st, 'MISSING')

    def test_missing_notes_the_image(self):
        st, _h, notes = dm.judge_literal(b'unknown', [('f', {b'Diablo II'}, {b'unknown'})], {b'unknown'})
        self.assertEqual(st, 'MISSING')
        self.assertIn('the image holds it elsewhere', notes[0])

    def test_object_of_the_use_holds_it(self):
        st, how, _n = dm.judge_literal(b'Caught', [('f', set(), set())], set(), [{b'Caught'}])
        self.assertEqual((st, how), ('MATCH', 'tu'))

    def test_no_use(self):
        self.assertEqual(dm.judge_literal(b'x', [], {b'x'})[0], 'UNUSED')


class FakeOurs:
    def __init__(self, fns, by_tu=None):
        self.fns, self.by_tu = fns, by_tu or {}
        self.everywhere = set().union(*fns.values()) if fns else set()

    def of_function(self, tu, sym):
        return self.fns.get((tu, sym))


class Rows(unittest.TestCase):
    """literal_rows pairs through the original's relocations."""

    def setUp(self):
        dm.LITERAL_FIELDS.clear()
        self.pe = FakePE({}, [(0x401005, 0x6cc7a4), (0x402005, 0x6cc838), (0x6f0000, 0x6cc900),
                              (0x403005, 0x6cc910)])
        self.pe.put(0x6cc7a4, b'Russian\0')
        self.pe.put(0x6cc838, b'Diablo II\0\0\0')
        self.pe.put(0x6cc900, b'abc\0')
        self.pe.put(0x6cc910, b'rt\0\0')
        self.pe.put(0x6cc920, b'dead\0')
        self.fm = [(0x401000, 0x401100, 'Fog::Src::ErrorManager::Write', 'Source/Fog/Src/ErrorManager', '?Write@'),
                   (0x402000, 0x402100, 'D2Client::Main', 'Source/D2Client/Main', '?Main@')]

    def lits(self):
        items = [item(0x6cc7a4, 8, 's_Russian_006cc7a4', ns='Fog::Src::ErrorManager')]
        strings = [(0x6cc838, 9, 'string', 's_Diablo_II_006cc838'), (0x6cc900, 3, 'string', 's_abc_006cc900'),
                   (0x6cc910, 2, 'string', 's_rt_006cc910'), (0x6cc920, 4, 'string', 's_dead_006cc920')]
        return dm.literal_candidates(items, strings, self.pe.read, set(), RO)

    def test_pairing(self):
        ours = FakeOurs({('Source/Fog/Src/ErrorManager', '?Write@'): {b'Russian'},
                         ('Source/D2Client/Main', '?Main@'): {b'Diablo 2'}})
        dm.LITERAL_FIELDS[0x6cc900].append(('gaTable+4', b'abc'))
        rows = {r[0]: r for r in dm.literal_rows(self.pe, self.lits(), self.fm, ours)}
        self.assertEqual(rows[0x6cc7a4][11], 'MATCH')            # code use, our function names it
        self.assertEqual(rows[0x6cc7a4][4], 'Fog')
        self.assertEqual(rows[0x6cc838][11], 'STRING')           # code use, ours spells another
        self.assertEqual(rows[0x6cc838][4], 'D2Client')           # module from the reader
        self.assertEqual(rows[0x6cc900][11], 'MATCH')            # data use: the pointer field of ours
        self.assertEqual(rows[0x6cc910][11], 'UNPAIRED')         # used only by code the funcmap does not pair
        self.assertEqual(rows[0x6cc920][11], 'UNUSED')           # nothing in the original points at it
        self.assertEqual(rows[0x6cc7a4][8], 'strlit')
        self.assertEqual(rows[0x6cc7a4][15], 8)                   # matched bytes: the original extent


if __name__ == '__main__':
    unittest.main()
