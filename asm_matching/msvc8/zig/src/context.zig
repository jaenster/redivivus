//! What the inspection tools share (audit.load_context and friends): the configuration, the
//! carved and decoded original, the function map and the symbol map built from it.
const std = @import("std");
const audit = @import("audit.zig");
const compare = @import("compare.zig");
const dec = @import("decode.zig");
const ours = @import("ours.zig");
const refimage = @import("refimage.zig");
const util = @import("util.zig");

const gpa = util.gpa;

pub const Context = struct {
    cfg: audit.Config,
    ref: *refimage.Reference,
    model: compare.Model,
    d: dec.Decoder,

    /// audit.load_context: the original, carved (build/ref/functions.tsv) and decoded.
    pub fn open(root: []const u8) !*Context {
        const x = try gpa.create(Context);
        x.cfg = try audit.Config.init(root);
        x.ref = try refimage.Reference.open(try util.join(root, "reference/Game.exe"));
        try x.ref.load(try util.join(x.cfg.refcache, "functions.tsv"));
        x.model = try x.ref.model();
        x.d = try dec.Decoder.init(gpa);
        return x;
    }

    /// audit.sym_map(rows, librows): with crtmap.tsv's names behind them.
    pub fn symMap(x: *Context, rows: []const audit.MapRow, librows: []const audit.MapRow) !audit.SymMap {
        return audit.symMap(rows, librows, try audit.loadCrtmap(try x.cfg.evidence("crtmap.tsv")), &x.ref.img, &x.model);
    }

    /// evidence/libmap.tsv's rows, or none.
    pub fn libRows(x: *Context) ![]audit.MapRow {
        const p = try x.cfg.evidence("libmap.tsv");
        return if (util.exists(p)) audit.loadFuncmap(p) else &.{};
    }

    /// ours.load_object(build/obj, <tu>.obj).
    pub fn loadObject(x: *Context, tu: []const u8) ![]ours.OurFunc {
        var out: std.ArrayList(ours.OurFunc) = .empty;
        try ours.loadObject(&x.d, try util.join(x.cfg.build, "obj"), try std.fmt.allocPrint(gpa, "{s}.obj", .{tu}), &out);
        return out.items;
    }
};
