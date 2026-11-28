local shared = require("redivivus:shared")
return { patch = function(engine)
    shared.log("no raw tier", engine.unsafe == nil, rawget(engine, "unsafe"), pcall(function() engine.side = 0 end))
    local found = {}
    for k in pairs(engine) do found[#found + 1] = k end
    shared.log("pairs", #found, getmetatable(engine))
    engine.TestShared.Echo.override(function(next, p)
        local t = type(p)
        local ok1 = pcall(function() return p.nValue end)
        local ok2 = pcall(function() return p[0] end)
        shared.log("opaque", t, ok1, ok2, ffi, jit)
        return next(p)
    end)
end }
