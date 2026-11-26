/**
 * Driving the client from a script the user starts locally (walk, cast, click through menus): the
 * capability a bot or an end-to-end test needs, which a server must never push onto a player.
 * Only a driver script resolves it; a mod never does. Nothing is in it yet but the marker below.
 * @side client
 */

/** True: the importer is a driver script. */
export const isDriver = true;
