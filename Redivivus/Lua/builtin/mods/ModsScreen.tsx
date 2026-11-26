// The Mods screen, the game's own (the built-in redivivus mod's) and the kit's first real consumer,
// built on the public API only: tabs Installed / Browse / Updates / Settings; a searchable list of
// mods on the left; the selected mod's details on the right with its README, changelog and
// dependencies; a bottom bar with Back, the pending changes and "Apply and restart".

import { For, h, Fragment, Match, Show, Switch, colourCode, type Child, Align, BoxStyle, ButtonVariant, Direction, FontRole, Justify, SizeMode, TextAlign, TextColour } from "redivivus:ui";
import { DependencyState, SettingType } from "redivivus:enums";
import { detailTabs, ModsModel, ModStatus, tabLabels, type Dependency, type ModInfo, type PortalMod, type SettingInfo } from "./model";

void Fragment;

function statusText(m: ModInfo, enabled: boolean): string {
    if (m.status === ModStatus.Broken) return "failed to load";
    if (m.status === ModStatus.MissingDependency) return "missing dependency";
    if (m.status === ModStatus.Update) return `update ${m.latest}`;
    return enabled ? "enabled" : "disabled";
}

function statusColour(m: ModInfo, enabled: boolean): TextColour {
    if (m.status === ModStatus.Broken || m.status === ModStatus.MissingDependency) return TextColour.Error;
    if (m.status === ModStatus.Update) return TextColour.Warning;
    return enabled ? TextColour.Ok : TextColour.Muted;
}

function depText(d: Dependency): string {
    if (d.state === DependencyState.Ok) return d.installed.length > 0 ? `installed ${d.installed}` : "not installed (incompatible)";
    if (d.state === DependencyState.Missing) return "missing";
    if (d.state === DependencyState.Version) return `wrong version (${d.installed} installed)`;
    if (d.state === DependencyState.Optional) return d.installed.length > 0 ? `optional, installed ${d.installed}` : "optional";
    return "incompatible";
}

function depColour(d: Dependency): TextColour {
    if (d.state === DependencyState.Missing || d.state === DependencyState.Version || d.state === DependencyState.Incompatible) return TextColour.Error;
    if (d.state === DependencyState.Optional) return TextColour.Muted;
    return TextColour.Ok;
}

/** A mod's thumbnail stand-in until mods ship one: the first letter of its title. */
function initial(title: string): string {
    return title.length > 0 ? title.substring(0, 1) : "?";
}

/** A mod's Markdown document, or a line saying it has none. */
function MarkdownOr(props: { source: string; none: string }): Child {
    return (
        <Show when={props.source.length > 0 ? props.source : undefined} fallback={<label colour={TextColour.Muted} text={props.none} />}>
            {(source: string) => <markdown source={source} />}
        </Show>
    );
}

function tostringValue(v: boolean | number | string): string {
    return typeof v === "string" ? v : `${v}`;
}

function ModRow(props: { model: ModsModel; mod: ModInfo }): Child {
    const { model, mod } = props;
    const enabled = (): boolean => model.isEnabled(mod);
    return (
        <flow gap={6} padding={[0, 4, 0, 4]} align={Align.Center}>
            <checkbox checked={enabled} onChange={(on) => model.setEnabled(mod, on)} />
            <image width={24} height={24} imageWidth={24} imageHeight={24} alt={initial(mod.title)} />
            <flow direction={Direction.Column} width={SizeMode.Fill} gap={1} clip>
                <label text={mod.title} colour={() => (mod.status === ModStatus.Broken ? TextColour.Error : enabled() ? TextColour.Text : TextColour.Muted)} wrap={false} />
                <flow gap={8}>
                    <label font={FontRole.Small} wrap={false} text={mod.version} />
                    <label font={FontRole.Small} wrap={false} text={() => statusText(mod, enabled())} colour={() => statusColour(mod, enabled())} />
                </flow>
            </flow>
        </flow>
    );
}

