-- Actions for scripts, written in plain Lua. An action is a function that may wait: it yields
-- the coroutine of the script that calls it, so it only works inside a script. From
-- TypeScript an action is called with `yield* wait_ticks(n)` (see actions.d.ts).
local actions = {}

-- Wait n ticks; the outcome is the tick the script resumes on.
function actions.wait_ticks(n)
  return script.wait(n)
end

return actions
