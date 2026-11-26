local data = require("redivivus:data").data
local log = require("redivivus:shared").log
data.extend(require("prototypes.items"))
log("alpha data stage, helper doubles", require("helper").double(4))
