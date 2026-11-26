import { For, h, Fragment, Match, Show, signal, stats, Switch, type Widget, Direction, SizeMode, UIEventType } from "redivivus:ui";
import { byId, count, dump, eq, find, mountTest, ok, test } from "./harness";

void Fragment;

function labels(root: Widget): string[] {
    const out: string[] = [];
    const walk = (w: Widget): void => {
        if (w.kind === "label" && typeof w.props.text === "string") out.push(w.props.text as string);
        for (let i = 0; i < w.children.length; i++) walk(w.children[i]);
    };
    walk(root);
    return out;
}

test("virtualised list: only the visible rows exist, scrolling swaps them", () => {
    const items: number[] = [];
    for (let i = 0; i < 1000; i++) items.push(i);
    let created = 0;
    const { ui } = mountTest(() => (
        <list
            width={200}
            height={100}
            rowHeight={20}
            items={items}
            render={(n: number) => {
                created++;
                return <label text={`row ${n}`} />;
            }}
            testId="l"
        />
    ));
    ui.layout();
    const l = byId(ui.root, "l");
    eq(l.children.length, 5);
    eq(labels(ui.root), ["row 0", "row 1", "row 2", "row 3", "row 4"]);
    ui.dispatch({ type: UIEventType.Wheel, x: 10, y: 10, delta: 1 });
    ui.layout();
    eq(l.scrollY, 24);
    eq(labels(ui.root), ["row 1", "row 2", "row 3", "row 4", "row 5", "row 6"], "a partly visible row exists");
    eq(created, 7, "rows that stayed were kept");
    l.scrollTo(1000 * 20);
    ui.layout();
    eq(l.scrollY, 19900, "clamped");
    eq(labels(ui.root), ["row 995", "row 996", "row 997", "row 998", "row 999"]);
});

test("virtualised list: rows are disposed when they leave and when items change", () => {
    const [items, setItems] = signal(["a", "b", "c"]);
    const before = stats.owners;
    const m = mountTest(() => <list width={100} height={100} rowHeight={20} items={items} render={(s: string) => <label text={s} />} />);
    m.ui.layout();
    eq(labels(m.ui.root), ["a", "b", "c"]);
    setItems(["a", "x"]);
    m.ui.layout();
    eq(labels(m.ui.root), ["a", "x"]);
    m.ui.dispose();
    eq(stats.owners, before, "no owner left behind");
});

test("Show: creates and disposes its children, with a fallback", () => {
    const [on, setOn] = signal(false);
    let made = 0;
    const before = stats.owners;
    const m = mountTest(() => (
        <frame>
            <Show when={on} fallback={<label text="off" />}>
                {() => {
                    made++;
                    return <label text="on" />;
                }}
            </Show>
        </frame>
    ));
    eq(labels(m.ui.root), ["off"]);
    setOn(true);
    eq(labels(m.ui.root), ["on"]);
    setOn(true);
    eq(made, 1);
    setOn(false);
    setOn(true);
    eq(made, 2);
    m.ui.dispose();
    eq(stats.owners, before);
});

test("Show keyed passes the value and re-creates on a new one", () => {
    const [v, setV] = signal<string | undefined>("x");
    const m = mountTest(() => (
        <frame>
            <Show when={v} keyed>
                {(s: string) => <label text={`v=${s}`} />}
            </Show>
        </frame>
    ));
    eq(labels(m.ui.root), ["v=x"]);
    setV("y");
    eq(labels(m.ui.root), ["v=y"]);
    setV(undefined);
    eq(labels(m.ui.root), []);
    m.ui.dispose();
});

test("For: keyed rows keep their widgets on reorder, removed rows are disposed, index updates", () => {
    const a = { id: "a" };
    const b = { id: "b" };
    const c = { id: "c" };
    const [list, setList] = signal([a, b, c]);
    const before = stats.owners;
    const m = mountTest(() => (
        <flow direction={Direction.Column} testId="col">
            <For each={list}>{(it: { id: string }, i: () => number) => <label text={() => `${i()}:${it.id}`} testId={it.id} />}</For>
        </flow>
    ));
    const wb = byId(m.ui.root, "b");
    eq(labels(m.ui.root), ["0:a", "1:b", "2:c"]);
    setList([c, b]);
    eq(labels(m.ui.root), ["0:c", "1:b"]);
    ok(byId(m.ui.root, "b") === wb, "b kept its widget");
    eq(find(m.ui.root, (w) => w.props.testId === "a") === undefined, true, "a removed");
    m.ui.dispose();
    eq(stats.owners, before);
});

