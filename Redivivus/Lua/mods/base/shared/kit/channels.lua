-- Generated from Redivivus/Lua/kit/shared/channels.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
local ____lualib = require("__base__/shared/lualib_bundle")
local __TS__Class = ____lualib.__TS__Class
local __TS__New = ____lualib.__TS__New
local __TS__ObjectKeys = ____lualib.__TS__ObjectKeys
local __TS__ArraySort = ____lualib.__TS__ArraySort
local __TS__ArrayIndexOf = ____lualib.__TS__ArrayIndexOf
local __TS__InstanceOf = ____lualib.__TS__InstanceOf
local __TS__ArrayMap = ____lualib.__TS__ArrayMap
local __TS__ArrayForEach = ____lualib.__TS__ArrayForEach
local ____exports = {}
local DecodeError
local INT_LIMIT = 2 ^ 52
local TEXT_LIMIT = 4096
--- The argument types.
____exports.t = {
    int = function(min, max)
        if min == nil then
            min = -2 ^ 31
        end
        if max == nil then
            max = 2 ^ 31 - 1
        end
        if not (min >= -INT_LIMIT and max <= INT_LIMIT and min <= max) then
            error("t.int: the range must lie within +-2^52, min <= max", 2)
        end
        return {kind = 1, min = min, max = max}
    end,
    number = function(min, max)
        if min == nil then
            min = -math.huge
        end
        if max == nil then
            max = math.huge
        end
        return {kind = 2, min = min, max = max}
    end,
    bool = function()
        return {kind = 3}
    end,
    text = function(maxLength)
        if not (maxLength >= 0 and maxLength <= TEXT_LIMIT) then
            error(
                "t.text: a maximum length from 0 to " .. tostring(TEXT_LIMIT),
                2
            )
        end
        return {kind = 4, max = maxLength}
    end,
    oneOf = function(values)
        if #values == 0 then
            error("t.oneOf: at least one value", 2)
        end
        return {kind = 5, values = values}
    end,
    list = function(item, maxLength)
        return {kind = 6, item = item, max = maxLength}
    end,
    record = function(fields)
        return {kind = 7, fields = fields}
    end,
    optional = function(inner)
        return {kind = 8, item = inner}
    end
}
local Writer = __TS__Class()
Writer.name = "Writer"
function Writer.prototype.____constructor(self)
    self.parts = {}
