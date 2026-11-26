import { h, Fragment, signal, type Widget, Align, Direction, Justify, SizeMode, UIEventType } from "redivivus:ui";
import { box, byId, eq, mountTest, test } from "./harness";

void Fragment;

test("row: fixed, content, fill and weighted fill share the space by floor plus remainder", () => {
    const { ui } = mountTest(() => (
        <flow width={203} height={20} gap={5} testId="row">
            <frame width={10} height={SizeMode.Fill} testId="a" />
            <label text="abc" testId="b" />
            <frame width={SizeMode.Fill} height={SizeMode.Fill} testId="c" />
            <frame width={SizeMode.Fill} flex={2} height={SizeMode.Fill} testId="d" />
        </flow>
    ));
    ui.layout();
    const r = ui.root;
    // free = 203 - 10 - 18 - 15 = 160, shares 53 + 106 = 159, the remainder pixel to the first fill.
    eq(box(byId(r, "a")), "0 0 10 20");
    eq(box(byId(r, "b")), "15 4 18 11", "content, centred on the cross axis");
    eq(box(byId(r, "c")), "38 0 54 20");
    eq(box(byId(r, "d")), "97 0 106 20");
});

test("column: padding, stretch by default, min and max clamp", () => {
    const { ui } = mountTest(() => (
        <flow direction={Direction.Column} width={100} height={100} padding={[1, 2, 3, 4]} gap={2}>
            <frame height={10} testId="a" />
            <frame height={SizeMode.Fill} maxHeight={30} testId="b" />
            <frame height={10} width={20} alignSelf={Align.End} testId="c" />
            <frame height={SizeMode.Fill} minHeight={5} testId="d" />
        </flow>
    ));
    ui.layout();
    eq(box(byId(ui.root, "a")), "4 1 94 10");
    eq(box(byId(ui.root, "b")), "4 13 94 30", "fill clamped to max");
    eq(box(byId(ui.root, "c")), "78 45 20 10");
    eq(box(byId(ui.root, "d")), "4 57 94 40", "what b could not take goes to d");
});

test("justify: center, end and between", () => {
    const { ui } = mountTest(() => (
        <flow direction={Direction.Column}>
            <flow width={100} height={10} justify={Justify.Center}>
                <frame width={20} testId="c" />
            </flow>
            <flow width={100} height={10} justify={Justify.End}>
                <frame width={20} testId="e" />
            </flow>
            <flow width={100} height={10} justify={Justify.Between}>
                <frame width={20} testId="b1" />
                <frame width={20} testId="b2" />
                <frame width={20} testId="b3" />
            </flow>
        </flow>
    ));
    ui.layout();
    eq(byId(ui.root, "c").x, 40);
    eq(byId(ui.root, "e").x, 80);
    eq([byId(ui.root, "b1").x, byId(ui.root, "b2").x, byId(ui.root, "b3").x], [0, 40, 80]);
});

test("anchors: edges, stretch between two edges, centre", () => {
    const { ui } = mountTest(() => (
        <frame width={200} height={100} padding={10}>
            <frame width={20} height={10} anchor={{ right: 5, bottom: 5 }} testId="rb" />
            <frame height={10} anchor={{ left: 5, right: 5, top: 0 }} testId="stretch" />
            <frame width={20} height={10} anchor={{ center: true }} testId="c" />
        </frame>
    ));
    ui.layout();
    eq(box(byId(ui.root, "rb")), "165 75 20 10");
    eq(box(byId(ui.root, "stretch")), "15 10 170 10");
    eq(box(byId(ui.root, "c")), "90 45 20 10");
});

test("wrap: children flow onto new lines", () => {
    const { ui } = mountTest(() => (
        <flow wrap width={50} gap={2} rowGap={3} align={Align.Start}>
            <frame width={20} height={5} testId="a" />
            <frame width={20} height={8} testId="b" />
            <frame width={20} height={5} testId="c" />
        </flow>
    ));
    ui.layout();
    eq(box(byId(ui.root, "a")), "0 0 20 5");
    eq(box(byId(ui.root, "b")), "22 0 20 8");
    eq(box(byId(ui.root, "c")), "0 11 20 5");
});

