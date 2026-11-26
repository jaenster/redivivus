// Generated from Redivivus/Lua/core/ui/md/url.ts by Redivivus/Lua/tools/src/gen.ts. Do not edit.
/**
 * Where a link or an image may point. A renderer never follows the `url` of a node: it asks
 * here and acts on the answer.
 *
 * A link is `external` (http or https, a plain host, no credentials), `package` (a path
 * inside the mod's own archive) or `blocked`. An image is `package` or `blocked`: nothing is
 * ever loaded from another host, and no data is read out of a URL.
 */
export interface Target {
    kind: 'external' | 'package' | 'blocked';
    /** `external`: the URL, as written. `package`: the path, `/` between directories, no fragment. Otherwise ''. */
    url: string;
    /** `blocked`: why, as one of the words in the README. Otherwise ''. */
    reason: string;
}
export declare function classifyLink(url: string): Target;
/** An image is shown only from the mod's own archive. */
export declare function classifyImage(url: string): Target;
