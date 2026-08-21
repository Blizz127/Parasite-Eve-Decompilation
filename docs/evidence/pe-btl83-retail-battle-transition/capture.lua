-- PE-BTL83 retail battle-transition capture for PCSX-Redux.
-- Watchpoints only. Does not poke 4D4 / scratch / mode / HP.

local ffi = require("ffi")
local bit = bit or bit32

local OUT = os.getenv("PE_BTL83_OUT")
    or "/var/home/blizz/Applications/pcsx-redux/captures/pe-btl83"
local NO_PAD = os.getenv("PE_BTL83_NO_PAD") == "1"
local AUTO_LOAD = os.getenv("PE_BTL83_AUTO_LOAD") ~= "0"

local ADDR = {
    D4D4 = 0x8009D244,   -- gp+0x4D4 byte
    MODE = 0x8009D28C,   -- gp+0x51C word
    DEST = 0x8009D280,   -- gp+0x510 dest token
    D278 = 0x8009D278,   -- Aya HP record pointer
    D2E8 = 0x8009D2E8,
    D1A0 = 0x8009D1A0,
    GP90 = 0x8009CE00,   -- gp+0x90 script cursor
    SCRATCH0 = 0x800B6A80,
    FN_1D340 = 0x8001D340,
    FN_1E940 = 0x8001E940,
    FN_1F41C = 0x8001F41C, -- mode=3 store
    FN_1F704 = 0x8001F704,
    FN_20210 = 0x80020210,
    FN_29464 = 0x80029464, -- 293F4 4D4=1
    FN_2CEE0 = 0x8002CEE0, -- mode=7 store
    FN_33A34 = 0x80033A34, -- 33A2C 4D4=1
    FN_144FC = 0x800144FC, -- 0x55
    FN_17A50 = 0x80017A50, -- 0x2A
    FN_293F4 = 0x800293F4,
    ENTRY = 0x80072534,
}

local EXPECT = {
    [0x8001D340] = 0x27BDFCD8,
    [0x800293F4] = 0x8F850508,
    [0x80072534] = 0x3C02800A,
    [0x8002CEE0] = 0x0C01A453,
    [0x80033A2C] = 0x24020001,
    [0x800144FC] = 0x27BDFFE0,
}

local M0004I = 0xA8000248
local M0005I = 0xA80002C8
local RAM64_SHA1 = "668f4a900da0f5d581a35b533f177cb521706edf"

local FN_NAME = {
    [0x8001D340] = "func_8001D340",
    [0x8001E940] = "func_8001D340:1E940",
    [0x8001F41C] = "func_8001D340:mode3",
    [0x8001F704] = "func_8001F4D4:1F704",
    [0x80020210] = "func_800201DC:20210",
    [0x800293F4] = "func_800293F4",
    [0x80029418] = "func_800293F4:copy_0C",
    [0x80029444] = "func_800293F4:copy_0E",
    [0x80029464] = "func_800293F4:4D4=1",
    [0x8002CEE0] = "func_8002CEE0:mode7",
    [0x80033A2C] = "func_80033A2C",
    [0x80033A34] = "func_80033A2C:4D4=1",
    [0x800144FC] = "func_800144FC:op55",
    [0x80017A50] = "func_80017A50:op2A",
    [0x80072534] = "exe_entry",
}

local mem = PCSX.getMemPtr()
local scratchpad = PCSX.getScratchPtr()
local BPS = {}
local tick = 0
local identity_ok = false
local identity_written = false
local hp_watch_armed = false
local last_hp_addr = 0
local pad_phase = "WAIT_ID"
local pad_age = 0
local pad_t0 = 0
local last_hb = 0
local THEATER = 0xA8002048 -- M0064I Day-1 Theater save dest

local first = {
    d4d4_nz = false,
    scratch_bit2 = false,
    mode3 = false,
    mode7 = false,
    hp_sub = false,
    d1340 = false,
    m0005i = false,
}

