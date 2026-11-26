// The UI kit's TSX, checked against its declarations. Nothing imports this file from main:
// redivivus:ui resolves only in the client state.
import { Align, BoxStyle, ButtonVariant, Direction, FontRole, h, signal, SizeMode, TextColour, type Child } from "redivivus:ui";

export function good(): Child {
    const [n, setN] = signal(0);
    return (
        <frame style={BoxStyle.Panel} direction={Direction.Column} width={SizeMode.Fill} align={Align.Start}>
            <label text={() => `${n()}`} font={FontRole.Small} colour={TextColour.Muted} />
            <button variant={ButtonVariant.Small} label="+" onClick={() => setN(n() + 1)} />
            <checkbox checked={true} onChange={(on) => setN(on ? 1 : 0)} />
        </frame>
    );
}

export function bad(): Child[] {
    return [
        // @ts-expect-error a variant is a ButtonVariant, not a string
        <button variant="small" />,
        // @ts-expect-error a direction is a Direction, not a string
        <frame direction="row" />,
        // @ts-expect-error a width is pixels or a SizeMode, not a string
        <frame width="fill" />,
        // @ts-expect-error a colour is a TextColour or a colour code, not a string
        <label colour="muted" />,
        // @ts-expect-error an unknown prop
        <label txt="x" />,
        // @ts-expect-error an unknown element
        <panel />,
        // @ts-expect-error text is a string or an accessor of one
        <label text={5} />,
        // @ts-expect-error a checkbox's onChange takes a boolean
        <checkbox onChange={(v: string) => void v} />,
    ];
}
