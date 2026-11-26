import { h, Fragment, signal, type UIEvent, ButtonVariant, Direction, SizeMode, UIEventType } from "redivivus:ui";
import { byId, eq, mountTest, test } from "./harness";

void Fragment;

function click(ui: { dispatch: (e: { type: UIEventType; x: number; y: number }) => boolean }, x: number, y: number): void {
    ui.dispatch({ type: UIEventType.MouseDown, x, y });
    ui.dispatch({ type: UIEventType.MouseUp, x, y });
}

test("click bubbles from the button to its parents; a handler can stop it", () => {
    const log: string[] = [];
    const { ui } = mountTest(() => (
        <frame width={200} height={100} onClick={() => log.push("root")}>
            <flow onClick={(e: UIEvent) => log.push(`row ${e.target === undefined ? "" : e.target.kind}`)}>
                <button variant={ButtonVariant.Small} label="Go" onClick={() => log.push("button")} testId="b" />
                <button variant={ButtonVariant.Small} label="Stop" onClick={(e: UIEvent) => {
                    log.push("stop");
                    e.stopped = true;
                }} testId="s" />
            </flow>
        </frame>
    ));
    ui.layout();
    const b = byId(ui.root, "b");
    click(ui, b.x + 2, b.y + 2);
    eq(log, ["button", "row button", "root"]);
    const s = byId(ui.root, "s");
    click(ui, s.x + 2, s.y + 2);
    eq(log, ["button", "row button", "root", "stop"]);
});

test("a disabled button gets no click", () => {
    let n = 0;
    const { ui } = mountTest(() => <button variant={ButtonVariant.Small} label="No" disabled onClick={() => n++} testId="b" />);
    ui.layout();
    click(ui, 3, 3);
    eq(n, 0);
});

test("capture: a press that is released outside is not a click", () => {
    let n = 0;
    const { ui } = mountTest(() => (
        <frame width={300} height={100}>
            <button variant={ButtonVariant.Small} label="Hold" onClick={() => n++} testId="b" />
        </frame>
    ));
    ui.layout();
    ui.dispatch({ type: UIEventType.MouseDown, x: 3, y: 3 });
    ui.dispatch({ type: UIEventType.MouseMove, x: 250, y: 80 });
    eq(ui.ctx.capture === byId(ui.root, "b"), true, "held while outside");
    ui.dispatch({ type: UIEventType.MouseUp, x: 250, y: 80 });
    eq(n, 0);
    eq(ui.ctx.capture === undefined, true, "released");
    click(ui, 3, 3);
    eq(n, 1);
});

test("hover follows the pointer", () => {
    const { ui } = mountTest(() => (
        <flow width={300} height={50}>
            <frame width={100} height={SizeMode.Fill} testId="a" />
            <frame width={100} height={SizeMode.Fill} testId="b" />
        </flow>
    ));
    ui.layout();
    ui.dispatch({ type: UIEventType.MouseMove, x: 150, y: 10 });
    eq(ui.ctx.hover === byId(ui.root, "b"), true);
    ui.dispatch({ type: UIEventType.MouseMove, x: 50, y: 10 });
    eq(ui.ctx.hover === byId(ui.root, "a"), true);
});

test("text field: focus by press, text and editing keys, enter submits, tab moves focus", () => {
    const [value, setValue] = signal("ab");
    let submitted = "";
    const { ui } = mountTest(() => (
        <flow direction={Direction.Column} width={300} height={100} gap={4}>
            <textField value={value} onInput={setValue} onSubmit={(v: string) => (submitted = v)} testId="f1" />
            <textField testId="f2" />
        </flow>
    ));
    ui.layout();
    const f1 = byId(ui.root, "f1");
    click(ui, f1.x + 5, f1.y + 5);
    eq(ui.ctx.focus === f1, true, "focused");
    ui.dispatch({ type: UIEventType.Text, text: "cd" });
    eq(value(), "abcd");
    ui.dispatch({ type: UIEventType.KeyDown, key: 37 });
    ui.dispatch({ type: UIEventType.KeyDown, key: 8 });
    eq(value(), "abd", "backspace before the caret");
    ui.dispatch({ type: UIEventType.KeyDown, key: 36 });
    ui.dispatch({ type: UIEventType.Text, text: "X" });
    eq(value(), "Xabd");
    ui.dispatch({ type: UIEventType.KeyDown, key: 13 });
    eq(submitted, "Xabd");
    ui.dispatch({ type: UIEventType.KeyDown, key: 9 });
    eq(ui.ctx.focus === byId(ui.root, "f2"), true, "tab");
    ui.dispatch({ type: UIEventType.Text, text: "typed" });
    eq(byId(ui.root, "f2").props.value, "typed", "uncontrolled field keeps its own value");
});

test("keys go to the focused widget and bubble", () => {
    const keys: number[] = [];
    const { ui } = mountTest(() => (
        <frame width={300} height={100} onKeyDown={(e: UIEvent) => keys.push(e.key)}>
            <textField testId="f" />
        </frame>
    ));
    ui.layout();
    click(ui, 5, 5);
    ui.dispatch({ type: UIEventType.KeyDown, key: 112 });
    eq(keys, [112], "F1 is not a field key, so it bubbles");
});

test("wheel scrolls the scroll pane under the pointer and stops there", () => {
    let outer = 0;
    const { ui } = mountTest(() => (
        <frame width={200} height={200} onWheel={() => outer++}>
            <scrollPane height={50} testId="sp">
                <frame height={200} />
            </scrollPane>
        </frame>
    ));
    ui.layout();
    ui.dispatch({ type: UIEventType.Wheel, x: 10, y: 10, delta: 1 });
    eq(byId(ui.root, "sp").scrollY, 24);
    eq(outer, 0);
    ui.dispatch({ type: UIEventType.Wheel, x: 10, y: 150, delta: 1 });
    eq(outer, 1, "outside the pane");
});

test("checkbox and tabs report changes", () => {
    const [on, setOn] = signal(false);
    const [tab, setTab] = signal(0);
    const { ui } = mountTest(() => (
        <flow direction={Direction.Column} width={300} height={100}>
            <checkbox label="Opt" checked={on} onChange={setOn} testId="c" />
            <tabs labels={["One", "Two", "Three"]} selected={tab} onChange={setTab} testId="t" />
        </flow>
    ));
    ui.layout();
    const c = byId(ui.root, "c");
    click(ui, c.x + 3, c.y + 3);
    eq(on(), true);
    click(ui, c.x + 3, c.y + 3);
    eq(on(), false);
    const t = byId(ui.root, "t");
    // Tabs: every label fits the smallest tab art, 96 wide, with 2-pixel gaps: "Two" spans 98..193.
    click(ui, t.x + 100, t.y + 5);
    eq(tab(), 1);
    click(ui, t.x + 200, t.y + 5);
    eq(tab(), 2);
});

test("hit testing respects clipping", () => {
    let n = 0;
    const { ui } = mountTest(() => (
        <frame width={200} height={200}>
            <frame width={50} height={50} clip>
                <frame anchor={{ left: 60, top: 0 }} width={20} height={20} onClick={() => n++} />
            </frame>
        </frame>
    ));
    ui.layout();
    click(ui, 65, 5);
    eq(n, 0, "outside the clip");
});
