"""oracle_config.py - where a build lives, decided from its configuration.

The same rule as scripts/env.sh, so a script run directly behaves exactly as
through `oracle`: the default configuration (SP1 compiler, default flags)
builds in build/, and anything else in build/experiment-<compiler>-<hash>/,
where it can never overwrite the objects the evidence is made from. An
experiment that names the baseline directory explicitly is refused.
"""
import hashlib
import os

ORACLE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
REPO = os.path.dirname(os.path.dirname(ORACLE))
DEFAULT_CFLAGS = '/O2 /Oy- /Ob1 /EHsc /GS /MT /W1 /D WIN32 /D NDEBUG /D _WINDOWS'
# Flags for the oracle's own use, not a claim about the original: /Z7 puts
# CodeView line tables in each object, so a diff can name the source line of
# every instruction. Codegen-neutral: `oracle objdiff` found all 318 functions of 8
# objects identical with and without it.
TOOL_CFLAGS = '/Z7'

VC = os.environ.get('ORACLE_VC', 'sp1')
CFLAGS = os.environ.get('ORACLE_CFLAGS') or DEFAULT_CFLAGS
EXPERIMENT = VC != 'sp1' or CFLAGS != DEFAULT_CFLAGS
BASELINE_BUILD = os.path.join(ORACLE, 'build')
REFCACHE = os.path.join(BASELINE_BUILD, 'ref')      # the carved original: one per checkout


def _build_dir():
    explicit = os.environ.get('ORACLE_BUILD')
    if EXPERIMENT:
        tag = hashlib.md5(os.environ.get('ORACLE_CFLAGS', '').encode()).hexdigest()[:8]
        default = os.path.join(BASELINE_BUILD, 'experiment-%s-%s' % (VC, tag))
        if explicit and os.path.realpath(explicit) == os.path.realpath(BASELINE_BUILD):
            raise SystemExit('refusing to build an experiment (ORACLE_VC=%s, ORACLE_CFLAGS=%r) into the '
                             'baseline directory %s' % (VC, CFLAGS, BASELINE_BUILD))
        return explicit or default
    return explicit or BASELINE_BUILD


BUILD = _build_dir()
