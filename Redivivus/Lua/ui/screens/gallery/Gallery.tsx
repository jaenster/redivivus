// Widget gallery sheets: every intrinsic element in its states, for previews and snapshot tests.

import { For, h, Fragment, colourCode, signal, type Child, type UIRoot, type Widget, Align, BoxStyle, ButtonVariant, Direction, FontRole, SizeMode, TextAlign, TextColour, UIEventType } from "redivivus:ui";

void Fragment;

function Section(props: { title: string; children?: unknown }): Child {
    return (
        <frame style={BoxStyle.Panel} padding={6} gap={6} direction={Direction.Column}>
            <label font={FontRole.H3} colour={TextColour.Heading} text={props.title} wrap={false} />
            {props.children as Child}
        </frame>
    );
}

/** Sheet 1: controls. */
export function ControlsSheet(): Child {
    const [checked, setChecked] = signal(true);
    const [text, setText] = signal("Hello, Sanctuary");
    const [tab, setTab] = signal(1);
    return (
        <frame style={BoxStyle.Screen} width={SizeMode.Fill} height={SizeMode.Fill} padding={10} gap={8} direction={Direction.Column}>
            <label font={FontRole.Title} colour={TextColour.Heading} text="Controls" wrap={false} />
            <flow gap={8} align={Align.Start} wrap>
                <Section title="Buttons">
                    <button variant={ButtonVariant.Wide} label="Single Player" />
                    <button variant={ButtonVariant.Medium} label="Medium" />
                    <flow gap={6}>
                        <button variant={ButtonVariant.Cancel} label="Cancel" />
                        <button variant={ButtonVariant.Short} label="Short" pressed />
                    </flow>
                    <button variant={ButtonVariant.Narrow} label="Narrow (disabled)" disabled />
                    <flow gap={6}>
                        <button variant={ButtonVariant.Small} label="Fit" />
                        <button variant={ButtonVariant.Small} label="Fit to a longer label" />
                    </flow>
                </Section>
                <Section title="Inputs">
                    <checkbox label="Checked" checked={checked} onChange={setChecked} />
                    <checkbox label="Unchecked" checked={false} />
                    <checkbox label="Disabled" checked disabled />
                    <textField value={text} onInput={setText} width={220} />
                    <textField placeholder="Placeholder text" width={220} />
                    <tabs labels={["One", "Two", "Three"]} selected={tab} onChange={setTab} />
                </Section>
                <Section title="Bars and art">
                    <bar width={220} value={70} max={100} text="Life 70/100" />
                    <bar width={220} value={30} max={100} rgb={0x2040c0} text="Mana 30/100" />
                    <bar width={220} value={0} max={100} />
                    <flow gap={8}>
                        <sprite path="test:icon" />
                        <sprite path="data\global\ui\FrontEnd\clickbox" frame={1} />
                        <itemSlot cols={2} rows={3} />
                        <itemSlot cols={1} rows={1} />
                    </flow>
                    <tooltip text="A tooltip shows over everything, near the pointer.">
                        <label text="Hover me (tooltip)" colour={TextColour.Link} />
                    </tooltip>
                </Section>
            </flow>
        </frame>
    );
}

/** Sheet 2: layout and text. */
export function LayoutSheet(): Child {
    const long = "The quick brown fox jumps over the lazy dog. Words wrap at spaces and a word longer than the line, Pneumonoultramicroscopicsilicovolcanoconiosis, breaks between characters.";
    return (
        <frame style={BoxStyle.Screen} width={SizeMode.Fill} height={SizeMode.Fill} padding={10} gap={8} direction={Direction.Column}>
            <label font={FontRole.Title} colour={TextColour.Heading} text="Layout and text" wrap={false} />
            <Section title="Row: fixed 60, content, fill, fill x2, min 40">
                <flow gap={4} height={24} align={Align.Stretch}>
                    <frame style={BoxStyle.Inset} width={60} />
                    <frame style={BoxStyle.Inset} padding={4}>
                        <label text="content" wrap={false} />
                    </frame>
                    <frame style={BoxStyle.Inset} width={SizeMode.Fill} />
                    <frame style={BoxStyle.Inset} width={SizeMode.Fill} flex={2} />
                    <frame style={BoxStyle.Inset} width={SizeMode.Fill} maxWidth={40} />
                </flow>
            </Section>
            <flow gap={8} height={150} align={Align.Stretch}>
                <Section title="Wrap flow">
                    <flow wrap gap={4} width={220}>
                        <For each={["alpha", "beta", "gamma", "delta", "epsilon", "zeta", "eta", "theta", "iota", "kappa"]}>
                            {(w: string) => (
                                <frame style={BoxStyle.Inset} padding={3}>
                                    <label text={w} wrap={false} />
                                </frame>
                            )}
                        </For>
                    </flow>
                </Section>
                <Section title="Grid 3 columns">
                    <grid columns={3} gap={4} width={200}>
                        <For each={[1, 2, 3, 4, 5, 6, 7]}>
                            {(n: number) => (
                                <frame style={BoxStyle.Inset} padding={2}>
                                    <label text={`cell ${n}`} textAlign={TextAlign.Center} />
                                </frame>
                            )}
                        </For>
                    </grid>
                </Section>
                <frame style={BoxStyle.Panel} width={SizeMode.Fill} padding={6}>
                    <label text="anchored top-left" anchor={{ left: 0, top: 0 }} wrap={false} />
                    <label text="top-right" anchor={{ right: 0, top: 0 }} wrap={false} />
                    <label text="centre" anchor={{ center: true }} colour={TextColour.Heading} wrap={false} />
                    <label text="bottom-left" anchor={{ left: 0, bottom: 0 }} wrap={false} />
                    <label text="bottom-right" anchor={{ right: 0, bottom: 0 }} wrap={false} />
                </frame>
            </flow>
            <flow gap={8} align={Align.Start}>
                <Section title="Text">
                    <label width={300} text={long} />
                    <label width={300} textAlign={TextAlign.Center} text={`${colourCode(1)}Red ${colourCode(2)}green ${colourCode(3)}blue ${colourCode(4)}gold ${colourCode(8)}orange ${colourCode(9)}yellow ${colourCode(5)}grey`} />
                    <label width={300} textAlign={TextAlign.Right} text={"Right aligned\nsecond line"} />
                    <label width={300} text={`Missing glyphs: ${String.fromCharCode(128)}${String.fromCharCode(7)} become a placeholder`} />
                </Section>
                <Section title="Scroll pane, clipped">
                    <scrollPane width={220} height={110} style={BoxStyle.Inset} padding={4}>
                        <For each={[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]}>{(n: number) => <label text={`Line ${n} of a scrolled column`} wrap={false} />}</For>
                    </scrollPane>
                </Section>
            </flow>
        </frame>
    );
}

