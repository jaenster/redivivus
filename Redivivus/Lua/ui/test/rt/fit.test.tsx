// Text and art that must fit their box: ellipses at a glyph boundary, fitted fonts, button and tab
// art picked from the theme's fixed sizes (never stretched), labels cut to the art's room.

import { h, Fragment, colourCode, HeadlessMetrics, layoutText, testTheme, type Widget, ButtonVariant, Direction, FontRole, Justify, SizeMode } from "redivivus:ui";
import { ellipsize, truncate, type TextLine } from "../../../core/ui/text";
import { byId, eq, mountTest, test } from "./harness";

void Fragment;

const b = new HeadlessMetrics();
const font = b.loadFont("test");

function cut(text: string, width: number): string {
    const line = ellipsize(b, layoutText(b, font, text, 0, -1)[0], width);
    let s = "";
    for (let r = 0; r < line.runs.length; r++) s += line.runs[r].text;
    return `${line.width} ${s}`;
}

function drawn(w: Widget): string {
    const lines = w.state.lines as TextLine[];
    let s = "";
    for (let i = 0; i < lines.length; i++) for (let r = 0; r < lines[i].runs.length; r++) s += lines[i].runs[r].text;
    return s;
}

test("ellipsize: cut at a glyph with room for the ellipsis, spaces before it dropped", () => {
    // 6 pixels a glyph: "..." is 18.
    eq(cut("abcdefghij", 60), "60 abcdefghij");
    eq(cut("abcdefghij", 59), "54 abcdef...");
    eq(cut("abcdefghij", 40), "36 abc...");
    eq(cut("ab cdefgh", 40), "30 ab...");
    eq(cut("abcdefghij", 18), "18 ...");
    eq(cut("abcdefghij", 17), "0 ");
    eq(truncate(b, font, "Dependencies", 60), "Depende...");
    eq(truncate(b, font, "Back", 60), "Back");
});

test("ellipsize keeps the colour of the run it ends in", () => {
    const line = ellipsize(b, layoutText(b, font, `ab${colourCode(1)}cdefgh`, 0, -1)[0], 40);
    eq(line.runs.length, 2);
    eq([line.runs[0].text, line.runs[0].colour, line.runs[1].text, line.runs[1].colour], ["ab", 0, "c...", 1]);
});

test("a label that does not wrap is cut with an ellipsis, or not when asked", () => {
    const { ui } = mountTest(() => (
        <flow direction={Direction.Column} width={100}>
            <label text="A title far too long for its box" wrap={false} testId="cut" />
            <label text="A title far too long for its box" wrap={false} ellipsis={false} testId="whole" />
            <label text="Short" wrap={false} testId="short" />
        </flow>
    ));
    ui.layout();
    eq(drawn(byId(ui.root, "cut")), "A title far t...");
    eq(drawn(byId(ui.root, "whole")), "A title far too long for its box");
    eq(drawn(byId(ui.root, "short")), "Short");
});

test("fitFonts: the first smaller font the text fits in, then the ellipsis", () => {
    const theme = testTheme();
    const { ui } = mountTest(() => (
        <flow direction={Direction.Column} width={100}>
            <label font={FontRole.H1} fitFonts={[FontRole.Body]} text="abcdefghij" wrap={false} testId="fits" />
            <label font={FontRole.H1} fitFonts={[FontRole.Body]} text="abcdefghijklmnopq" wrap={false} testId="cut" />
            <label font={FontRole.H1} fitFonts={[FontRole.Body]} text="abcdefgh" wrap={false} testId="big" />
        </flow>
    ), 400, 300, theme);
    ui.layout();
    const fits = byId(ui.root, "fits");
    const big = byId(ui.root, "big");
    eq([drawn(fits), (fits.state.lines as TextLine[])[0].runs[0].font, fits.h], ["abcdefghij", theme.font(FontRole.Body), 11]);
    eq(drawn(byId(ui.root, "cut")), "abcdefghijklm...");
    eq([drawn(big), (big.state.lines as TextLine[])[0].runs[0].font], ["abcdefgh", theme.font(FontRole.H1)]);
});

test("a Small button takes the smallest art its label fits, at the art's own size", () => {
    const { ui } = mountTest(() => (
        <flow gap={0}>
            <button variant={ButtonVariant.Small} label="Install" testId="short" />
            <button variant={ButtonVariant.Small} label="A label much too long for the short art" testId="narrow" />
            <button variant={ButtonVariant.Cancel} label="Cancel" testId="cancel" />
        </flow>
    ));
    ui.layout();
    eq([byId(ui.root, "short").w, byId(ui.root, "short").h], [135, 25]);
    eq([byId(ui.root, "narrow").w, byId(ui.root, "narrow").h], [272, 25]);
    eq([byId(ui.root, "cancel").w, byId(ui.root, "cancel").h], [96, 32]);
});

test("tabs share one art: the smallest every label fits that fits the width, else cut labels", () => {
    const short = ["One", "Two", "Three"];
    const long = ["Description", "Changelog", "Dependencies"];
    const { ui, rec } = mountTest(() => (
        <flow direction={Direction.Column} width={500}>
            <tabs labels={short} testId="small" />
            <tabs labels={long} testId="wide" />
            <flow direction={Direction.Column} width={300}>
                <tabs labels={long} testId="tight" />
            </flow>
            <flow width={400}>
                <tabs labels={short} width={SizeMode.Fill} justify={Justify.End} testId="end" />
            </flow>
        </flow>
    ), 600, 400);
    ui.layout();
    // Cancel art 96x32 fits "Three" (30); "Dependencies" (72) needs the short art, 135x25.
    eq([byId(ui.root, "small").w, byId(ui.root, "small").h], [292, 32]);
    eq([byId(ui.root, "wide").w, byId(ui.root, "wide").h], [409, 25]);
    // 300 wide: three short tabs (409) do not fit; the cancel art does, with the labels cut to 60.
    eq([byId(ui.root, "tight").w, byId(ui.root, "tight").h], [292, 32]);
    ui.frame();
    const text = rec.text();
    eq([text.indexOf("Depende...") >= 0, text.indexOf("Descrip...") >= 0, text.indexOf("Changelog") >= 0], [true, true, true]);
    // Justified to the end of a 400-wide box: the row of 292 starts 108 in.
    const end = byId(ui.root, "end");
    eq(end.w, 400);
    const ones = text.split("\n").filter((l) => l.indexOf(" One") >= 0);
    // The test theme centres "One" (18) in a 96-wide tab: 39 in.
    eq(Number(ones[ones.length - 1].split(" ")[2]) - end.x, 108 + 39);
});
