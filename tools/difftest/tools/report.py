#!/usr/bin/env python3
"""Summarise a difftest results.jsonl as markdown.

  report.py <results.jsonl> <spec.txt>

Buckets per function:
  equivalent   every input set agreed (a fault counts as agreeing when both sides
               raised the same exception; the addresses differ by construction)
  different    at least one input set disagreed on a value, a write or a register
  crash        one side faulted or hung where the other returned
The verdict on each difference (bug or harness artifact) is a human call and
belongs in the run's report, not here.
"""
import json, sys

FAIL = ('diff', 'fault_orig_only', 'fault_ours_only', 'timeout_one_side')


def bucket(d):
    if d['diff']:
        return 'different'
    if d['fault_orig_only'] or d['fault_ours_only'] or d['timeout_one_side']:
        return 'crash'
    return 'equivalent'


def main():
    res = [json.loads(l) for l in open(sys.argv[1]) if l.strip()]
    wedged = [d for d in res if d.get('wedged')]
    res = [d for d in res if not d.get('wedged')]
    vc8 = {}
    tsv = sys.argv[2] + '.tsv'
    try:
        for line in open(tsv):
            c = line.rstrip('\n').split('\t')
            vc8[c[0]] = c[2]
    except OSError:
        pass
    by = {'equivalent': [], 'different': [], 'crash': []}
    for d in res:
        by[bucket(d)].append(d)
    print(f'# difftest summary\n\n{len(res)} functions, {res[0]["iters"] if res else 0} input sets each.\n')
    print('|bucket|functions|VC8 MATCH among them|\n|-|-|-|')
    for k, v in by.items():
        print(f'|{k}|{len(v)}|{sum(1 for d in v if vc8.get(d["orig"]) == "MATCH")}|')
    print('\n|1.14d|function|VC8|equal|equal fault|diff|fault orig only|fault ours only|timeout|verdict|\n|-|-|-|-|-|-|-|-|-|-|')
    for d in res:
        to = d['equal_timeout'] + d['timeout_one_side']
        print(f"|{d['orig']}|{d['name']}|{vc8.get(d['orig'], '?')}|{d['equal']}|{d['equal_fault']}|{d['diff']}|"
              f"{d['fault_orig_only']}|{d['fault_ours_only']}|{to}|{bucket(d)}|")
    for d in res:
        if bucket(d) == 'equivalent':
            continue
        m = d.get('minimal') or {}
        print(f"\n## {d['orig']} {d['name']}\n")
        print(f"- first failure: `{d['first_failure']['what']}`")
        print(f"- minimal input: `{json.dumps(m.get('input'))}`")
        print(f"- minimal diff: `{m.get('what')}`")
        if d.get('abi'):
            print(f"- ABI note: `{d['abi']}`")
    if wedged:
        print('\n## Wedged\n\nThe host stopped making progress (a wild write into wine\'s own memory); '
              'no verdict:\n')
        for d in wedged:
            print(f"- id {d['id']} {d.get('name')}")
    weak = [d for d in res if d['equal'] < d['iters'] // 10]
    if weak:
        print('\n## Weak coverage\n\nFewer than 10% of input sets returned normally on both sides, '
              'so equivalence rests mostly on agreeing faults:\n')
        for d in weak:
            print(f"- {d['orig']} {d['name']}: {d['equal']} returned, {d['equal_fault']} faulted on both")


if __name__ == '__main__':
    main()