function Details(props: { model: ModsModel; mod: ModInfo }): Child {
    const { model, mod } = props;
    return (
        <flow direction={Direction.Column} gap={6} height={SizeMode.Fill}>
            <label font={FontRole.H1} fitFonts={[FontRole.H2]} colour={TextColour.Heading} text={mod.title} wrap={false} />
            <grid columnWidths={[SizeMode.Content, SizeMode.Fill, SizeMode.Content, SizeMode.Fill]} gap={8} rowGap={2}>
                <label colour={TextColour.Muted} text="Author" />
                <label text={mod.author} />
                <label colour={TextColour.Muted} text="Version" />
                <label text={mod.latest.length > 0 ? `${mod.version} (${mod.latest} available)` : mod.version} />
                <label colour={TextColour.Muted} text="Licence" />
                <Show when={mod.licence.length > 0 ? mod.licence : undefined} fallback={<label colour={TextColour.Muted} text="none" />}>
                    {(l: string) => <label text={l} />}
                </Show>
                <label colour={TextColour.Muted} text="Name" />
                <label text={mod.name} />
            </grid>
            <Show when={mod.error.length > 0 ? mod.error : undefined}>{(e: string) => <label colour={TextColour.Error} text={`Load error: ${e}`} />}</Show>
            <tabs labels={detailTabs} selected={model.detailTab} onChange={(i) => model.setDetailTab(i)} />
            <frame style={BoxStyle.Inset} padding={6} height={SizeMode.Fill} direction={Direction.Column}>
                <Switch>
                    <Match when={() => model.detailTab() === 0}>
                        {() => (
                            <scrollPane height={SizeMode.Fill}>
                                <MarkdownOr source={model.data.readme(mod.name)} none="This mod has no README." />
                            </scrollPane>
                        )}
                    </Match>
                    <Match when={() => model.detailTab() === 1}>
                        {() => (
                            <scrollPane height={SizeMode.Fill}>
                                <MarkdownOr source={model.data.changelog(mod.name)} none="This mod has no changelog." />
                            </scrollPane>
                        )}
                    </Match>
                    <Match when={() => model.detailTab() === 2}>
                        {() => (
                            <flow direction={Direction.Column} gap={4}>
                                <Show when={mod.dependencies.length === 0 ? true : undefined}>{() => <label colour={TextColour.Muted} text="No dependencies." />}</Show>
                                <For each={mod.dependencies}>
                                    {(d: Dependency) => (
                                        <flow gap={8}>
                                            <label width={180} wrap={false} text={d.spec} />
                                            <label width={SizeMode.Fill} colour={depColour(d)} text={depText(d)} />
                                        </flow>
                                    )}
                                </For>
                            </flow>
                        )}
                    </Match>
                </Switch>
            </frame>
        </flow>
    );
}

function InstalledTab(props: { model: ModsModel }): Child {
    const { model } = props;
    const selectedIndex = (): number => {
        const list = model.visible();
        for (let i = 0; i < list.length; i++) if (list[i].name === model.selected()) return i;
        return -1;
    };
    return (
        <flow gap={10} height={SizeMode.Fill} align={Align.Stretch}>
            <frame style={BoxStyle.Inset} width={SizeMode.Fill} flex={2} maxWidth={320} padding={6} gap={6} direction={Direction.Column} clip>
                <textField placeholder="Search mods" value={model.query} onInput={(v) => model.setQuery(v)} width={SizeMode.Fill} />
                <list
                    items={model.visible}
                    rowHeight={40}
                    height={SizeMode.Fill}
                    selected={selectedIndex}
                    onSelect={(i) => {
                        const m = model.visible()[i];
                        if (m !== undefined) model.select(m.name);
                    }}
                    render={(m: ModInfo) => <ModRow model={model} mod={m} />}
                />
            </frame>
            <frame style={BoxStyle.Inset} width={SizeMode.Fill} flex={3} padding={8} direction={Direction.Column} clip>
                <Show when={model.current} keyed fallback={<label colour={TextColour.Muted} text="Select a mod." />}>
                    {(m: ModInfo) => <Details model={model} mod={m} />}
                </Show>
            </frame>
        </flow>
    );
}

