/**
 * The data stage: a mod's `data` entry (and settings, data-updates, data-final-fixes) build the
 * prototypes every table is made from. Every mod sees the same raw table.
 * @side data server client
 */

import * as internal from "../internal/data";
import { readonly } from "../internal/proxy";

/** A prototype: a row of a data table. */
export interface Prototype {
    readonly type: string;
    readonly name: string;
    [field: string]: unknown;
}

/** The data stage's shared table. */
export interface Data {
    /** Every prototype by type and name. */
    readonly raw: { [type: string]: { [name: string]: Prototype } };
    /** Add prototypes; only in the data stage. */
    extend(this: void, prototypes: Prototype[]): void;
}

/** The data stage's shared table. Every mod sees the same one. */
export const data = readonly<Data>({ raw: internal.raw, extend: (prototypes: Prototype[]) => internal.extend(prototypes) }, "data");
