return { patch = function(engine)
    engine.TestShared.Fail.override(function(next, a) if a == 13 then error("unlucky") end return next(a) + 5 end)
    engine.TestShared.Echo.override(function(next, p) return "not a pointer" end)
end }
