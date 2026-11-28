local shared = require("redivivus:shared")
return { patch = function(engine)
    engine.TestShared.Scale.override(function(next, a) shared.log("top in", a) local r = next(a) * 10 shared.log("top out", r) return r end)
    engine.TestShared.Chained.override(function(next, a) return next(a + 1) * 2 end)
end }
