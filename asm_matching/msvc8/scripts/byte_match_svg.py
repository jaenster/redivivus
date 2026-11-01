#!/usr/bin/env python3
"""byte_match_svg.py - draw the audit summary's byte kinds as a stacked bar.

    byte_match_svg.py [--summary evidence/audit.tsv.summary] [--out evidence/byte-match]

Reads the `byte kinds` block audit.py writes and draws <out>-light.svg and
<out>-dark.svg: one horizontal bar, a segment per kind in proportion to its
bytes, and a legend with each kind's bytes, share and meaning. Two files with
fixed colours rather than one with a media query: an SVG shown through <img>
follows the operating system's scheme, not the page's, so a README picks the
file with <picture>. Nothing but the numbers in the summary.
"""
import argparse
import os
import re
import sys
from xml.sax.saxutils import escape

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from oracle_config import ORACLE  # noqa: E402

# Kind -> (light fill, dark fill), in the summary's order.
FILLS = {
    'whole': ('#1f6feb', '#4493f8'),
    'ident': ('#79b8ff', '#2f6eb5'),
    'operand': ('#e3b341', '#bb8009'),
    'differ': ('#d1d5da', '#5a6370'),
    'noncode': ('#eceff3', '#3d444d'),
    'uncompared': ('#9aa4ae', '#8b949e'),
}
THEMES = {
    'light': {'bg': '#ffffff', 'text': '#1f2328', 'muted': '#59636e', 'edge': '#d1d9e0', 'ix': 0},
    'dark': {'bg': '#0d1117', 'text': '#f0f6fc', 'muted': '#9198a1', 'edge': '#3d444d', 'ix': 1},
}
LABELS = {
    'whole': 'Whole function identical',
    'ident': 'Matching instructions',
    'operand': 'Same instruction, other operands',
    'differ': 'Different instructions',
    'noncode': 'Tables and padding',
    'uncompared': 'Not compared',
}


def parse(path):
    """(total, [(key, bytes, percent, what)]) from the summary's byte kinds block."""
    total, kinds = None, []
    for line in open(path):
        m = re.match(r'^byte kinds of the headline total: (\d+) bytes', line)
        if m:
            total = int(m.group(1))
            continue
        m = re.match(r'^  kind (\S+)\s+(\d+)\s+([\d.]+)% (.*)$', line.rstrip('\n'))
        if m and total is not None:
            kinds.append((m.group(1), int(m.group(2)), float(m.group(3)), m.group(4)))
    if total is None or not kinds:
        sys.exit('%s: no byte kinds block (run oracle audit)' % path)
    if sum(k[1] for k in kinds) != total:
        sys.exit('%s: the byte kinds do not sum to %d' % (path, total))
    return total, kinds


def svg(total, kinds, theme):
    t = THEMES[theme]
    w, pad, bar_y, bar_h = 800, 24, 52, 30
    row_h, legend_y = 26, bar_y + bar_h + 30
    h = legend_y + row_h * len(kinds) + 12
    font = 'font-family="system-ui, -apple-system, Segoe UI, Helvetica, Arial, sans-serif"'
    out = ['<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" viewBox="0 0 %d %d" %s>' % (
        w, h, w, h, font)]
    out.append('<title>Bytes of the original game code by kind of match</title>')
    out.append('<rect width="%d" height="%d" rx="6" fill="%s"/>' % (w, h, t['bg']))
    out.append('<text x="%d" y="30" font-size="15" font-weight="600" fill="%s">'
               'Original game code, C runtime and compiler EH stubs left out: %s bytes</text>' % (pad, t['text'], format(total, ',')))
    span = w - 2 * pad
    x = float(pad)
    for key, n, pct, _what in kinds:
        seg = span * n / total
        if seg <= 0:
            continue
        out.append('<rect x="%.2f" y="%d" width="%.2f" height="%d" fill="%s"/>' % (
            x, bar_y, seg, bar_h, FILLS.get(key, ('#888', '#888'))[t['ix']]))
        if seg >= 48:
            out.append('<text x="%.2f" y="%d" font-size="12" text-anchor="middle" fill="%s">%.1f%%</text>' % (
                x + seg / 2, bar_y + bar_h / 2 + 4, _ink(FILLS.get(key, ('#888', '#888'))[t['ix']]), pct))
        x += seg
    out.append('<rect x="%d" y="%d" width="%d" height="%d" fill="none" stroke="%s"/>' % (
        pad, bar_y, span, bar_h, t['edge']))
    for k, (key, n, pct, what) in enumerate(kinds):
        y = legend_y + row_h * k
        out.append('<rect x="%d" y="%d" width="14" height="14" rx="2" fill="%s" stroke="%s"/>' % (
            pad, y - 11, FILLS.get(key, ('#888', '#888'))[t['ix']], t['edge']))
        out.append('<text x="%d" y="%d" font-size="13" fill="%s">%s</text>' % (
            pad + 22, y, t['text'], escape(LABELS.get(key, key))))
        out.append('<text x="%d" y="%d" font-size="13" text-anchor="end" fill="%s">%s</text>' % (
            pad + 310, y, t['text'], format(n, ',')))
        out.append('<text x="%d" y="%d" font-size="13" text-anchor="end" fill="%s">%.2f%%</text>' % (
            pad + 374, y, t['text'], pct))
        out.append('<text x="%d" y="%d" font-size="12" fill="%s">%s</text>' % (
            pad + 392, y, t['muted'], escape(what)))
    out.append('</svg>')
    return '\n'.join(out) + '\n'


def _ink(fill):
    """Black or white, whichever reads on `fill`."""
    r, g, b = (int(fill[i:i + 2], 16) for i in (1, 3, 5))
    return '#000000' if 0.299 * r + 0.587 * g + 0.114 * b > 140 else '#ffffff'


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--summary', default=os.path.join(ORACLE, 'evidence', 'audit.tsv.summary'))
    ap.add_argument('--out', default=os.path.join(ORACLE, 'evidence', 'byte-match'))
    args = ap.parse_args()
    total, kinds = parse(args.summary)
    for theme in THEMES:
        path = '%s-%s.svg' % (args.out, theme)
        with open(path, 'w') as fh:
            fh.write(svg(total, kinds, theme))
        print(path)


if __name__ == '__main__':
    main()
