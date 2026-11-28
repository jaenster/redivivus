return { patch = function(engine)
    engine.TestShared.Plain.override(function(next, a) return next(a) + 1 end)
    error("patch fails after one override")
end }
