local stash
return { patch = function(engine)
    engine.TestShared.Echo.override(function(next, p) if stash == nil then stash = p end return next(p) end)
    engine.TestShared.Other.override(function(next, p, q) return next(stash, q) end)
end }
