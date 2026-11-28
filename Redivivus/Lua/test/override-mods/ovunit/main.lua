local shared = require("redivivus:shared")
return { patch = function(engine)
    engine.TestServer.Pick.override(function(next, a, b, which)
        shared.log("pick", type(a), a and a.valid, b and b.valid, which)
        return next(b, a, which)
    end)
end }
