return { patch = function(engine)
    engine.TestShared.Hot.override(function(next, a) return next(a) + 1 end)
end }
