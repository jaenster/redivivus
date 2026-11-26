// A counter panel. Until the game draws mod UI it is drawn on the kit's recorder, which writes
// every draw call down, and clicked through the same input path the engine will use.
import { log } from "redivivus:shared";
import { Align, BoxStyle, ButtonVariant, Direction, h, HeadlessMetrics, mount, Recorder, signal, SizeMode, testTheme, UIEventType, type Child, type Widget } from "redivivus:ui";

function Counter(props: { name: string; plus: (this: void, w: Widget) => void; count: (this: void, w: Widget) => void }) {
    const [clicks, setClicks] = signal(0);
    return (
        <frame style={BoxStyle.Panel} direction={Direction.Column} padding={8} gap={6}>
            <label text={props.name} />
            <label text={() => `clicked ${clicks()} times`} ref={props.count} />
            <button variant={ButtonVariant.Small} label="+1" onClick={() => setClicks(clicks() + 1)} ref={props.plus} />
        </frame>
    );
}

export function open(side: string, gameId: number) {
    const recorder = new Recorder(new HeadlessMetrics());
    let plus: Widget | undefined;
    let count: Widget | undefined;
    const ui = mount(() => <Counter name="ui-mod" plus={(w) => (plus = w)} count={(w) => (count = w)} />, recorder, testTheme(), 320, 200);
    ui.draw();
    if (plus === undefined || count === undefined) throw new Error("the panel's widgets were not made");
    for (let i = 0; i < 2; i++) {
        ui.dispatch({ type: UIEventType.MouseDown, x: plus.x + 2, y: plus.y + 2 });
        ui.dispatch({ type: UIEventType.MouseUp, x: plus.x + 2, y: plus.y + 2 });
    }
    ui.draw();
    log(`ui-mod ${side} panel in game ${gameId}: ${count.props.text as string}, ${recorder.lines.length > 10 ? "drawn" : "not drawn"}`);
}

/** The counter as a main menu screen: the game draws it, and Back (or Escape) returns to the menu. */
export function CounterScreen(props: { close: (this: void) => void }): Child {
    const [clicks, setClicks] = signal(0);
    return (
        <frame style={BoxStyle.Screen} width={SizeMode.Fill} height={SizeMode.Fill}>
            <frame style={BoxStyle.Dialog} anchor={{ left: 200, top: 150, right: 200, bottom: 150 }} padding={16} gap={12} direction={Direction.Column} align={Align.Center}>
                <label text="ui-mod" />
                <label text={() => `clicked ${clicks()} times`} />
                <button variant={ButtonVariant.Medium} label="+1" onClick={() => setClicks(clicks() + 1)} />
                <button variant={ButtonVariant.Medium} label="Back" onClick={() => props.close()} />
            </frame>
        </frame>
    );
}
