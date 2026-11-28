local shared = require("redivivus:shared")
return { patch = function(engine)
    shared.log("raw tier", engine.unsafe ~= nil)
    engine.unsafe.ffi.cdef("struct FixtureThing { int nValue; int aData[4]; };")
    engine.unsafe.TestShared.Read.override(function(next, p, k)
        shared.log("raw arg", type(p), p.nValue, p.aData[2])
        p.nValue = p.nValue + 1
        return next(p, k) + p.aData[0]
    end)
    engine.unsafe.TestShared.Echo.override(function(next, p) return next(p) end)
end }
