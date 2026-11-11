"""ts_translate.py - the VC8 translation done by ghidra-reconstruct, for build.py.

ghidra-reconstruct's VC8 output mode (packages/reconstruct/src/vc8) makes the same
rewrites as vc8_translate.py from the C++ token stream instead of regular
expressions, and keys the __thiscall functions on their qualified name, so a
same-named function in another namespace or class no longer strips one of its
convention. GR=<ghidra-reconstruct checkout> selects it. Without GR, the live
daemon's checkout (~/code/ts/gr-daemon, recon/runs/start-live.sh) is used when it
has the VC8 mode and a built cpp-parser; otherwise build.py translates with
vc8_translate.py.

One process serves the whole build: it reads the tree and runs the pre-pass
once, mirrors the tree into build/src, then translates single files on request
(a quarantine round, a support TU) with the same per-tree state.
"""
import base64
import json
import os
import subprocess
import threading

CLI = os.path.join('packages', 'reconstruct', 'src', 'vc8', 'cli.ts')
# The parser the CLI imports, resolved as the CLI resolves it: through the
# reconstruct package's workspace link to cpp-parser's built dist/.
PARSER = os.path.join('packages', 'reconstruct', 'node_modules', '@ghidra-mcp', 'cpp-parser',
                      'dist', 'index.js')
# The live daemon's checkout, detached at LIVE_REF (recon/runs/start-live.sh).
DAEMON = os.path.join(os.path.expanduser('~'), 'code', 'ts', 'gr-daemon')


def checkout():
    """The ghidra-reconstruct checkout to translate with, or None for
    vc8_translate.py. GR names one; a GR without the VC8 mode is an error, not
    a silent fall-back. Without GR, the daemon's checkout when it has the VC8
    mode and a built parser."""
    gr = os.environ.get('GR')
    if gr:
        if not os.path.exists(os.path.join(gr, CLI)):
            raise SystemExit('GR=%s has no %s' % (gr, CLI))
        return gr
    if all(os.path.exists(os.path.join(DAEMON, p)) for p in (CLI, PARSER)):
        return DAEMON
    return None


class Translator:
    """The serving translator for tree `repo`. mirror() runs first."""

    def __init__(self, gr, repo):
        self.gr, self.repo = gr, repo
        self.proc = None
        self.lock = threading.Lock()

    def mirror(self, out, quarantine_tsv):
        """Translate every source into `out`; returns (written, unchanged)."""
        args = ['pnpm', '--silent', '--dir', self.gr, 'exec', 'tsx', CLI, self.repo, out, '--serve']
        if os.path.exists(quarantine_tsv):
            args += ['--quarantine', quarantine_tsv]
        self.proc = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True,
                                     encoding='utf-8')
        line = self.proc.stdout.readline()
        if not line.startswith('vc8_translate: '):
            self.proc.wait()
            raise SystemExit('ghidra-reconstruct translation failed (exit %s)' % self.proc.returncode)
        words = line.split()
        return int(words[1]), int(words[3])

    def _ask(self, req):
        with self.lock:
            self.proc.stdin.write(json.dumps(req) + '\n')
            self.proc.stdin.flush()
            reply = json.loads(self.proc.stdout.readline())
        if 'error' in reply:
            raise SystemExit('%s: %s' % (os.path.join(self.repo, req['rel']), reply['error']))
        return reply

    def translate(self, rel, lines):
        """File `rel` of the tree translated, the functions holding `lines` quarantined."""
        data = base64.b64decode(self._ask({'rel': rel, 'lines': sorted(lines)})['text'])
        return data.decode('utf-8', errors='surrogateescape')

    def functions(self, rel, lines, at):
        """For each 1-based line of `at`: (line of the function body's `{`, name) of
        the function it lies in, in the translation under `lines`; None outside one."""
        found = self._ask({'rel': rel, 'lines': sorted(lines), 'at': list(at)})['functions']
        return [(f['line'], f['name']) if f else None for f in found]

    def close(self):
        if self.proc:
            self.proc.stdin.close()
            self.proc.wait()
            self.proc = None
