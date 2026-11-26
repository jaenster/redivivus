/**
 * Every enum a mod names, canonical here; redivivus:server and redivivus:client re-export the ones
 * their objects use. TypeScript inlines the values; plain Lua gets a read-only table per enum
 * (`require("redivivus:enums").UnitType.Monster`), and pairs walks its members.
 * @side data server client
 */

export * from "./engine-enums";

/**
 * Every event. The value is the event's name on the wire, in payloads (`e.name`) and in logs;
 * which events a side raises is its events map (ServerEvents, ClientEvents).
 */
export const enum Event {
    /** A game tick (server) or a client frame tick. */
    Tick = "tick",
    /** The client draws the UI. */
    Draw = "draw",
}

/** The states a mod runs in, each a Lua state of its own. */
export const enum Side {
    Data = 1,
    Server = 2,
    Client = 4,
}

/** Whose side a unit is on (the engine's alignment stat). */
export const enum Alignment {
    Evil = 0,
    Neutral = 1,
    Good = 2,
}

/** The masks of states.txt: a unit has one when any of its states has that column set. */
export const enum StateMask {
    /** Undead-like: frozen, revived, a corpse not drawn or not selectable, shattered... */
    Udead = 0,
    /** Hidden. */
    Hide = 1,
    Active = 2,
    Aura = 3,
    RemHit = 4,
    Restrict = 5,
    Life = 6,
    Shatter = 7,
}

/** An order a group's leader gives its minions (the command their AIs read next). */
export const enum MinionOrder {
    /** Go for the target and attack it, without fleeing. */
    Attack = 1,
}

/** How an action or a wait ended. */
export const enum Outcome {
    /** It did what it set out to. */
    Done = 0,
    /** Something intervened (the target died, the unit was stunned). */
    Interrupted = 1,
    /** It could not start or go on (no path). */
    Blocked = 2,
    /** Its time ran out first. */
    Timeout = 3,
}


/**
 * The monster AIs: the stock rows of monai.txt (the MonStats "AI" column), in the engine's order.
 * Registry ids of `game.registry.ai`; the value is the row's name.
 */
