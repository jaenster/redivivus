-- The adversarial suite's override mod: it keeps everything its patch and its overrides are handed,
-- for the host test to walk as the mod could.
return { patch = function(engine)
    held = { engine = engine, ns = engine.TestShared, fn = engine.TestShared.Echo, other = engine.TestShared.Other }
    engine.TestShared.Echo.override(function(next, p)
        held.next = next
        held.p = p
        held.args = { p }
        return next(p)
    end)
    engine.TestShared.Other.override(function(next, p, q)
        -- a pointer of another call is refused, so is a number, a table, a string
        held.r1 = { pcall(next, held.p, q) }
        held.r2 = { pcall(next, 12345, q) }
        held.r3 = { pcall(next, {}, q) }
        held.r4 = { pcall(next, "x", q) }
        return next(p, q)
    end)
end }