/** Sheet 3: a virtualised list and Markdown. */
export function ListSheet(): Child {
    const items: number[] = [];
    for (let i = 0; i < 500; i++) items.push(i);
    const [sel, setSel] = signal(3);
    const md = ["## Markdown", "", "A paragraph with *emphasis*, **strong**, `code` and a [link](https://example.com).", "", "- one", "- two", "  - nested", "", "1. first", "2. second", "", "> quoted", "", "| a | b |", "|--|--:|", "| 1 | 22 |", "", "```", "fenced code", "```"].join("\n");
    return (
        <frame style={BoxStyle.Screen} width={SizeMode.Fill} height={SizeMode.Fill} padding={10} gap={8} direction={Direction.Column}>
            <label font={FontRole.Title} colour={TextColour.Heading} text="List and Markdown" wrap={false} />
            <flow gap={8} height={SizeMode.Fill} align={Align.Stretch}>
                <frame style={BoxStyle.Panel} width={260} padding={6} direction={Direction.Column} gap={4}>
                    <label colour={TextColour.Muted} text="500 rows, only the visible ones exist" />
                    <list
                        items={items}
                        rowHeight={18}
                        height={SizeMode.Fill}
                        selected={sel}
                        onSelect={setSel}
                        render={(n: number) => (
                            <flow gap={6} padding={[0, 4, 0, 4]}>
                                <label width={40} colour={TextColour.Muted} text={`${n}`} wrap={false} />
                                <label width={SizeMode.Fill} text={`Row number ${n}`} wrap={false} />
                            </flow>
                        )}
                    />
                </frame>
                <frame style={BoxStyle.Panel} width={SizeMode.Fill} padding={6} direction={Direction.Column}>
                    <scrollPane height={SizeMode.Fill}>
                        <markdown source={md} />
                    </scrollPane>
                </frame>
            </flow>
        </frame>
    );
}

export const sheets: { [name: string]: (this: void) => Child } = {
    controls: ControlsSheet,
    layout: LayoutSheet,
    list: ListSheet,
};

function findKind(w: Widget, kind: string): Widget | undefined {
    if (w.kind === kind) return w;
    for (let i = 0; i < w.children.length; i++) {
        const r = findKind(w.children[i], kind);
        if (r !== undefined) return r;
    }
    return undefined;
}

/** Named states for previews: each finds its widget, so it works in every theme and size. */
export const sheetStates: { [name: string]: (this: void, ui: UIRoot) => void } = {
    default: () => {},
    tooltip: (ui) => {
        ui.layout();
        const t = findKind(ui.root, "tooltip");
        if (t !== undefined) ui.dispatch({ type: UIEventType.MouseMove, x: t.x + 4, y: t.y + 4 });
    },
    focus: (ui) => {
        ui.layout();
        const t = findKind(ui.root, "textField");
        if (t === undefined) return;
        ui.dispatch({ type: UIEventType.MouseDown, x: t.x + 4, y: t.y + 4 });
        ui.dispatch({ type: UIEventType.MouseUp, x: t.x + 4, y: t.y + 4 });
    },
    scrolled: (ui) => {
        ui.layout();
        const t = findKind(ui.root, "list") ?? findKind(ui.root, "scrollPane");
        if (t === undefined) return;
        ui.dispatch({ type: UIEventType.MouseMove, x: t.x + 20, y: t.y + 20 });
        ui.dispatch({ type: UIEventType.Wheel, x: t.x + 20, y: t.y + 20, delta: 20 });
    },
};