export const enum MonsterAiId {
    None = "None",
    Idle = "Idle",
    Skeleton = "Skeleton",
    Zombie = "Zombie",
    Bighead = "Bighead",
    BloodHawk = "BloodHawk",
    Fallen = "Fallen",
    Brute = "Brute",
    SandRaider = "SandRaider",
    Wraith = "Wraith",
    CorruptRogue = "CorruptRogue",
    Baboon = "Baboon",
    Goatman = "Goatman",
    FallenShaman = "FallenShaman",
    QuillRat = "QuillRat",
    SandMaggot = "SandMaggot",
    ClawViper = "ClawViper",
    SandLeaper = "SandLeaper",
    PantherWoman = "PantherWoman",
    Swarm = "Swarm",
    Scarab = "Scarab",
    Mummy = "Mummy",
    GreaterMummy = "GreaterMummy",
    Vulture = "Vulture",
    Mosquito = "Mosquito",
    WillOWisp = "WillOWisp",
    Arach = "Arach",
    ThornHulk = "ThornHulk",
    Vampire = "Vampire",
    BatDemon = "BatDemon",
    Fetish = "Fetish",
    NpcOutOfTown = "NpcOutOfTown",
    Npc = "Npc",
    HellMeteor = "HellMeteor",
    Andariel = "Andariel",
    CorruptArcher = "CorruptArcher",
    CorruptLancer = "CorruptLancer",
    SkeletonBow = "SkeletonBow",
    MaggotLarva = "MaggotLarva",
    PinHead = "PinHead",
    MaggotEgg = "MaggotEgg",
    Towner = "Towner",
    Vendor = "Vendor",
    FoulCrowNest = "FoulCrowNest",
    Duriel = "Duriel",
    Sarcophagus = "Sarcophagus",
    ElementalBeast = "ElementalBeast",
    FlyingScimitar = "FlyingScimitar",
    ZakarumZealot = "ZakarumZealot",
    ZakarumPriest = "ZakarumPriest",
    Mephisto = "Mephisto",
    Diablo = "Diablo",
    FrogDemon = "FrogDemon",
    Summoner = "Summoner",
    NpcStationary = "NpcStationary",
    Izual = "Izual",
    Tentacle = "Tentacle",
    TentacleHead = "TentacleHead",
    Navi = "Navi",
    BloodRaven = "BloodRaven",
    GoodNpcRanged = "GoodNpcRanged",
    Hireable = "Hireable",
    TownRogue = "TownRogue",
    GargoyleTrap = "GargoyleTrap",
    SkeletonMage = "SkeletonMage",
    FetishShaman = "FetishShaman",
    SandMaggotQueen = "SandMaggotQueen",
    NecroPet = "NecroPet",
    VileMother = "VileMother",
    VileDog = "VileDog",
    FingerMage = "FingerMage",
    Regurgitator = "Regurgitator",
    DoomKnight = "DoomKnight",
    AbyssKnight = "AbyssKnight",
    OblivionKnight = "OblivionKnight",
    QuillMother = "QuillMother",
    EvilHole = "EvilHole",
    TrapMissile = "TrapMissile",
    TrapRightArrow = "TrapRightArrow",
    TrapLeftArrow = "TrapLeftArrow",
    TrapPoison = "TrapPoison",
    JarJar = "JarJar",
    InvisoSpawner = "InvisoSpawner",
    MosquitoNest = "MosquitoNest",
    BoneWall = "BoneWall",
    HighPriest = "HighPriest",
    Hydra = "Hydra",
    TrapMelee = "TrapMelee",
    SevenTombsIllusion = "SevenTombsIllusion",
    Megademon = "Megademon",
    Griswold = "Griswold",
    DarkWanderer = "DarkWanderer",
    TrapNova = "TrapNova",
    ArcaneTower = "ArcaneTower",
    DesertTurret = "DesertTurret",
    PantherJavelin = "PantherJavelin",
    FetishBlowgun = "FetishBlowgun",
    Spirit = "Spirit",
    Smith = "Smith",
    TrappedSoul = "TrappedSoul",
    Buffy = "Buffy",
    AssassinSentry = "AssassinSentry",
    BladeCreeper = "BladeCreeper",
    InvisoPet = "InvisoPet",
    DeathSentry = "DeathSentry",
    ShadowWarrior = "ShadowWarrior",
    ShadowMaster = "ShadowMaster",
    Raven = "Raven",
    DruidWolf = "DruidWolf",
    Totem = "Totem",
    Vines = "Vines",
    CycleOfLife = "CycleOfLife",
    DruidBear = "DruidBear",
    SiegeTower = "SiegeTower",
    ReanimatedHorde = "ReanimatedHorde",
    SiegeBeast = "SiegeBeast",
    Minion = "Minion",
    SuicideMinion = "SuicideMinion",
    Succubus = "Succubus",
    SuccubusWitch = "SuccubusWitch",
    Overseer = "Overseer",
    MinionSpawner = "MinionSpawner",
    Imp = "Imp",
    Catapult = "Catapult",
    FrozenHorror = "FrozenHorror",
    BloodLord = "BloodLord",
    CatapultSpotter = "CatapultSpotter",
    NpcBarb = "NpcBarb",
    Nihlathak = "Nihlathak",
    GenericSpawner = "GenericSpawner",
    DeathMauler = "DeathMauler",
    Wussie = "Wussie",
    AncientStatue = "AncientStatue",
    Ancient = "Ancient",
    BaalThrone = "BaalThrone",
    BaalCrab = "BaalCrab",
    BaalTaunt = "BaalTaunt",
    PutridDefiler = "PutridDefiler",
    BaalToStairs = "BaalToStairs",
    BaalTentacle = "BaalTentacle",
    BaalCrabClone = "BaalCrabClone",
    BaalMinion = "BaalMinion",
    ClawViperEx = "ClawViperEx",
    ShadowMasterEx = "ShadowMasterEx",
    UberIzual = "UberIzual",
    UberBaal = "UberBaal",
    UberMephisto = "UberMephisto",
    UberDiablo = "UberDiablo",
}

/** Which grid a point is on. Units and collision use subtiles; rooms and levels are laid out in tiles of 5 by 5 subtiles. */
export const enum PointKind {
    Subtile = "subtile",
    Tile = "tile",
}

/** The value type of a mod setting. */
export const enum SettingType {
    Bool = "bool",
    Int = "int",
    Number = "number",
    String = "string",
    Choice = "choice",
}

/** Who reads a mod setting, and when it can change. */
export const enum SettingScope {
    /** Every state, the data stage included; fixed until the game restarts. */
    Startup = "startup",
    /** The server; fixed for a game. */
    Game = "game",
    /** The client: the local player's own value. */
    Player = "player",
}