test("Switch / Match: the first match wins, else the fallback", () => {
    const [n, setN] = signal(1);
    const m = mountTest(() => (
        <frame>
            <Switch fallback={<label text="none" />}>
                <Match when={() => n() > 5}>{() => <label text="big" />}</Match>
                <Match when={() => n() > 0}>{() => <label text="positive" />}</Match>
            </Switch>
        </frame>
    ));
    eq(labels(m.ui.root), ["positive"]);
    setN(9);
    eq(labels(m.ui.root), ["big"]);
    setN(0);
    eq(labels(m.ui.root), ["none"]);
    m.ui.dispose();
});

test("markdown: blocks become widgets", () => {
    const src = ["# Title", "", "Text with *em*, **strong**, `code` and a [link](https://x.org).", "", "- one", "- two", "", "1. first", "", "```", "code line", "```", "", "> quote", "", "| a | b |", "|:-|-:|", "| 1 | 2 |", "", "![alt text](docs/a.png)", "", "---"].join("\n");
    const m = mountTest(() => <markdown source={src} />, 400, 400);
    m.ui.layout();
    const r = m.ui.root;
    const rich = (w: Widget): boolean => w.kind === "rich";
    eq(count(r, rich), 10, "heading, paragraph, three list items, the quote, four table cells");
    const heading = find(r, rich) as Widget;
    eq(heading.props.spans, [{ colour: "heading", font: "h1", text: "Title" }]);
    const para = find(r, (w) => rich(w) && (w.props.spans as unknown[]).length > 3) as Widget;
    eq(para.props.spans, [
        { colour: "text", font: "body", text: "Text with " },
        { colour: "emphasis", font: "body", text: "em" },
        { colour: "text", font: "body", text: ", " },
        { colour: "strong", font: "body", text: "strong" },
        { colour: "text", font: "body", text: ", " },
        { colour: "code", font: "mono", text: "code" },
        { colour: "text", font: "body", text: " and a " },
        { colour: "link", font: "body", text: "link" },
        { colour: "text", font: "body", text: "." },
    ]);
    eq(count(r, (w) => w.kind === "frame" && w.props.style === "code"), 1, "code block");
    eq(count(r, (w) => w.kind === "frame" && w.props.style === "quote"), 1, "quote");
    eq(count(r, (w) => w.kind === "grid"), 1, "table");
    eq(count(r, (w) => w.kind === "image"), 1, "image placeholder");
    eq(count(r, (w) => w.kind === "rule"), 1, "rule");
    eq(labels(r), ["-", "-", "1.", "code line"], "list markers and code");
    m.ui.dispose();
});

test("markdown: a parse error is shown, and a new source rebuilds", () => {
    const [src, setSrc] = signal("ok");
    const m = mountTest(() => <markdown source={src} />);
    m.ui.layout();
    eq(count(m.ui.root, (w) => w.kind === "rich"), 1);
    setSrc(String.fromCharCode(1));
    eq(labels(m.ui.root).length, 1);
    ok(labels(m.ui.root)[0].indexOf("README line 1") === 0, labels(m.ui.root)[0]);
    m.ui.dispose();
});

test("mount and dispose leave nothing behind (Mods-like tree)", () => {
    const beforeOwners = stats.owners;
    const beforeComp = stats.computations;
    const [q, setQ] = signal("");
    const m = mountTest(() => (
        <frame width={SizeMode.Fill} height={SizeMode.Fill}>
            <textField value={q} onInput={setQ} />
            <For each={() => [1, 2, 3]}>{(n: number) => <checkbox label={`${n}`} checked={() => q().length > n} />}</For>
            <markdown source={() => `# ${q()}`} />
        </frame>
    ));
    m.ui.frame();
    setQ("abcd");
    m.ui.frame();
    ok(dump(m.ui.root).length > 5, "built");
    m.ui.dispose();
    eq(stats.owners, beforeOwners, "owners");
    eq(stats.computations, beforeComp, "computations");
});
