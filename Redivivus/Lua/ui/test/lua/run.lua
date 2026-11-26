-- Runs the compiled kit's tests under LuaJIT: luajit test/lua/run.lua OUT_LUA_DIR [tests|drawlists]
-- "tests" prints one line per test and exits 1 on a failure; "drawlists" prints every draw list
-- between marker lines for tools/test.ts to compare with Node's.

local dir, mode = arg[1], arg[2] or "tests"
package.path = dir .. "/?.lua;" .. package.path
local main = require("ui.test.rt.main")

if mode == "drawlists" then
  local lists = main.drawLists()
  for i = 1, #lists do
    io.write("@@@ ", lists[i].name, "\n", lists[i].text, "\n")
  end
  io.write("@@@ end\n")
  os.exit(0)
end

local report = main.runAll()
for i = 1, #report.lines do print(report.lines[i]) end
print(string.format("luajit: %d passed, %d failed", report.passed, report.failed))
os.exit(report.failed == 0 and 0 or 1)
