import { batch, computed, createRoot, effect, frame, onCleanup, signal, stats, untrack, watch, watchCount } from "redivivus:ui";
import { eq, ok, test } from "./harness";

test("signal: read, write, effect re-runs on change only", () => {
    const [n, setN] = signal(1);
    const seen: number[] = [];
    const dispose = createRoot((d) => {
        effect(() => {
            seen.push(n());
        });
        return d;
    });
    setN(2);
    setN(2);
    setN(3);
    eq(seen, [1, 2, 3]);
    dispose();
    setN(4);
    eq(seen, [1, 2, 3], "after dispose");
});

test("computed: lazy and cached", () => {
    const [n, setN] = signal(2);
    let runs = 0;
    const sq = computed(() => {
        runs++;
        return n() * n();
    });
    eq(runs, 0, "not run before read");
    eq(sq(), 4);
    eq(sq(), 4);
    eq(runs, 1, "cached");
    setN(3);
    eq(runs, 1, "lazy after write");
    eq(sq(), 9);
    eq(runs, 2);
});

test("glitch-free: a diamond runs its effect once with consistent values", () => {
    const [a, setA] = signal(1);
    const b = computed(() => a() * 2);
    const c = computed(() => a() + 10);
    const seen: string[] = [];
    const dispose = createRoot((d) => {
        effect(() => {
            seen.push(`${a()}:${b()}:${c()}`);
        });
        return d;
    });
    setA(2);
    setA(5);
    eq(seen, ["1:2:11", "2:4:12", "5:10:15"]);
    dispose();
});

test("equality cut-off: an unchanged computed stops propagation", () => {
    const [a, setA] = signal(2);
    const parity = computed(() => a() % 2);
    let runs = 0;
    const dispose = createRoot((d) => {
        effect(() => {
            parity();
            runs++;
        });
        return d;
    });
    setA(4);
    setA(6);
    eq(runs, 1, "parity unchanged");
    setA(7);
    eq(runs, 2);
    dispose();
});

test("batch: several writes, one effect run", () => {
    const [a, setA] = signal(0);
    const [b, setB] = signal(0);
    const seen: number[] = [];
    const dispose = createRoot((d) => {
        effect(() => {
            seen.push(a() + b());
        });
        return d;
    });
    batch(() => {
        setA(1);
        setB(2);
        setA(3);
    });
    eq(seen, [0, 5]);
    dispose();
});

test("untrack: a read that does not subscribe", () => {
    const [a, setA] = signal(1);
    const [b, setB] = signal(1);
    let runs = 0;
    const dispose = createRoot((d) => {
        effect(() => {
            a();
            untrack(b);
            runs++;
        });
        return d;
    });
    setB(2);
    eq(runs, 1);
    setA(2);
    eq(runs, 2);
    dispose();
});

test("ownership: a re-run disposes what the last run created; dispose cleans everything", () => {
    const before = stats.computations;
    const beforeOwners = stats.owners;
    const [flag, setFlag] = signal(true);
    const log: string[] = [];
    const dispose = createRoot((d) => {
        effect(() => {
            const f = flag();
            effect(() => {
                log.push(`inner ${f}`);
            });
            onCleanup(() => log.push(`cleanup ${f}`));
        });
        return d;
    });
    setFlag(false);
    eq(log, ["inner true", "cleanup true", "inner false"]);
    ok(stats.computations > before, "computations alive");
    dispose();
    eq(log, ["inner true", "cleanup true", "inner false", "cleanup false"]);
    eq(stats.computations, before, "computations after dispose");
    eq(stats.owners, beforeOwners, "owners after dispose");
});

test("watch: polled once per frame, propagates only on change, disposed with its owner", () => {
    const engine = { life: 100 };
    const before = watchCount();
    const seen: number[] = [];
    const dispose = createRoot((d) => {
        const life = watch(() => engine.life);
        effect(() => {
            seen.push(life());
        });
        return d;
    });
    frame();
    engine.life = 90;
    eq(seen, [100], "no change until the frame");
    frame();
    frame();
    engine.life = 80;
    frame();
    eq(seen, [100, 90, 80]);
    eq(watchCount(), before + 1);
    dispose();
    eq(watchCount(), before, "watch gone");
    engine.life = 1;
    frame();
    eq(seen, [100, 90, 80]);
});

test("effect writing another signal settles in one flush", () => {
    const [a, setA] = signal(1);
    const [b, setB] = signal(0);
    const seen: number[] = [];
    const dispose = createRoot((d) => {
        effect(() => setB(a() * 10));
        effect(() => {
            seen.push(b());
        });
        return d;
    });
    setA(2);
    eq(seen, [10, 20]);
    dispose();
});