local prev = {
    d4d4 = 0,
    mode = 0,
    scratch0 = 0,
    dest = 0,
    d278 = 0,
    hp = -1,
}

local function ptr(address)
    if address >= 0x1f800000 and address < 0x1f800400 then
        return scratchpad + (address - 0x1f800000)
    end
    return mem + bit.band(address, 0x001fffff)
end

local function u8(a) return tonumber(ffi.cast("uint8_t*", ptr(a))[0]) end
local function u16(a) return tonumber(ffi.cast("uint16_t*", ptr(a))[0]) end
local function u32(a) return tonumber(ffi.cast("uint32_t*", ptr(a))[0]) end
local function u32hex(v)
    v = tonumber(v) or 0
    if v < 0 then v = v + 4294967296 end
    return v % 4294967296
end
local function hex32(v) return string.format("0x%08x", u32hex(v)) end
local function hex8(v) return string.format("0x%02x", u32hex(v) % 256) end

local promote_identity

local function mkdir_p(path)
    os.execute("mkdir -p '" .. path:gsub("'", "'\\''") .. "'")
end

mkdir_p(OUT)

local files = {}
local function csv(name, header)
    local f = assert(io.open(OUT .. "/" .. name, "w"))
    f:write(header .. "\n")
    f:flush()
    files[name] = f
    return f
end

local F_MODE = csv("MODE_WRITES.csv",
    "tick,cycles,pc,ra,fn,old,new,d4d4,scratch0,dest,d278,hp,d2e8,gp90,a0,a1,a2,a3,sp,insn_words")
local F_4D4 = csv("4D4_WRITES.csv",
    "tick,cycles,pc,ra,fn,old,new,mode,scratch0,dest,d278,hp,d2e8,gp90,a0,a1,a2,a3,sp,insn_words")
local F_SCR = csv("SCRATCH_BIT4_WRITES.csv",
    "tick,cycles,pc,ra,fn,old,new,old_bit2,new_bit2,mode,d4d4,dest,d278,hp,d2e8,gp90,a0,a1,a2,a3,sp,insn_words")
local F_HP = csv("HP_WRITES.csv",
    "tick,cycles,pc,ra,fn,hp_addr,old,new,delta,mode,d4d4,scratch0,dest,d278,d2e8,gp90,a0,a1,a2,a3,sp,insn_words")
local F_CALL = csv("CALL_TRACE.csv",
    "tick,cycles,kind,pc,ra,fn,mode,d4d4,scratch0,dest,d278,hp,d2e8,gp90,a0,a1,a2,a3,sp,s0,s1,note")
local F_DEST = csv("DEST_WRITES.csv",
    "tick,cycles,pc,ra,fn,old,new,mode,d4d4,scratch0,d278,hp")
local F_HB = csv("HEARTBEAT.csv",
    "wall,tick,cycles,pc,mode,d4d4,scratch0,dest,d278,hp,d2e8,gp90,pad_phase,identity")
local F_LOG = assert(io.open(OUT .. "/capture.log", "w"))

local function log(msg)
    local line = string.format("[%s] %s", os.date("%H:%M:%S"), msg)
    print(line)
    F_LOG:write(line .. "\n")
    F_LOG:flush()
end

local function fn_at(pc)
    local p = bit.band(tonumber(pc) or 0, 0xffffffff)
    if FN_NAME[p] then return FN_NAME[p] end
    -- nearest known below
    local best, best_d = "unknown", 0x10000
    for addr, name in pairs(FN_NAME) do
        if p >= addr and (p - addr) < best_d then
            best = name
            best_d = p - addr
        end
    end
    if best ~= "unknown" then
        return string.format("%s+0x%X", best, best_d)
    end
    return "unknown"
end

