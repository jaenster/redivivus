local chat = require("shared.chat")
chat.on.said(function(from, text) log("client said", from, text) end)
chat.on.tags(function(list, flag, n) log("client tags", #list, list[1], list[3], flag, n) end)
