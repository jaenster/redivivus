// The mod entry and the engine: defineMod takes only entry points, each parameter is typed, the
// engine of a side holds that side's functions, and overrides are typed by the function.
import { defineMod } from "redivivus:mod";
import { Side } from "redivivus:enums";

export const ok = defineMod({
    patch(engine) {
        if (engine.side === Side.Server) {
            engine.D2Common.Skills.SKILLS_CalculateManaCost.override((next, skill, level) => next(skill, level) + 1);
            engine.D2Game.GAME.Server.ServerGameLoop.override((next, game) => next(game));
            // @ts-expect-error a server state's engine has no client functions
            engine.D2Client;
            // @ts-expect-error an override returns what the function returns
            engine.D2Common.Skills.SKILLS_CalculateManaCost.override((next, skill, level) => "cheap");
            // @ts-expect-error next takes the function's own parameters
            engine.D2Common.Skills.SKILLS_CalculateManaCost.override((next, skill, level) => next("x", level));
            // @ts-expect-error a refused function has no override
            engine.Storm.Source.SSTR.SStrPrintf.override(() => 0);
        } else {
            // @ts-expect-error the client's engine has no server functions
            engine.D2Game;
        }
        // @ts-expect-error the raw tier may be absent
        engine.unsafe.ffi;
    },
    server(game) {
        void game.id;
    },
});

// @ts-expect-error a key that is no entry point
export const typo = defineMod({ pach(engine: unknown) {} });
