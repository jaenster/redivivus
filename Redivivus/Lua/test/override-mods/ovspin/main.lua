return { patch = function(engine)
    engine.TestShared.Slow.override(function(next, a) while true do end end)
    engine.TestShared.Yields.override(function(next, a) coroutine.yield(1) return a end)
end }
