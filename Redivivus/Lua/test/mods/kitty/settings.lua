-- The data stage declares the settings, so the host can list them at boot.
local settings = require("settingsdef")
require("redivivus:shared").log("data stage settings:", settings.boost, settings.rate, settings.motto)