function BrowseTab(props: { model: ModsModel }): Child {
    return (
        <frame style={BoxStyle.Inset} padding={8} height={SizeMode.Fill} direction={Direction.Column} gap={6}>
            <label colour={TextColour.Muted} text="Mods on redivivus.app. Installing one takes effect after a restart." />
            <list
                items={props.model.portal}
                rowHeight={40}
                height={SizeMode.Fill}
                render={(p: PortalMod) => (
                    <flow gap={10} padding={[0, 4, 0, 4]}>
                        <image width={32} height={32} imageWidth={32} imageHeight={32} alt={initial(p.title)} />
                        <flow direction={Direction.Column} width={SizeMode.Fill}>
                            <label wrap={false} text={`${p.title} ${colourCode(5)}${p.version} by ${p.author}`} />
                            <label font={FontRole.Small} colour={TextColour.Muted} wrap={false} text={`${p.summary}  ${p.downloads} downloads`} />
                        </flow>
                        <button variant={ButtonVariant.Small} label={p.installed ? "Installed" : "Install"} disabled={p.installed} />
                    </flow>
                )}
            />
        </frame>
    );
}

function SettingsTab(props: { model: ModsModel }): Child {
    return (
        <frame style={BoxStyle.Inset} padding={8} height={SizeMode.Fill} direction={Direction.Column} gap={8}>
            <For each={props.model.settings}>
                {(s: SettingInfo) => (
                    <flow gap={10}>
                        <label width={120} colour={TextColour.Muted} wrap={false} text={s.mod} />
                        <Show when={s.type === SettingType.Bool ? true : undefined} fallback={() => (
                            <flow gap={8} width={SizeMode.Fill}>
                                <label width={260} text={s.title} />
                                <textField width={120} value={tostringValue(s.value)} />
                            </flow>
                        )}>
                            {() => <checkbox label={s.title} checked={s.value === true} />}
                        </Show>
                    </flow>
                )}
            </For>
        </frame>
    );
}

export function ModsScreen(props: { model: ModsModel }): Child {
    const { model } = props;
    const pendingText = (): string => {
        const n = model.pending();
        if (n === 0) return "No changes.";
        return `${n} pending change${n === 1 ? "" : "s"}: restart to apply.`;
    };
    return (
        <frame style={BoxStyle.Screen} width={SizeMode.Fill} height={SizeMode.Fill}>
            <frame style={BoxStyle.Dialog} anchor={{ left: 16, top: 12, right: 16, bottom: 12 }} padding={8} gap={8} direction={Direction.Column}>
                <flow gap={12}>
                    <label font={FontRole.Title} colour={TextColour.Heading} text="Mods" wrap={false} />
                    <tabs labels={tabLabels} width={SizeMode.Fill} justify={Justify.End} selected={model.tab} onChange={(i) => model.setTab(i)} />
                </flow>
                <flow direction={Direction.Column} height={SizeMode.Fill}>
                    <Switch>
                        <Match when={() => model.tab() === 0 || model.tab() === 2}>{() => <InstalledTab model={model} />}</Match>
                        <Match when={() => model.tab() === 1}>{() => <BrowseTab model={model} />}</Match>
                        <Match when={() => model.tab() === 3}>{() => <SettingsTab model={model} />}</Match>
                    </Switch>
                </flow>
                <flow gap={12}>
                    <button variant={ButtonVariant.Medium} label="Back" onClick={() => model.data.back()} testId="mods-back" />
                    <label width={SizeMode.Fill} textAlign={TextAlign.Center} colour={() => (model.pending() > 0 ? TextColour.Warning : TextColour.Muted)} text={pendingText} />
                    <button variant={ButtonVariant.Wide} label="Apply and restart" disabled={() => model.pending() === 0} onClick={() => model.data.apply(model.changes())} testId="mods-apply" />
                </flow>
            </frame>
        </frame>
    );
}
