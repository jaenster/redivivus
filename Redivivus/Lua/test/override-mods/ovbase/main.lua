local shared = require("redivivus:shared")
return { patch = function(engine)
    engine.TestShared.Scale.override(function(next, a) shared.log("base in", a) local r = next(a) + 1 shared.log("base out", r) return r end)
    engine.TestShared.Chained.override(function(next, a) return next(a) + 1 end)
end }
