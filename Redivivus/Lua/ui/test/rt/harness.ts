// A test harness that runs the same in Node and under LuaJIT (it compiles with TypeScriptToLua):
// tests register with test(), runAll() runs them and returns the report lines.

import { HeadlessMetrics, mount, Recorder, testTheme, Widget, type Child, type Theme, type UIRoot } from "redivivus:ui";

type TestFn = () => void;
interface Case {
    name: string;
    fn: TestFn;
}

const cases: Case[] = [];

export function test(name: string, fn: TestFn): void {
    cases.push({ name, fn });
}

function keysOf(o: { [k: string]: unknown }): string[] {
    const keys: string[] = [];
    for (const k in o) keys.push(k);
    keys.sort();
    return keys;
}

/** A deterministic text form of a value, the same in both languages. */
export function show(v: unknown): string {
    if (v === undefined) return "undefined";
    if (v === null) return "null";
    if (typeof v === "string") return `"${v}"`;
    if (typeof v === "number" || typeof v === "boolean") return `${v}`;
    if (typeof v === "function") return "function";
    if (v instanceof Widget) return `Widget(${v.kind})`;
    if (Array.isArray(v)) {
        const parts: string[] = [];
        for (let i = 0; i < v.length; i++) parts.push(show(v[i]));
        return `[${parts.join(", ")}]`;
    }
    const o = v as { [k: string]: unknown };
    const parts: string[] = [];
    const keys = keysOf(o);
    for (let i = 0; i < keys.length; i++) parts.push(`${keys[i]}: ${show(o[keys[i]])}`);
    return `{${parts.join(", ")}}`;
}

export function eq(actual: unknown, expected: unknown, what?: string): void {
    const a = show(actual);
    const e = show(expected);
    if (a !== e) throw new Error(`${what ?? "value"}: expected ${e}, got ${a}`);
}

export function ok(cond: boolean, what: string): void {
    if (!cond) throw new Error(`not true: ${what}`);
}

export interface Report {
    passed: number;
    failed: number;
    lines: string[];
}

/** Forgets every registered test (the Node runner reuses this module for the host tests). */
export function clear(): void {
    cases.length = 0;
}

export function runAll(): Report {
    const r: Report = { passed: 0, failed: 0, lines: [] };
    for (let i = 0; i < cases.length; i++) {
        const c = cases[i];
        let err: string | undefined = undefined;
        try {
            c.fn();
        } catch (e) {
            const m = e instanceof Error ? e.message : `${e}`;
            err = m;
        }
        if (err === undefined) {
            r.passed++;
            r.lines.push(`ok   ${c.name}`);
        } else {
            r.failed++;
            r.lines.push(`FAIL ${c.name}: ${err}`);
        }
    }
    return r;
}

// --- helpers --------------------------------------------------------------------------------------

export interface Mounted {
    ui: UIRoot;
    rec: Recorder;
}

/** Mounts a view on the headless recorder with the test theme (or another theme). */
export function mountTest(view: () => Child, width = 400, height = 300, theme?: Theme): Mounted {
    const rec = new Recorder(new HeadlessMetrics());
    const ui = mount(view, rec, theme ?? testTheme(), width, height);
    return { ui, rec };
}

/** The box of a widget as "x y w h". */
export function box(w: Widget): string {
    return `${w.x} ${w.y} ${w.w} ${w.h}`;
}

/** The first widget (depth first) for which pred holds. */
export function find(w: Widget, pred: (w: Widget) => boolean): Widget | undefined {
    if (pred(w)) return w;
    for (let i = 0; i < w.children.length; i++) {
        const r = find(w.children[i], pred);
        if (r !== undefined) return r;
    }
    return undefined;
}

export function byId(root: Widget, id: string): Widget {
    const w = find(root, (x) => x.props.testId === id);
    if (w === undefined) throw new Error(`no widget with testId ${id}`);
    return w;
}

export function count(w: Widget, pred: (w: Widget) => boolean): number {
    let n = pred(w) ? 1 : 0;
    for (let i = 0; i < w.children.length; i++) n += count(w.children[i], pred);
    return n;
}

/** The tree as one line per widget: depth, kind, box and the text it shows. */
export function dump(w: Widget, depth = 0, out: string[] = []): string[] {
    let pad = "";
    for (let i = 0; i < depth; i++) pad += "  ";
    const t = w.props.text;
    out.push(`${pad}${w.kind} ${box(w)}${typeof t === "string" ? ` "${t}"` : ""}`);
    for (let i = 0; i < w.children.length; i++) dump(w.children[i], depth + 1, out);
    return out;
}