end
function Writer.prototype.byte(self, b)
    local ____self_parts_0 = self.parts
    ____self_parts_0[#____self_parts_0 + 1] = string.char(b)
end
function Writer.prototype.varint(self, n)
    while n >= 128 do
        self:byte(n % 128 + 128)
        n = math.floor(n / 128)
    end
    self:byte(n)
end
function Writer.prototype.bytes(self, s)
    self:varint(#s)
    local ____self_parts_1 = self.parts
    ____self_parts_1[#____self_parts_1 + 1] = s
end
local Reader = __TS__Class()
Reader.name = "Reader"
function Reader.prototype.____constructor(self, data)
    self.data = data
    self.pos = 1
end
function Reader.prototype.fail(self, why)
    error(
        __TS__New(DecodeError, why),
        0
    )
end
function Reader.prototype.byte(self)
    if self.pos > #self.data then
        self:fail("the message ends early")
    end
    local b = string.byte(self.data, self.pos)
    self.pos = self.pos + 1
    return b
end
function Reader.prototype.varint(self)
    local n = 0
    local scale = 1
    do
        while true do
            local b = self:byte()
            n = n + b % 128 * scale
            if b < 128 then
                return n
            end
            scale = scale * 128
            if scale > 2 ^ 56 then
                self:fail("a number is too long")
            end
        end
    end
end
function Reader.prototype.bytes(self, max)
    local length = self:varint()
    if length > max then
        self:fail(((("a string of " .. tostring(length)) .. " bytes, at most ") .. tostring(max)) .. " allowed")
    end
    if self.pos + length - 1 > #self.data then
        self:fail("the message ends early")
    end
    local s = string.sub(self.data, self.pos, self.pos + length - 1)
    self.pos = self.pos + length
    return s
end
DecodeError = __TS__Class()
DecodeError.name = "DecodeError"
function DecodeError.prototype.____constructor(self, why)
    self.why = why
end
local function sortedKeys(o)
    return __TS__ArraySort(__TS__ObjectKeys(o))
end
local function encodeValue(w, s, v, path)
    repeat
        local ____switch33 = s.kind
        local ____cond33 = ____switch33 == 1
        if ____cond33 then
            do
                if type(v) ~= "number" or v ~= math.floor(v) or v < s.min or v > s.max then
                    error(
                        (((((path .. ": a whole number from ") .. tostring(s.min)) .. " to ") .. tostring(s.max)) .. " expected, got ") .. tostring(v),
                        0
                    )
                end
                w:varint(v >= 0 and v * 2 or -v * 2 - 1)
                return
            end
        end
        ____cond33 = ____cond33 or ____switch33 == 2
        if ____cond33 then
            do
                if type(v) ~= "number" or v ~= v or v == math.huge or v == -math.huge or v < s.min or v > s.max then
                    error(
                        (path .. ": a finite number in range expected, got ") .. tostring(v),
                        0
                    )
                end
                w:bytes(string.format("%.17g", v))
                return
            end
        end
        ____cond33 = ____cond33 or ____switch33 == 3
        if ____cond33 then
            if type(v) ~= "boolean" then
                error(
                    (path .. ": a boolean expected, got ") .. type(v),
                    0
                )
            end
            w:byte(v and 1 or 0)
            return
        end
        ____cond33 = ____cond33 or ____switch33 == 4
        if ____cond33 then
            if type(v) ~= "string" or #v > s.max then
                error(
                    ((path .. ": a string of at most ") .. tostring(s.max)) .. " bytes expected",
                    0
                )
            end
            w:bytes(v)
            return
        end
        ____cond33 = ____cond33 or ____switch33 == 5
        if ____cond33 then
            do
                local index = __TS__ArrayIndexOf(s.values, v)
                if index < 0 then
                    error(
                        ((path .. ": ") .. tostring(v)) .. " is not one of the choices",
                        0
                    )
                end
                w:varint(index)
                return
            end
        end
        ____cond33 = ____cond33 or ____switch33 == 6
        if ____cond33 then
            do
                if type(v) ~= "table" then
                    error(
                        (path .. ": a list expected, got ") .. type(v),
                        0
                    )
                end
                local list = v
                if #list > s.max then
                    error(
                        ((path .. ": at most ") .. tostring(s.max)) .. " items",
                        0
                    )
                end
                w:varint(#list)
                do
                    local i = 0
                    while i < #list do
                        encodeValue(
                            w,
                            s.item,
                            list[i + 1],
                            ((path .. "[") .. tostring(i + 1)) .. "]"
                        )
                        i = i + 1
                    end
                end
                return
            end
        end
        ____cond33 = ____cond33 or ____switch33 == 7
        if ____cond33 then
            do
                if type(v) ~= "table" then
                    error(
                        (path .. ": a table expected, got ") .. type(v),
                        0
                    )
                end
                local record = v
                for ____, key in ipairs(sortedKeys(record)) do
                    if not (s.fields[key] ~= nil) then
                        error(((path .. ": no field '") .. key) .. "' is declared", 0)
                    end
                end
                for ____, key in ipairs(sortedKeys(s.fields)) do
                    encodeValue(w, s.fields[key], record[key], (path .. ".") .. key)
                end
                return
            end
        end
        ____cond33 = ____cond33 or ____switch33 == 8
        if ____cond33 then
            if v == nil then
                w:byte(0)
            else
                w:byte(1)
                encodeValue(w, s.item, v, path)
            end
            return
        end
    until true
end
local function decodeValue(r, s)
    repeat
        local ____switch57 = s.kind
        local ____cond57 = ____switch57 == 1
        if ____cond57 then
            do
                local z = r:varint()
                local v = z % 2 == 0 and z / 2 or -(z + 1) / 2
                if v < s.min or v > s.max then
                    r:fail((((tostring(v) .. " is outside ") .. tostring(s.min)) .. "..") .. tostring(s.max))
                end
                return v
            end
        end
        ____cond57 = ____cond57 or ____switch57 == 2
        if ____cond57 then
            do
                local v = tonumber(r:bytes(32))
                if v == nil or v ~= v or v == math.huge or v == -math.huge or v < s.min or v > s.max then
                    r:fail("not a finite number in range")
                end
                return v
            end
        end
        ____cond57 = ____cond57 or ____switch57 == 3
        if ____cond57 then
            do
                local b = r:byte()
                if b > 1 then
                    r:fail("not a boolean")
                end
                return b == 1
            end
        end
        ____cond57 = ____cond57 or ____switch57 == 4
        if ____cond57 then
            return r:bytes(s.max)
        end
        ____cond57 = ____cond57 or ____switch57 == 5
        if ____cond57 then
            do
                local index = r:varint()
                if index >= #s.values then
                    r:fail("not one of the choices")
                end
                return s.values[index + 1]
            end
        end
        ____cond57 = ____cond57 or ____switch57 == 6
        if ____cond57 then
            do
                local count = r:varint()
                if count > s.max then
                    r:fail(((tostring(count) .. " items, at most ") .. tostring(s.max)) .. " allowed")
                end
                local out = {}
                do
                    local i = 0
                    while i < count do
                        out[#out + 1] = decodeValue(r, s.item)
                        i = i + 1
                    end
                end
                return out
            end
        end
        ____cond57 = ____cond57 or ____switch57 == 7
        if ____cond57 then
            do
                local out = {}
                for ____, key in ipairs(sortedKeys(s.fields)) do
                    out[key] = decodeValue(r, s.fields[key])
                end
                return out
            end
        end
        ____cond57 = ____cond57 or ____switch57 == 8
        if ____cond57 then
            do
                local flag = r:byte()
                if flag > 1 then
                    r:fail("not an optional's flag")
                end
                local ____temp_2
                if flag == 1 then
                    ____temp_2 = decodeValue(r, s.item)
                else
                    ____temp_2 = nil
                end
                return ____temp_2
            end
        end
    until true
    return r:fail("unknown type")
end
local FORMAT = 1
local REQUEST = 1
local EVENT = 2
local channels = {}
local receiving = false
local function encode(kind, channel, index, entry, args)
    local w = __TS__New(Writer)
    w:byte(FORMAT)
    w:byte(kind)
    w:bytes(channel)
    w:varint(index)
    local n = #entry.args
    if #args > n then
        error(
            (((((channel .. ".") .. entry.name) .. ": ") .. tostring(n)) .. " arguments expected, got ") .. tostring(#args),
            0
        )
    end
    do
        local i = 0
        while i < n do
            encodeValue(
                w,
                entry.args[i + 1],
                args[i + 1],
                (((channel .. ".") .. entry.name) .. " argument ") .. tostring(i + 1)
            )
            i = i + 1
        end
    end
    return table.concat(w.parts)
end
local function receive(payload, peer)
    local r = __TS__New(Reader, payload)
    local entry
    local args = {}
    local where = ""
    do
        local function ____catch(e)
            if __TS__InstanceOf(e, DecodeError) then
                log((((("channel message from peer " .. tostring(peer)) .. " dropped") .. (where and (" (" .. where) .. ")" or "")) .. ": ") .. e.why)
                return true
            end
            error(e, 0)
        end
        local ____try, ____hasReturned, ____returnValue = pcall(function()
            if r:byte() ~= FORMAT then
                r:fail("unknown channel format")
            end
            local kind = r:byte()
            local name = r:bytes(64)
            local state = channels[name]
            if not state then
                r:fail(("no channel '" .. name) .. "'")
            end
            local expect = script.side == 2 and REQUEST or EVENT
            if kind ~= expect then
                r:fail(script.side == 2 and "only requests reach the server" or "only events reach the client")
            end
            local list = kind == REQUEST and state.requests or state.events
            local index = r:varint()
            entry = list[index + 1]
            if not entry then
                r:fail((("channel '" .. name) .. "' has no message ") .. tostring(index))
            end
            where = (name .. ".") .. entry.name
            for ____, s in ipairs(entry.args) do
                args[#args + 1] = decodeValue(r, s)
            end
            if r.pos ~= #r.data + 1 then
                r:fail("bytes after the last argument")
            end
        end)
        if not ____try then
            ____hasReturned, ____returnValue = ____catch(____hasReturned)
        end
        if ____hasReturned then
            return ____returnValue
        end
    end
    if not entry.handler then
        log(((("channel message " .. where) .. " from peer ") .. tostring(peer)) .. " has no handler")
        return
    end
    if script.side == 2 then
        entry.handler(
            peer,
            unpack(args)
        )
    else
        entry.handler(unpack(args))
    end
end
local function entries(messages)
    if not messages then
        return {}
    end
    return __TS__ArrayMap(
        sortedKeys(messages),
        function(____, name) return {name = name, args = messages[name]} end
    )
end
local function onSide(side, what)
    local here = type(script) == "nil" and 1 or script.side
    if here ~= side then
        error(((what .. " is for the ") .. (side == 2 and "server" or "client")) .. " side", 3)
    end
end
--- Declare a channel of this mod. The same declaration runs on both sides.
function ____exports.defineChannel(name, messages)
    if #name == 0 or #name > 64 then
        error("defineChannel: a name of 1 to 64 bytes", 2)
    end
    if channels[name] ~= nil then
        error(("defineChannel: this mod already has a channel '" .. name) .. "'", 2)
    end
    local state = {
        name = name,
        requests = entries(messages.requests),
        events = entries(messages.events)
    }
    channels[name] = state
    if not receiving and type(script) ~= "nil" then
        script.on_message(receive)
        receiving = true
    end
    local call = {}
    local emit = {}
    local on = {}
    __TS__ArrayForEach(
        state.requests,
        function(____, entry, i)
            call[entry.name] = function(...)
                local args = {...}
                onSide(4, (name .. ".call.") .. entry.name)
                script.send(encode(
                    REQUEST,
                    name,
                    i,
                    entry,
                    args
                ))
            end
        end
    )
    __TS__ArrayForEach(
        state.events,
        function(____, entry, i)
            emit[entry.name] = function(peer, ...)
                local args = {...}
                onSide(2, (name .. ".emit.") .. entry.name)
                script.send(
                    encode(
                        EVENT,
                        name,
                        i,
                        entry,
                        args
                    ),
                    peer
                )
            end
            on[entry.name] = function(handler)
                onSide(4, (name .. ".on.") .. entry.name)
                entry.handler = handler
            end
        end
    )
    return {
        name = name,
        implement = function(handlers)
            onSide(2, name .. ".implement")
            for ____, entry in ipairs(state.requests) do
                local handler = handlers[entry.name]
                if type(handler) ~= "function" then
                    error(((name .. ".implement: no handler for request '") .. entry.name) .. "'", 2)
                end
                entry.handler = handler
            end
        end,
        call = call,
        emit = emit,
        on = on
    }
end
return ____exports
