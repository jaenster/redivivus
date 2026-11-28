local shared = require("redivivus:shared")
return { patch = function(engine)
    shared.log("client engine", engine.side, engine.TestServer == nil, engine.TestClient ~= nil)
    engine.TestClient.ClientOnly.override(function(next, a) return next(a) + 2 end)
    engine.TestShared.Add.override(function(next, a, b) return next(a, b) + 1000 end)
end }