local function regs_snap()
    local r = PCSX.getRegisters()
    local n = r.GPR and r.GPR.n or r
    local function g(name, idx)
        if n and n[name] ~= nil then return tonumber(n[name]) or 0 end
        if r.GPR and r.GPR.r and idx then return tonumber(r.GPR.r[idx]) or 0 end
        if r[name] ~= nil then return tonumber(r[name]) or 0 end
        return 0
    end
    return {
        pc = tonumber(r.pc) or g("pc") or 0,
        ra = g("ra", 31),
        sp = g("sp", 29),
        a0 = g("a0", 4),
        a1 = g("a1", 5),
        a2 = g("a2", 6),
        a3 = g("a3", 7),
        s0 = g("s0", 16),
        s1 = g("s1", 17),
        s2 = g("s2", 18),
        s3 = g("s3", 19),
    }
end

local function insn_words(pc)
    local p = bit.band(tonumber(pc) or 0, 0xffffffff)
    if p < 0x80010080 or p > 0x801F0000 then return "" end
    local t = {}
    for i = 32, 1, -1 do
        t[#t + 1] = string.format("%08x", u32(p - i * 4))
    end
    return table.concat(t, " ")
end

local function hp_now()
    local rec = u32(ADDR.D278)
    if rec == 0 or rec < 0x80000000 or rec > 0x801F0000 then return -1, rec end
    return u16(rec + 0x0C), rec
end

local function snap()
    local hp, rec = hp_now()
    return {
        cycles = tonumber(PCSX.getCPUCycles()) or 0,
        d4d4 = u8(ADDR.D4D4),
        mode = u32(ADDR.MODE),
        scratch0 = u32(ADDR.SCRATCH0),
        dest = u32(ADDR.DEST),
        d278 = rec or u32(ADDR.D278),
        hp = hp,
        d2e8 = u32(ADDR.D2E8),
        gp90 = u32(ADDR.GP90),
        d1a0 = u32(ADDR.D1A0),
    }
end

local function row(vals)
    local t = {}
    for i = 1, #vals do
        local v = vals[i]
        if type(v) == "number" then
            if v < 0 then t[i] = tostring(v)
            elseif v > 0xFF then t[i] = hex32(v)
            else t[i] = tostring(v)
            end
        else
            t[i] = tostring(v or "")
        end
    end
    return table.concat(t, ",")
end

local function write_first(name, text)
    local f = io.open(OUT .. "/FIRST_EVENTS.md", "a")
    f:write(text .. "\n")
    f:close()
    log("FIRST " .. name .. " " .. text)
end

local function maybe_firsts(kind, rs, st, extra)
    if kind == "4D4" and (not first.d4d4_nz) and prev.d4d4 == 0 and st.d4d4 ~= 0 then
        first.d4d4_nz = true
        write_first("4D4_nonzero", string.format(
            "pc=%s ra=%s fn=%s old=0 new=%s mode=%s dest=%s",
            hex32(rs.pc), hex32(rs.ra), fn_at(rs.pc), hex8(st.d4d4), hex32(st.mode), hex32(st.dest)))
    end
    if kind == "SCRATCH" then
        local oldb = bit.band(prev.scratch0, 4) ~= 0
        local newb = bit.band(st.scratch0, 4) ~= 0
        if (not first.scratch_bit2) and (not oldb) and newb then
            first.scratch_bit2 = true
            write_first("scratch_bit2", string.format(
                "pc=%s ra=%s fn=%s old=%s new=%s mode=%s d4d4=%s dest=%s",
                hex32(rs.pc), hex32(rs.ra), fn_at(rs.pc), hex32(prev.scratch0),
                hex32(st.scratch0), hex32(st.mode), hex8(st.d4d4), hex32(st.dest)))
        end
    end
    if kind == "MODE" then
        if (not first.mode3) and st.mode == 3 then
            first.mode3 = true
            write_first("mode3", string.format(
                "pc=%s ra=%s fn=%s old=%s new=3 d4d4=%s dest=%s",
                hex32(rs.pc), hex32(rs.ra), fn_at(rs.pc), hex32(prev.mode),
                hex8(st.d4d4), hex32(st.dest)))
        end
        if (not first.mode7) and st.mode == 7 then
            first.mode7 = true
            write_first("mode7", string.format(
                "pc=%s ra=%s fn=%s old=%s new=7 d4d4=%s dest=%s",
                hex32(rs.pc), hex32(rs.ra), fn_at(rs.pc), hex32(prev.mode),
                hex8(st.d4d4), hex32(st.dest)))
        end
    end
    if kind == "HP" and extra and extra.delta and extra.delta < 0 and (not first.hp_sub) then
        local pc = rs.pc
        if pc ~= 0x80029418 and pc ~= 0x80029444 then
            first.hp_sub = true
            write_first("hp_subtract", string.format(
                "pc=%s ra=%s fn=%s old=%s new=%s delta=%s mode=%s d4d4=%s dest=%s",
                hex32(pc), hex32(rs.ra), fn_at(pc), tostring(extra.old),
                tostring(extra.new), tostring(extra.delta), hex32(st.mode),
                hex8(st.d4d4), hex32(st.dest)))
        end
    end
    if kind == "DEST" and (not first.m0005i) and st.dest == M0005I then
        first.m0005i = true
        write_first("m0005i", string.format(
            "pc=%s ra=%s fn=%s old=%s", hex32(rs.pc), hex32(rs.ra), fn_at(rs.pc), hex32(prev.dest)))
        pad_phase = "PLAY"
    end
end

local function on_write(kind, file, extra)
    tick = tick + 1
    promote_identity()
    local ok, err = pcall(function()
        local rs = regs_snap()
        local st = snap()
        maybe_firsts(kind, rs, st, extra)
        local hp = st.hp
        if kind == "4D4" then
            file:write(row({
                tick, st.cycles, hex32(rs.pc), hex32(rs.ra), fn_at(rs.pc),
                hex8(prev.d4d4), hex8(st.d4d4), hex32(st.mode), hex32(st.scratch0),
                hex32(st.dest), hex32(st.d278), hp, hex32(st.d2e8), hex32(st.gp90),
                hex32(rs.a0), hex32(rs.a1), hex32(rs.a2), hex32(rs.a3), hex32(rs.sp),
                insn_words(rs.pc),
            }) .. "\n")
            prev.d4d4 = st.d4d4
        elseif kind == "MODE" then
            file:write(row({
                tick, st.cycles, hex32(rs.pc), hex32(rs.ra), fn_at(rs.pc),
                hex32(prev.mode), hex32(st.mode), hex8(st.d4d4), hex32(st.scratch0),
                hex32(st.dest), hex32(st.d278), hp, hex32(st.d2e8), hex32(st.gp90),
                hex32(rs.a0), hex32(rs.a1), hex32(rs.a2), hex32(rs.a3), hex32(rs.sp),
                insn_words(rs.pc),
            }) .. "\n")
            prev.mode = st.mode
        elseif kind == "SCRATCH" then
            file:write(row({
                tick, st.cycles, hex32(rs.pc), hex32(rs.ra), fn_at(rs.pc),
                hex32(prev.scratch0), hex32(st.scratch0),
                bit.band(prev.scratch0, 4) ~= 0 and 1 or 0,
                bit.band(st.scratch0, 4) ~= 0 and 1 or 0,
                hex32(st.mode), hex8(st.d4d4), hex32(st.dest), hex32(st.d278), hp,
                hex32(st.d2e8), hex32(st.gp90),
                hex32(rs.a0), hex32(rs.a1), hex32(rs.a2), hex32(rs.a3), hex32(rs.sp),
                insn_words(rs.pc),
            }) .. "\n")
            prev.scratch0 = st.scratch0
        elseif kind == "HP" then
            file:write(row({
                tick, st.cycles, hex32(rs.pc), hex32(rs.ra), fn_at(rs.pc),
                hex32(extra.addr), extra.old, extra.new, extra.delta,
                hex32(st.mode), hex8(st.d4d4), hex32(st.scratch0), hex32(st.dest),
                hex32(st.d278), hex32(st.d2e8), hex32(st.gp90),
                hex32(rs.a0), hex32(rs.a1), hex32(rs.a2), hex32(rs.a3), hex32(rs.sp),
                insn_words(rs.pc),
            }) .. "\n")
            prev.hp = extra.new
        elseif kind == "DEST" then
            file:write(row({
                tick, st.cycles, hex32(rs.pc), hex32(rs.ra), fn_at(rs.pc),
                hex32(prev.dest), hex32(st.dest), hex32(st.mode), hex8(st.d4d4),
                hex32(st.scratch0), hex32(st.d278), hp,
            }) .. "\n")
            prev.dest = st.dest
        end
        file:flush()
    end)
    if not ok then log("write-cb error " .. tostring(err)) end
end

local function on_exec(kind, note)
    tick = tick + 1
    local ok, err = pcall(function()
        local rs = regs_snap()
        local st = snap()
        if kind == "1D340" and not first.d1340 then
            first.d1340 = true
            write_first("1D340_entry", string.format(
                "ra=%s a0=%s a1=%s a2=%s a3=%s s0=%s s1=%s sp=%s mode=%s d4d4=%s dest=%s hp=%s d2e8=%s",
                hex32(rs.ra), hex32(rs.a0), hex32(rs.a1), hex32(rs.a2), hex32(rs.a3),
                hex32(rs.s0), hex32(rs.s1), hex32(rs.sp), hex32(st.mode),
                hex8(st.d4d4), hex32(st.dest), tostring(st.hp), hex32(st.d2e8)))
        end
        F_CALL:write(row({
            tick, st.cycles, kind, hex32(rs.pc), hex32(rs.ra), fn_at(rs.pc),
            hex32(st.mode), hex8(st.d4d4), hex32(st.scratch0), hex32(st.dest),
            hex32(st.d278), st.hp, hex32(st.d2e8), hex32(st.gp90),
            hex32(rs.a0), hex32(rs.a1), hex32(rs.a2), hex32(rs.a3), hex32(rs.sp),
            hex32(rs.s0), hex32(rs.s1), note or "",
        }) .. "\n")
        F_CALL:flush()
    end)
    if not ok then log("exec-cb error " .. tostring(err)) end
end

local function arm_hp(rec)
    if rec == 0 or rec < 0x80010000 or rec > 0x801E0000 then return end
    local addr = rec + 0x0C
    if hp_watch_armed and last_hp_addr == addr then return end
    last_hp_addr = addr
    local old_hp = { v = u16(addr) }
    BPS.hp = PCSX.addBreakpoint(addr, "Write", 2, "BTL83 Aya HP +0x0C", function()
        local newv = u16(addr)
        local oldv = old_hp.v
        on_write("HP", F_HP, { addr = addr, old = oldv, new = newv, delta = newv - oldv })
        old_hp.v = newv
    end)
    hp_watch_armed = true
    log(string.format("armed HP watch %s (rec=%s)", hex32(addr), hex32(rec)))
end

local function check_identity()
    if identity_ok then return true end
    for va, word in pairs(EXPECT) do
        if u32(va) ~= word then return false end
    end
    identity_ok = true
    return true
end

local function write_identity()
    if identity_written then return end
    identity_written = true
    local f = assert(io.open(OUT .. "/IDENTITY.txt", "w"))
    f:write("identity=PASS\n")
    for va, word in pairs(EXPECT) do
        f:write(string.format("ram[%s]=%s expected=%s\n", hex32(va), hex32(u32(va)), hex32(word)))
    end
    local win = ffi.string(ptr(0x80010000), 0x10000)
    local rf = assert(io.open(OUT .. "/ram64k.bin", "wb"))
    rf:write(win)
    rf:close()
    f:write("ram64k_path=" .. OUT .. "/ram64k.bin\n")
    f:write("expected_ram64k_sha1=" .. RAM64_SHA1 .. "\n")
    f:write(string.format("pc=%s\n", hex32(regs_snap().pc)))
    f:close()
    log("IDENTITY PASS — retail EXE words match SLUS_006.62")
end

local function pad()
    return PCSX.SIO0.slots[1].pads[1]
end

local function clear_pad()
    local p = pad()
    local B = PCSX.CONSTS.PAD.BUTTON
    for _, name in ipairs({ "START", "CROSS", "DOWN", "UP", "LEFT", "RIGHT",
                            "CIRCLE", "SQUARE", "TRIANGLE", "SELECT" }) do
        if B[name] then pcall(function() p.clearOverride(B[name]) end) end
    end
end

local function hold(buttons)
    clear_pad()
    local p = pad()
    for i = 1, #buttons do
        p.setOverride(buttons[i])
    end
end

promote_identity = function()
    if identity_ok then return end
    if not check_identity() then return end
    write_identity()
    if pad_phase == "WAIT_ID" then
        pad_phase = "MASH_SKIP"
        pad_age = 0
    end
end

local function pad_step()
    if NO_PAD or not AUTO_LOAD or not identity_ok then return end
    local ok, err = pcall(function()
    if pad_phase == "PLAY" then
        clear_pad()
        return
    end
    local st = snap()
    local dest = st.dest
    local combat = (st.mode ~= 0) or (st.d4d4 ~= 0) or (dest == M0005I)
    if combat then
        pad_phase = "PLAY"
        clear_pad()
        log("pad stop; combat dest=" .. hex32(dest) .. " mode=" .. hex32(st.mode))
        return
    end
    local now = os.time()
    if pad_t0 == 0 then pad_t0 = now end
    local elapsed = now - pad_t0
    local B = PCSX.CONSTS.PAD.BUTTON
    -- Wall-clock phases so interpreter/imgui rate cannot skip the title.
    if pad_phase == "WAIT_ID" or pad_phase == "MASH_SKIP" then
        if pad_phase ~= "MASH_SKIP" then
            pad_phase = "MASH_SKIP"
            log("pad MASH_SKIP")
        end
        if (elapsed % 2) == 0 then hold({ B.START }) else clear_pad() end
        if elapsed >= 28 then
            pad_phase = "LOAD_DOWN"
            pad_t0 = now
            log("pad LOAD_DOWN")
        end
        return
    end
    if pad_phase == "LOAD_DOWN" then
        hold({ B.DOWN })
        if elapsed >= 2 then
            pad_phase = "LOAD_CONFIRM"
            pad_t0 = now
            log("pad LOAD_CONFIRM")
        end
        return
    end
    if pad_phase == "LOAD_CONFIRM" then
        if (elapsed % 2) == 0 then hold({ B.CROSS }) else clear_pad() end
        -- Only the Day-1 Theater save dest. Other A8 tokens include Tutorial.
        if dest == THEATER then
            pad_phase = "WAIT_FIELD"
            pad_t0 = now
            clear_pad()
            log("pad WAIT_FIELD dest=" .. hex32(dest))
            return
        end
        if elapsed >= 45 then
            pad_phase = "PLAY"
            clear_pad()
            log("pad PLAY (load timeout dest=" .. hex32(dest) .. ")")
        end
        return
    end
    if pad_phase == "WAIT_FIELD" then
        -- Let the room finish loading before walking.
        if elapsed >= 12 then
            pad_phase = "EXPLORE"
            pad_t0 = now
            log("pad EXPLORE")
        else
            clear_pad()
        end
        return
    end
    if pad_phase == "EXPLORE" then
        if dest ~= THEATER then
            pad_phase = "PLAY"
            clear_pad()
            log("pad stop; left theater dest=" .. hex32(dest))
            return
        end
        local dirs = { B.DOWN, B.LEFT, B.UP, B.RIGHT }
        local i = (math.floor(elapsed / 4) % 4) + 1
        hold({ dirs[i] })
        return
    end
    end)
    if not ok then log("pad_step error " .. tostring(err)) end
end

local function heartbeat()
    local now = os.time()
    if now == last_hb then return end
    last_hb = now
    local ok, err = pcall(function()
        local rs = regs_snap()
        local st = snap()
        F_HB:write(string.format(
            "%s,%d,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
            os.date("%H:%M:%S"), tick, tostring(st.cycles), hex32(rs.pc),
            hex32(st.mode), hex8(st.d4d4), hex32(st.scratch0), hex32(st.dest),
            hex32(st.d278), tostring(st.hp), hex32(st.d2e8), hex32(st.gp90),
            pad_phase, identity_ok and "1" or "0"))
        F_HB:flush()
        if identity_ok then
            arm_hp(st.d278)
        end
    end)
    if not ok then log("heartbeat error " .. tostring(err)) end
end

local function arm_watches()
    BPS.d4d4 = PCSX.addBreakpoint(ADDR.D4D4, "Write", 4, "BTL83 4D4", function()
        on_write("4D4", F_4D4)
    end)
    BPS.mode = PCSX.addBreakpoint(ADDR.MODE, "Write", 4, "BTL83 MODE", function()
        on_write("MODE", F_MODE)
    end)
    BPS.scratch = PCSX.addBreakpoint(ADDR.SCRATCH0, "Write", 4, "BTL83 scratch0", function()
        on_write("SCRATCH", F_SCR)
    end)
    BPS.dest = PCSX.addBreakpoint(ADDR.DEST, "Write", 4, "BTL83 DEST", function()
        on_write("DEST", F_DEST)
    end)
    BPS.d278 = PCSX.addBreakpoint(ADDR.D278, "Write", 4, "BTL83 D278 ptr", function()
        local rec = u32(ADDR.D278)
        log("D278 ptr write " .. hex32(rec))
        arm_hp(rec)
    end)
    local function exec(addr, kind, note)
        BPS[kind] = PCSX.addBreakpoint(addr, "Exec", 4, "BTL83 " .. kind, function()
            on_exec(kind, note)
        end)
    end
    exec(ADDR.FN_1D340, "1D340", "damage_entry")
    exec(ADDR.FN_1E940, "1E940", "hp_minus_lbu92")
    exec(ADDR.FN_1F704, "1F704", "computed_damage")
    exec(ADDR.FN_20210, "20210", "dot_plus38")
    exec(ADDR.FN_1F41C, "MODE3_STORE", "sw mode=3")
    exec(ADDR.FN_2CEE0, "MODE7_STORE", "sw mode=7")
    exec(ADDR.FN_29464, "4D4_SET_293F4", "sb 4D4=1")
    exec(ADDR.FN_33A34, "4D4_SET_33A2C", "sb 4D4=1")
    exec(ADDR.FN_144FC, "OP55", "battle_0x55")
    exec(ADDR.FN_17A50, "OP2A", "scratch_bit_op")
    exec(ADDR.FN_293F4, "293F4", "hp_copy")
    BPS.entry = PCSX.addBreakpoint(ADDR.ENTRY, "Exec", 4, "BTL83 entry", function()
        promote_identity()
        on_exec("ENTRY", "exe_pc0")
    end)
    log("watchpoints armed")
end

pcall(function()
    PCSX.settings.emulator.Scaler = 400
end)

arm_watches()
log("BTL83 capture started out=" .. OUT)

local function pump()
    promote_identity()
    pad_step()
    heartbeat()
end

local function loop()
    PCSX.nextTick(function()
        local ok, err = pcall(pump)
        if not ok then log("pump error " .. tostring(err)) end
        loop()
    end)
end
loop()

function DrawImguiFrame()
    pcall(pump)
end
