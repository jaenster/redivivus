# Redivivus/Lua/ui

The UI kit for mods and for the engine's own screens. Two halves:

- **The runtime** (`../core/ui/`): signals, TSX, the widget tree, layout, text and input.
  TypeScript compiled into the game with the rest of the core and served as `redivivus:ui` (client
  state only, an instance per mod); the same sources run in Node.
- **The host renderer** (`host/`, Node only): the runtime's backend drawn into a paletted
  framebuffer with the game's own art, read straight from a 1.14d install, and written as PNG.
  It is how a screen is seen without the game, and what the snapshot tests use.

This directory is the package that builds and tests both, with the screens written on the kit.

## Layout

|Path|What|
|-|-|
|`../core/ui/signal.ts`|`signal`, `computed`, `effect`, `watch` + `frame()`, `batch`, `untrack`, owners and disposal|
|`../core/ui/jsx.ts`|`h`/`Fragment`, the `JSX` namespace (every intrinsic element and its props)|
|`../core/ui/control.ts`|`Show`, `For` (keyed), `Switch`/`Match`|
|`../core/ui/widget.ts`, `kinds.ts`|The widget tree; what each kind measures, paints and does with input|
|`../core/ui/layout.ts`|Integer layout: rows, columns, wrap, grid, stack, scroll, virtualised list, anchors|
|`../core/ui/text.ts`|Colour codes, missing glyphs, measuring, wrapping, drawing laid-out lines|
|`../core/ui/input.ts`, `draw.ts`, `root.ts`|Hit testing and event routing; the draw pass; `mount`|
|`../core/ui/backend.ts`|**The backend interface**: the one seam to whatever draws|
|`../core/ui/recorder.ts`, `headless.ts`|The headless backend: every call as a line of text; stand-in metrics|
|`../core/ui/theme.ts`, `themes/`|The theme interface; `stock` (the game's art) and `test` (built in, no game files)|
|`../core/ui/markdown.ts`, `md/`|Markdown READMEs as widgets; `md/` is the portal's parser, vendored (`md/PROVENANCE.md`)|
|`../core/redivivus/enums.ts`|The kit's enums, beside every other enum of the API|
|`../builtin/mods/`|The Mods screen, the built-in redivivus mod's; its data comes through `ModsData` (`model.ts`), which the game fills from `redivivus:mods` (`../builtin/data.ts`)|
|`screens/mods/`|The Mods screen as previews and tests see it (`ModsScreen.tsx`, over `fixtures.ts`; `GameMods.tsx`, over the example mods as the game showed them, to lay a preview over an in-game screenshot)|
|`screens/gallery/`|Three widget gallery sheets|
|`host/`|MPQ reader (`mpq.ts`, `explode.ts`), DC6, palette and blend tables, PNG, the raster backend, `ui-preview`|
|`test/rt/`|Runtime tests and the draw lists; compiled to Lua and run under LuaJIT too|
|`test/host/`|Node-only tests: MPQ, DC6, PNG, raster, golden images, determinism, Node against LuaJIT|
|`test/lua/run.lua`|The LuaJIT runner|
|`test/golden/`|Golden PNGs of the test theme, and `stock-hashes.json` (hashes only) for the stock theme|
|`tools/`|`build.ts` (both builds, mirroring `Redivivus/Lua` under `out/`), `test.ts` (everything), `ui-preview.ts`|

## Commands

```
npx pnpm@10 install                     # pinned: typescript 6.0.2, typescript-to-lua 1.37.1
node tools/test.ts [-v]                 # builds both, runs every test; luajit must be on PATH
UPDATE_GOLDEN=1 node tools/test.ts      # rewrites test-theme PNGs and stock hashes (read the diff)
node tools/build.ts [node|lua]          # out/node (ESM JavaScript), out/lua (LuaJIT)
node tools/ui-preview.ts screens/mods/ModsScreen.tsx --theme stock --size 800x600 --out out/mods.png [--state pending] [--drawlist out/mods.txt]
node tools/ui-preview.ts screens/gallery/Gallery.tsx#controls --theme test --out out/controls.png
```

The stock theme reads `Patch_D2.mpq`, `d2exp.mpq`, `d2data.mpq`, `d2char.mpq` from the 1.14d
install `D2_INSTALL` or `--install` names. Without one the stock tests are skipped and say so.
It draws under `palette\sky`, the palette the game's front-end screens (and the Mods screen) set,
so a stock preview is the game's frame pixel for pixel; under another palette the front-end art
shows colours the game never draws.

**No game art in git.** Stock-theme renders go to `out/golden-stock/` (ignored) and only their pixel
hashes are committed; `UPDATE_GOLDEN=1 node tools/test.ts` regenerates both. Test-theme goldens are
drawn only with the built-in palette, shapes and 5x7 font, and are committed as PNGs.

## Writing UI

```tsx
import { ButtonVariant, h, signal } from "redivivus:ui";

function Counter() {
    const [n, setN] = signal(0);
    return (
        <flow gap={6}>
            <button variant={ButtonVariant.Small} label="+1" onClick={() => setN(n() + 1)} />
            <label text={() => `clicked ${n()} times`} />
        </flow>
    );
}
```

- A component runs once. A prop given as a function is reactive; everything else is set once.
- Children may be widgets, text, arrays or functions (a function becomes a region that is rebuilt
  when what it reads changes). In Lua a child must not be `nil`: use `<Show>`, not `cond && <x/>`.
- `Show` hides on `undefined`, `null`, `false` only: `0` and `""` are shown, as they are true in Lua.
- `watch(() => engineValue)` is re-read once per `frame()` and propagates only on change.
- Props that pick from a set take the kit's enums, never strings: `SizeMode.Fill`, `Align.Center`,
  `BoxStyle.Inset`, `FontRole.Small`, `TextColour.Muted`.
- `redivivus:ui` resolves in every state, so a mod imports its UI from `main` directly. Outside the
  client state nothing is drawn and no input arrives; everything else works the same.
- Sizes: a number, `SizeMode.Content` (the default) or `SizeMode.Fill` (shared by `flex`); `min*`/`max*` clamp, and
  what a clamped fill could not take is shared again among the others. Rows centre their children
  on the cross axis by default, columns stretch them; art-sized widgets (button, checkbox, sprite,
  item slot, tabs, image) are never stretched unless they ask for `SizeMode.Fill`.

## The engine backend

`../engine/d2_lua_menu.cpp` implements `../core/ui/backend.ts` in C++ on D2gfx and D2Win, for the
main menu's screens: its own scissor stack (D2gfx clips per call only), `BlendMode` to DRAWMODE
(Trans25 -> 0 VERYTRANSPARENT, Trans50 -> 1, Trans75 -> 2, Additive -> 3, AlphaBlack -> 4,
Normal -> 5), colours to the nearest palette index (cached, cleared on every palette load), and
`drawText`'s line top to the engine's glyph-bottom y (`y + ascent - 1`). `host/raster.ts` follows
the same rules, so a preview is no better than the game: the engine's nearest-colour metric, the
software renderer's four fill alpha levels and its undrawn last column and row, no alpha on lines,
and a glyph cut by a scissor drawn clipped only in colour 0 (the clipped draw takes no palette
shift) and dropped otherwise.

The stock theme draws button labels as the engine does (D2WinButton: FontExocet10, colour 0), which
is dark lettering on the stone; no text colour of the palette makes those glyphs lighter.