test("grid: equal columns, and content plus fill columns", () => {
    const { ui } = mountTest(() => (
        <flow direction={Direction.Column} width={100}>
            <grid columns={3} gap={2} testId="g">
                <frame height={10} testId="g1" />
                <frame height={12} testId="g2" />
                <frame height={10} testId="g3" />
                <frame height={10} testId="g4" />
            </grid>
            <grid columnWidths={[SizeMode.Content, SizeMode.Fill]} gap={4}>
                <label text="ab" testId="k" />
                <label text="v" testId="v" />
            </grid>
        </flow>
    ));
    ui.layout();
    eq(box(byId(ui.root, "g1")), "0 0 32 12", "stretched to the row");
    eq(box(byId(ui.root, "g2")), "34 0 32 12");
    eq(box(byId(ui.root, "g3")), "68 0 32 12");
    eq(box(byId(ui.root, "g4")), "0 14 32 10");
    eq(box(byId(ui.root, "k")), "0 24 12 11");
    eq(box(byId(ui.root, "v")), "16 24 84 11");
});

test("resolution independence: the same tree at 640x480 and 1024x768", () => {
    const view = () => (
        <frame width={SizeMode.Fill} height={SizeMode.Fill} padding={8}>
            <frame anchor={{ left: 0, right: 0, bottom: 0 }} height={30} testId="bar" />
            <frame width={100} height={50} anchor={{ center: true }} testId="mid" />
        </frame>
    );
    const a = mountTest(view, 640, 480);
    a.ui.layout();
    eq(box(byId(a.ui.root, "bar")), "8 442 624 30");
    eq(box(byId(a.ui.root, "mid")), "270 215 100 50");
    const b = mountTest(view, 1024, 768);
    b.ui.layout();
    eq(box(byId(b.ui.root, "bar")), "8 730 1008 30");
    eq(box(byId(b.ui.root, "mid")), "462 359 100 50");
    a.ui.resize(800, 600);
    a.ui.layout();
    eq(box(byId(a.ui.root, "bar")), "8 562 784 30", "after resize");
});

test("reactive props re-lay out", () => {
    const [w, setW] = signal(40);
    const { ui } = mountTest(() => (
        <flow width={200} height={10}>
            <frame width={w} testId="a" />
            <frame width={SizeMode.Fill} testId="b" />
        </flow>
    ));
    ui.layout();
    eq(byId(ui.root, "b").x, 40);
    setW(70);
    ui.layout();
    eq(byId(ui.root, "b").x, 70);
    eq(byId(ui.root, "b").w, 130);
});

test("scroll pane: offset clamps to the content, and its children are clipped", () => {
    const { ui, rec } = mountTest(() => (
        <scrollPane width={100} height={50} testId="sp">
            <frame height={40} testId="a" />
            <frame height={40} testId="b" />
        </scrollPane>
    ));
    ui.layout();
    const sp = byId(ui.root, "sp");
    eq(sp.contentH, 80);
    eq(byId(ui.root, "a").w, 88, "room for the scrollbar");
    ui.dispatch({ type: UIEventType.Wheel, x: 10, y: 10, delta: 10 });
    ui.layout();
    eq(sp.scrollY, 30, "clamped to content - view");
    eq(byId(ui.root, "b").y, 10);
    ui.draw();
    eq(rec.lines.indexOf("clip 0 0 100 50") >= 0, true, "scissor pushed");
});

test("z order: higher z draws later and is hit first", () => {
    const { ui, rec } = mountTest(() => (
        <frame width={100} height={100}>
            <frame anchor={{ left: 0, top: 0 }} width={50} height={50} bg={0x111111} z={2} testId="top" />
            <frame anchor={{ left: 0, top: 0 }} width={50} height={50} bg={0x222222} testId="under" />
        </frame>
    ));
    ui.draw();
    const i1 = rec.lines.indexOf("rect 0 0 50 50 222222 255");
    const i2 = rec.lines.indexOf("rect 0 0 50 50 111111 255");
    eq(i1 >= 0 && i2 > i1, true, "z=2 after z=0");
    let hit: Widget | undefined = undefined;
    byId(ui.root, "top").props.onMouseDown = (e: { target: Widget | undefined }) => {
        hit = e.target;
    };
    ui.dispatch({ type: UIEventType.MouseDown, x: 5, y: 5 });
    eq(hit === byId(ui.root, "top"), true);
});
