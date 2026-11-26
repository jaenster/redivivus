import { colourCode, HeadlessMetrics, layoutText, measure, parseRuns, plain } from "redivivus:ui";
import { eq, test } from "./harness";

const b = new HeadlessMetrics();
const font = b.loadFont("test");

function lines(text: string, width: number): string[] {
    const out: string[] = [];
    const ls = layoutText(b, font, text, 0, width);
    for (let i = 0; i < ls.length; i++) {
        let s = "";
        for (let r = 0; r < ls[i].runs.length; r++) s += (r > 0 ? "|" : "") + `${ls[i].runs[r].colour}:${ls[i].runs[r].text}`;
        out.push(`${ls[i].width} ${s}`);
    }
    return out;
}

test("colour codes split runs and carry over a line end", () => {
    const runs = parseRuns(`a${colourCode(1)}red${colourCode(4)}gold\nnext`, 0);
    eq(runs.length, 2);
    eq(runs[0], [
        { text: "a", colour: 0 },
        { text: "red", colour: 1 },
        { text: "gold", colour: 4 },
    ]);
    eq(runs[1], [{ text: "next", colour: 4 }]);
    eq(plain(`x${colourCode(2)}y`), "xy");
});

test("a colour code at the very end, or a lone escape byte, is handled", () => {
    eq(parseRuns(`ab${colourCode(3)}`, 0)[0], [{ text: "ab", colour: 0 }]);
    const lone = String.fromCharCode(255);
    eq(parseRuns(`a${lone}`, 0)[0], [{ text: `a${lone}`, colour: 0 }]);
});

test("measure ignores colour codes and takes the widest line", () => {
    eq(measure(b, font, `${colourCode(1)}abc`), 18);
    eq(measure(b, font, "ab\nabcd"), 24);
});

test("wrap: words, dropped spaces at the break, kept leading spaces", () => {
    eq(lines("the quick brown fox", 60), ["54 0:the quick", "54 0:brown fox"]);
    eq(lines("  indented text", 200), ["90 0:  indented text"]);
    eq(lines("a  b", 200), ["24 0:a  b"], "inner runs of spaces kept");
});

test("wrap: a word longer than the line breaks between characters", () => {
    eq(lines("abcdefghij", 24), ["24 0:abcd", "24 0:efgh", "12 0:ij"]);
    eq(lines("xy abcdefghij", 30), ["12 0:xy", "30 0:abcde", "30 0:fghij"]);
});

test("wrap keeps colours across the break", () => {
    eq(lines(`aaa ${colourCode(1)}bbb ccc`, 42), ["42 0:aaa |1:bbb", "18 1:ccc"]);
});

test("missing glyphs become the placeholder", () => {
    const t = `a${String.fromCharCode(200)}b`;
    eq(lines(t, -1), ["18 0:a?b"]);
});

test("no wrap with width -1; hard line breaks still split", () => {
    eq(lines("one two three", -1), ["78 0:one two three"]);
    eq(lines("one\n\nthree", -1), ["18 0:one", "0 ", "30 0:three"]);
});