/** How an installed mod stands this session (redivivus:mods). */
export const enum ModState {
    /** It is loaded. */
    Enabled = "enabled",
    /** The player turned it off. */
    Disabled = "disabled",
    /** It failed while loading (an error, or it ran out of time or memory); `error` says why. */
    Faulted = "faulted",
    /** A dependency is missing, turned off, the wrong version, or an incompatible mod is on; `error` says which. */
    MissingDependency = "missingDependency",
}

/** How one dependency of an installed mod is met (redivivus:mods). */
export const enum DependencyState {
    Ok = "ok",
    Missing = "missing",
    /** Installed, but not a version the constraint accepts. */
    Version = "version",
    /** An optional dependency (installed or not). */
    Optional = "optional",
    /** "! other": installed although it may not be. */
    Incompatible = "incompatible",
}

/**
 * The ids of the example registry. A mod adds its own by augmenting this enum
 * (`declare module "redivivus:enums" { const enum ExampleId { X = "my-mod:x" } }`).
 */
export const enum ExampleId {
    /** "hello, <name>". */
    Greet = "redivivus:greet",
}

// --- the UI kit (redivivus:ui re-exports these) ----------------------------------------------------

/** A size mode on one axis; a number of pixels is the third kind of size. */
export const enum SizeMode {
    /** As large as the content (the default). */
    Content = "content",
    /** A share of the free space, by `flex`. */
    Fill = "fill",
}

/** Where a child sits on the cross axis of its row or column. */
export const enum Align {
    Start = "start",
    Center = "center",
    End = "end",
    Stretch = "stretch",
}

/** How the children of a row or column share its main axis. */
export const enum Justify {
    Start = "start",
    Center = "center",
    End = "end",
    /** The free space between the children. */
    Between = "between",
}

/** Horizontal alignment of text inside its box. */
export const enum TextAlign {
    Left = "left",
    Center = "center",
    Right = "right",
}

/** The main axis of a frame or flow. */
export const enum Direction {
    Row = "row",
    Column = "column",
}

/** The theme's chrome behind a container. */
export const enum BoxStyle {
    None = "none",
    Panel = "panel",
    Dialog = "dialog",
    Inset = "inset",
    Code = "code",
    Quote = "quote",
    /** The full-screen backdrop of a menu screen. */
    Screen = "screen",
}

/** The theme's button art. */
export const enum ButtonVariant {
    Wide = "wide",
    Medium = "medium",
    Narrow = "narrow",
    Short = "short",
    Cancel = "cancel",
    /** A text button in the smallest of the theme's fixed button art its label fits (never stretched). */
    Small = "small",
}

/** The fonts a widget can ask the theme for. */
export const enum FontRole {
    Body = "body",
    Small = "small",
    Tiny = "tiny",
    Title = "title",
    Heading = "heading",
    H1 = "h1",
    H2 = "h2",
    H3 = "h3",
    Button = "button",
    ButtonSmall = "buttonSmall",
    Mono = "mono",
}

/** The text colours a theme names; a number is a colour code of the game's own. */
export const enum TextColour {
    Text = "text",
    Muted = "muted",
    Link = "link",
    Code = "code",
    Heading = "heading",
    Ok = "ok",
    Warning = "warning",
    Error = "error",
    Accent = "accent",
    Disabled = "disabled",
    Emphasis = "emphasis",
    Strong = "strong",
    Black = "black",
}

/** How an image or rectangle is blended. The kit's numbers, not the engine's DrawMode. */
export const enum BlendMode {
    /** Opaque; palette index 0 of a sprite is transparent. */
    Normal = 0,
    /** 25% opaque: the engine's light transparency. */
    Trans25 = 1,
    /** 50% opaque. */
    Trans50 = 2,
    /** 75% opaque. */
    Trans75 = 3,
    /** Darkens what is under it by the image's luminance (the game's shadows). */
    AlphaBlack = 4,
    /** Adds the image to what is under it (the game's light). */
    Additive = 5,
}

/** What an input or UI event is. The host sends all but Click, which the kit makes from a press and a release. */
export const enum UIEventType {
    Click = "click",
    MouseMove = "mousemove",
    MouseDown = "mousedown",
    MouseUp = "mouseup",
    Wheel = "wheel",
    KeyDown = "keydown",
    KeyUp = "keyup",
    Text = "text",
}
