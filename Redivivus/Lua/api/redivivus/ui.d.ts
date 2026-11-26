// Generated from Redivivus/Lua/core/redivivus/ui.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
/**
 * The UI kit: signals, TSX widgets, layout, text and input (core/ui). It resolves in every state,
 * as every redivivus module does: a mod imports it wherever it likes, main included. Only the
 * client state has a renderer and input; UI built in the server state or the data stage works
 * the same and is simply never drawn. Each mod gets its own instance, so one mod's signals and
 * widgets are never another's.
 * @side data server client
 */
export * from "../ui/index";
